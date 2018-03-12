from contextlib import contextmanager
import tempfile

from morphio import Morphology
from nose.tools import ok_, assert_raises
from functools import partial


@contextmanager
def _tmp_file(content, extension):
    with tempfile.NamedTemporaryFile(suffix='.' + extension, mode='w') as tmp_file:
        tmp_file.write(content)
        tmp_file.seek(0)
        yield tmp_file


tmp_asc_file = partial(_tmp_file, extension='asc')
tmp_swc_file = partial(_tmp_file, extension='swc')


def assert_substring(substring, string):
    sep = ['\n' + 80 * '>' + '\n', '\n' + 80 * '<' + '\n']
    ok_(substring in string, "{}\n not in \n{}".format(substring.join(sep), string.join(sep)))


def _test_exception(content, exception, str1, str2, extension):
    '''Create tempfile with given content and check that the exception is raised'''
    with _tmp_file(content, extension) as tmp_file:
        with assert_raises(exception) as obj:
            Morphology(tmp_file.name)
        assert_substring(str1, str(obj.exception))
        assert_substring(str2, str(obj.exception))


_test_asc_exception = partial(_test_exception, extension='asc')
_test_swc_exception = partial(_test_exception, extension='swc')
