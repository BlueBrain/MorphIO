'''Module providing utility functions for the tests'''
import re
import shutil
import sys
import tempfile
from contextlib import contextmanager
from functools import partial
from io import StringIO

from nose.tools import assert_raises, ok_

from morphio import Morphology


@contextmanager
def setup_tempdir(prefix):
    '''Context manager returning a temporary directory'''
    temp_dir = tempfile.mkdtemp(prefix=prefix)
    try:
        yield temp_dir
    finally:
        shutil.rmtree(temp_dir)


@contextmanager
def _tmp_file(content, extension):
    with tempfile.NamedTemporaryFile(suffix='.' + extension, mode='w') as tmp_file:
        tmp_file.write(content)
        tmp_file.seek(0)
        yield tmp_file


tmp_asc_file = partial(_tmp_file, extension='asc')
tmp_swc_file = partial(_tmp_file, extension='swc')


def strip_color_codes(string):
    '''Strip color codes from the input string'''
    ansi_escape = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')
    return ansi_escape.sub('', string)


def assert_substring(substring, string):
    sep = ['\n' + 80 * '>' + '\n', '\n' + 80 * '<' + '\n']
    ok_(substring in string, "{}\n NOT IN \n{}".format(substring.join(sep), string.join(sep)))


def _test_exception(content, exception, str1, str2, extension):
    '''Create tempfile with given content and check that the exception is raised'''
    with _tmp_file(content, extension) as tmp_file:
        with assert_raises(exception) as obj:
            Morphology(tmp_file.name)
        assert_substring(str1, str(obj.exception))
        assert_substring(str2, str(obj.exception))


_test_asc_exception = partial(_test_exception, extension='asc')
_test_swc_exception = partial(_test_exception, extension='swc')


@contextmanager
def captured_output():
    '''Capture the python streams

    To be used as:
    with captured_output() as (out, err):
        print('hello world')
    assert_equal(out.getvalue().strip(), 'hello world')
    '''
    new_out, new_err = StringIO(), StringIO()
    old_out, old_err = sys.stdout, sys.stderr
    try:
        sys.stdout, sys.stderr = new_out, new_err
        yield sys.stdout, sys.stderr
    finally:
        sys.stdout, sys.stderr = old_out, old_err
