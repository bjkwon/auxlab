#!/usr/bin/env python3
"""Build and stage the Windows baseline; GUI/audio acceptance remains manual."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import time

from audit_build_inputs import audit

ROOT = Path(__file__).resolve().parents[1]
BINARIES = ('auxlab64.exe', 'auxp64.dll', 'AUXLib.dll')


class QualificationError(RuntimeError):
    pass


def save_json(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def run_step(command, cwd, log, timeout, steps):
    """Keep full output on disk and distinguish failure from timeout."""
    step = {'command': [str(arg) for arg in command], 'cwd': str(cwd),
            'log': str(log), 'status': 'running'}
    steps.append(step)
    started = time.monotonic()
    print(f'Running {log.name}', flush=True)
    try:
        with log.open('wb') as output:
            with subprocess.Popen(step['command'], cwd=cwd, stdout=output,
                                  stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL) as process:
                try:
                    code = process.wait(timeout=timeout)
                except (subprocess.TimeoutExpired, KeyboardInterrupt) as error:
                    # Kill MSBuild's compiler children too, not just the parent.
                    if os.name == 'nt':
                        try:
                            subprocess.run(['taskkill', '/PID', str(process.pid), '/T', '/F'],
                                           stdout=output, stderr=subprocess.STDOUT, timeout=30)
                        except (OSError, subprocess.TimeoutExpired):
                            pass
                    process.kill()
                    process.wait()
                    if isinstance(error, KeyboardInterrupt):
                        step['status'] = 'interrupted'
                        raise
                    step['status'] = 'timeout'
                    raise QualificationError(f'Timed out: {log}')
        step.update(returncode=code, status='passed' if code == 0 else 'failed')
        if code:
            raise QualificationError(f'Exit {code}: {log}')
    except OSError as error:
        step.update(status='failed', error=str(error))
        raise QualificationError(str(error)) from error
    finally:
        step['elapsed_seconds'] = round(time.monotonic() - started, 3)


def stage_binaries(build, stage):
    missing = [name for name in BINARIES if not (build / name).is_file()]
    if missing:
        raise QualificationError('Missing build artifacts: ' + ', '.join(missing))
    stage.mkdir(parents=True, exist_ok=False)
    manifest = {}
    for name in BINARIES:
        target = stage / name
        shutil.copy2(build / name, target)
        manifest[name] = hashlib.sha256(target.read_bytes()).hexdigest()
    # Retain directory structure to distinguish compiler and linker PDBs.
    for source in build.rglob('*.pdb'):
        target = stage / 'symbols' / source.relative_to(build)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    return manifest


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', required=True, help='Exact SDK version, e.g. 10.0.19041.0')
    parser.add_argument('--timeout', type=int, default=1800, help='Seconds per build (default: 1800)')
    args = parser.parse_args(argv)
    if os.name != 'nt':
        parser.error('Run on Windows from an x64 Native Tools prompt with the v142 toolset.')
    if not re.fullmatch(r'10\.0\.\d+\.\d+', args.sdk) or args.timeout <= 0:
        parser.error('Supply an exact Windows 10 SDK version and a positive timeout.')
    programs = {name: shutil.which(name) for name in ('msbuild', 'cl', 'dumpbin', 'git')}
    missing = [name for name, path in programs.items() if path is None]
    if missing:
        parser.error('Missing tools in PATH: ' + ', '.join(missing))
    if os.environ.get('VSCMD_ARG_TGT_ARCH', '').lower() != 'x64':
        parser.error('Use an x64 Native Tools prompt (VSCMD_ARG_TGT_ARCH=x64).')
    if not os.environ.get('VCToolsVersion', '').startswith('14.2'):
        parser.error('Activate the v142 compiler (VCToolsVersion=14.2x); do not retarget silently.')
    sdk_root = os.environ.get('WindowsSdkDir')
    if not sdk_root or not (Path(sdk_root) / 'Lib' / args.sdk / 'um' / 'x64').is_dir():
        parser.error('Requested SDK was not found under WindowsSdkDir/Lib.')

    output = ROOT / 'artifacts' / 'qualification'
    output.mkdir(parents=True, exist_ok=True)
    run = Path(tempfile.mkdtemp(prefix='run-', dir=output))
    build = run / 'build with spaces'
    stage = Path(tempfile.mkdtemp(prefix='AUXLAB staged '))
    report = {'started_utc': datetime.now(timezone.utc).isoformat(),
              'status': 'running', 'work_package_a': 'pending',
              'host': platform.platform(), 'python': sys.version,
              'sdk': args.sdk, 'programs': programs, 'source': str(ROOT),
              'build': str(build), 'stage': str(stage), 'steps': [],
              'environment': {name: os.environ.get(name) for name in
                              ('VCToolsVersion', 'VSCMD_ARG_TGT_ARCH', 'WindowsSDKVersion')},
              'manual_checks': 'pending: staged launch, scripts, GUI, debugger, persistence, audio; '
                               'changed-header rebuild, checkout path with spaces, second clean checkout'}
    try:
        save_json(run / 'report.json', report)
        inputs = audit()
        save_json(run / 'inputs.json', inputs)
        if inputs['missing'] or inputs['unresolved']:
            raise QualificationError('Input audit failed; see inputs.json.')
        commands = [('revision', [programs['git'], 'rev-parse', 'HEAD']),
                    ('working-tree', [programs['git'], 'status', '--short']),
                    ('working-diff', [programs['git'], 'diff', '--binary', 'HEAD']),
                    ('msbuild-version', [programs['msbuild'], '-version']),
                    ('copy-regressions', [sys.executable, '-m', 'unittest', 'discover',
                                          '-s', str(ROOT / 'tools' / 'tests'),
                                          '-p', 'test_copy_scripts.py', '-v'])]
        for name, command in commands:
            run_step(command, ROOT, run / (name + '.log'), 120, report['steps'])
        for config in ('Debug', 'Release'):
            for target in ('Rebuild', 'Build'):
                name = config.lower() + '-' + target.lower()
                command = [programs['msbuild'], str(ROOT / 'auxlab.sln'), '/t:' + target,
                           '/m:1', '/nr:false', '/p:Configuration=' + config,
                           '/p:Platform=x64', '/p:PlatformToolset=v142',
                           '/p:WindowsTargetPlatformVersion=' + args.sdk,
                           '/p:BuildDir=' + str(build) + os.sep,
                           '/bl:' + str(run / (name + '.binlog'))]
                run_step(command, ROOT, run / (name + '.log'), args.timeout, report['steps'])
            destination = stage / config
            manifest = stage_binaries(build / 'x64' / config, destination)
            save_json(run / (config.lower() + '-artifacts.json'), manifest)
            for binary in BINARIES:
                run_step([programs['dumpbin'], '/dependents', str(destination / binary)],
                         destination, run / (config.lower() + '-' + binary + '-dependencies.log'),
                         60, report['steps'])
        # Preserve the exact manual test inputs separately from runtime staging.
        cases = stage / 'script-inputs'
        cases.mkdir()
        for source in [*ROOT.glob('test_*.aux'), *ROOT.glob('reccb*.aux'),
                       *(ROOT / 'auxp').glob('*.aux')]:
            shutil.copy2(source, cases / source.name)
        report['status'] = 'build_and_stage_passed'
    except (QualificationError, OSError, ValueError) as error:
        report.update(status='failed', error=str(error))
        print(str(error), file=sys.stderr)
    except KeyboardInterrupt:
        report.update(status='interrupted')
    finally:
        report['finished_utc'] = datetime.now(timezone.utc).isoformat()
        save_json(run / 'report.json', report)
        print(f'Record: {run}\nStaged files: {stage}\nGUI/audio qualification remains pending.')
    return 0 if report['status'] == 'build_and_stage_passed' else 1


if __name__ == '__main__':
    sys.exit(main())
