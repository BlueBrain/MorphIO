'''Module providing utility functions for the tests'''
import re
import shutil
import sys
import tempfile
from contextlib import contextmanager
from functools import partial
from io import StringIO

from nose.tools import assert_raises, ok_

from morphio import Morphology, set_ignored_warning


@contextmanager
def setup_tempdir(prefix, no_cleanup=False):
    '''Context manager returning a temporary directory'''
    temp_dir = tempfile.mkdtemp(prefix=prefix)
    try:
        yield temp_dir
    finally:
        if not no_cleanup:
            shutil.rmtree(temp_dir)

@contextmanager
def ignored_warning(warning):
    '''Context manager during which a warning is ignored'''
    try:
        set_ignored_warning(warning, True)
        yield
    finally:
        set_ignored_warning(warning, False)

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


def strip_all(string):
    '''Strip color code and whitespace at the beginning end of each line'''
    lines = (strip_color_codes(line).strip() for line in string.split('\n'))
    return '\n'.join(filter(None, lines))


def assert_substring(substring, string):
    sep = ['\n' + 80 * '>' + '\n', '\n' + 80 * '<' + '\n']
    ok_(substring in string, "{}\n NOT IN \n{}".format(substring.join(sep), string.join(sep)))


def assert_string_equal(str1, str2):
    sep = ['\n' + 80 * '>' + '\n', '\n' + 80 * '<' + '\n']
    ok_(strip_all(str1) == strip_all(str2), "{}\n NOT IN \n{}".format(
        str1.join(sep), str2.join(sep)))


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
