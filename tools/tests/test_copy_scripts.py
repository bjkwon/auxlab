"""Execute the real batch helpers in disposable paths containing spaces."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


@unittest.skipUnless(os.name == 'nt', 'Requires Windows cmd.exe')
class CopyScriptsTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='AUXLAB copy tests ')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'repo with spaces'
        self.root.mkdir()
        self.cwd = Path(self.temp.name) / 'unrelated working directory'
        self.cwd.mkdir()
        for name in ('copy_header.bat', 'copy_headers.bat', 'copy_lib_files.bat'):
            shutil.copy2(ROOT / name, self.root / name)

    def run_batch(self, name, *args):
        # A fixed driver name avoids cmd.exe /c's special outer-quote rules.
        # All paths passed to CALL are quoted and end in a filename or dot.
        command = 'call ' + ' '.join('"' + str(arg) + '"'
                                    for arg in (self.root / name, *args))
        (self.cwd / 'driver.cmd').write_bytes(
            ('@echo off\r\n' + command + '\r\nexit /b %errorlevel%\r\n').encode('utf-8'))
        result = subprocess.run([os.environ.get('COMSPEC', 'cmd.exe'), '/d', '/c', 'driver.cmd'],
                                cwd=self.cwd, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=30)
        return result.returncode

    def test_single_header_refresh_and_missing_source(self):
        source = self.root / 'source headers'
        source.mkdir()
        header = source / 'sample.h'
        header.write_bytes(b'first')
        self.assertEqual(self.run_batch('copy_header.bat', 'sample', source), 0)
        staged = self.root / 'include' / 'sample.h'
        self.assertEqual(staged.read_bytes(), b'first')
        header.write_bytes(b'updated')
        self.assertEqual(self.run_batch('copy_header.bat', 'sample', source), 0)
        self.assertEqual(staged.read_bytes(), b'updated')
        header.unlink()
        self.assertNotEqual(self.run_batch('copy_header.bat', 'sample', source), 0)

    def test_header_list_propagates_first_failure(self):
        source = self.root / 'available'
        source.mkdir()
        (source / 'available.h').write_bytes(b'present')
        self.assertNotEqual(self.run_batch('copy_headers.bat', 'missing', 'available'), 0)
        self.assertFalse((self.root / 'include' / 'available.h').exists())
        self.assertEqual(self.run_batch('copy_headers.bat', 'available'), 0)
        self.assertEqual((self.root / 'include' / 'available.h').read_bytes(), b'present')

    def test_library_list_refresh_and_failure(self):
        source = self.root / 'source libraries'
        destination = self.root / 'output libraries'
        source.mkdir()
        for name in ('one.lib', 'two.lib'):
            (source / name).write_bytes(name.encode())
        args = (source, destination, 'one.lib', 'two.lib')
        self.assertEqual(self.run_batch('copy_lib_files.bat', *args), 0)
        (source / 'one.lib').write_bytes(b'replacement')
        self.assertEqual(self.run_batch('copy_lib_files.bat', *args), 0)
        self.assertEqual((destination / 'one.lib').read_bytes(), b'replacement')
        self.assertEqual((destination / 'two.lib').read_bytes(), b'two.lib')
        self.assertNotEqual(self.run_batch('copy_lib_files.bat', source, destination,
                                          'missing.lib', 'two.lib'), 0)

    def test_invalid_destination_fails(self):
        source = self.root / 'source'
        source.mkdir()
        (source / 'sample.h').write_bytes(b'header')
        (self.root / 'include').write_bytes(b'not a directory')
        self.assertNotEqual(self.run_batch('copy_header.bat', 'sample', source), 0)
        destination = self.root / 'not a directory'
        destination.write_bytes(b'file')
        self.assertNotEqual(self.run_batch('copy_lib_files.bat', source, destination, 'sample.h'), 0)


if __name__ == '__main__':
    unittest.main()
