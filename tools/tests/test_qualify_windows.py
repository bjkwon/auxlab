"""Portable checks for recording build failures and preserving artifacts."""
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from qualify_windows import BINARIES, QualificationError, run_step, stage_binaries


class QualificationTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='qualification tests ')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def test_nonzero_exit_retains_diagnostics(self):
        log, steps = self.root / 'failure.log', []
        with self.assertRaises(QualificationError):
            run_step([sys.executable, '-c', "print('build failed'); raise SystemExit(7)"],
                     self.root, log, 10, steps)
        self.assertEqual(steps[0]['status'], 'failed')
        self.assertEqual(steps[0]['returncode'], 7)
        self.assertIn('build failed', log.read_text())

    def test_timeout_is_distinct_from_failure(self):
        steps = []
        with self.assertRaises(QualificationError):
            run_step([sys.executable, '-c', 'import time; time.sleep(30)'],
                     self.root, self.root / 'timeout.log', 0.1, steps)
        self.assertEqual(steps[0]['status'], 'timeout')

    def test_missing_artifact_cannot_pass_staging(self):
        with self.assertRaises(QualificationError):
            stage_binaries(self.root, self.root / 'stage')
        self.assertFalse((self.root / 'stage').exists())

    def test_staging_preserves_binaries_and_symbol_paths(self):
        build = self.root / 'build'
        build.mkdir()
        for name in BINARIES:
            (build / name).write_bytes(name.encode())
        for project in ('auxlab', 'AUXLib'):
            (build / project).mkdir()
            (build / project / 'vc142.pdb').write_bytes(project.encode())
        stage = self.root / 'stage'
        manifest = stage_binaries(build, stage)
        self.assertEqual(set(manifest), set(BINARIES))
        for name in BINARIES:
            self.assertEqual((stage / name).read_bytes(), name.encode())
        for project in ('auxlab', 'AUXLib'):
            self.assertEqual((stage / 'symbols' / project / 'vc142.pdb').read_bytes(),
                             project.encode())


if __name__ == '__main__':
    unittest.main()
