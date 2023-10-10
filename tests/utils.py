# Copyright (c) 2013-2023, EPFL/Blue Brain Project
# SPDX-License-Identifier: Apache-2.0
'''Module providing utility functions for the tests'''
import re
import sys
from contextlib import contextmanager
from functools import partial
from io import StringIO

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


def strip_color_codes(string):
    '''Strip color codes from the input string'''
    ansi_escape = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')
    return ansi_escape.sub('', string)


def _assert_exception(content, exception, str1, str2, extension):
    '''with given content and check that the exception is raised'''
    with pytest.raises(exception) as obj:
        Morphology(content, extension=extension)
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
