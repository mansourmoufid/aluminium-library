#!/usr/bin/env python

import sys

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

setup(
    py_modules=['libal'],
    platforms=['macosx-11.0-arm64'] if 'bdist_wheel' in sys.argv else [],
)
