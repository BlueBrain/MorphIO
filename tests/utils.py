'''Module providing utility functions for the tests'''
import re
import shutil
import sys
import tempfile

from contextlib import contextmanager
from difflib import Differ
from functools import partial
from io import StringIO
from pprint import pformat

import pytest

from morphio import Morphology, set_ignored_warning


@contextmanager
def ignored_warning(warning):
    '''Context manager during which a warning is ignored'''
    try:
        set_ignored_warning(warning, True)
        yield
    finally:
        set_ignored_warning(warning, False)


@contextmanager
def _tmp_file(tmp_folder, content, extension):
    '''create temp file in `tmp_folder` with `content` and `extension`'''
    # Note: the file isn't cleaned up until the tmp_folder is deleted;
    # this should aide in debugging, and is cross-platform an auto-deleting
    # NamedTemporaryFile doesn't allow Windows to use the temporary file per
    # https://docs.python.org/3/library/tempfile.html#tempfile.NamedTemporaryFile:
    #   "(it can be so used on Unix; it cannot on Windows NT or later)"
    suffix = '.' + extension
    with tempfile.NamedTemporaryFile(suffix=suffix,
                                     mode='w',
                                     dir=tmp_folder,
                                     delete=False) as tmp_file:
        tmp_file.write(content)
    yield tmp_file


tmp_asc_file = partial(_tmp_file, extension='asc')
tmp_swc_file = partial(_tmp_file, extension='swc')


def strip_color_codes(string):
    '''Strip color codes from the input string'''
    ansi_escape = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')
    return ansi_escape.sub('', string)


def strip_all(string):
    '''Strip color code and whitespace at the beginning end of each line'''
    lines = (strip_color_codes(line).strip() for line in string.splitlines())
    return list(filter(None, lines))


def assert_substring(substring, string):
    sep = ['\n' + 80 * '>' + '\n', '\n' + 80 * '<' + '\n']
    assert substring in string, "{}\n NOT IN \n{}".format(substring.join(sep), string.join(sep))


def assert_string_equal(str1, str2):
	str1, str2 = strip_all(str1), strip_all(str2)
	diff = list(Differ().compare(str1, str2))
	if '\n'.join(str1) != '\n'.join(str2):
		raise AssertionError('Strings does not match:\n\n' + pformat(diff))


def _assert_exception(tmp_path, content, exception, str1, str2, extension):
    '''Create tempfile with given content and check that the exception is raised'''
    with _tmp_file(tmp_path, content, extension) as tmp_file:
        with pytest.raises(exception) as obj:
            Morphology(tmp_file.name)
        assert obj.match(str1)
        assert obj.match(str2)


assert_asc_exception = partial(_assert_exception, extension='asc')
assert_swc_exception = partial(_assert_exception, extension='swc')


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
