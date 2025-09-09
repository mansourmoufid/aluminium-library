# This file is part of Aluminium Library.
#
# Aluminium Library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Aluminium Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with Aluminium Library. If not, see <https://www.gnu.org/licenses/>.


'''Aluminium Library is a cross-platform computer vision library.'''


__author__ = 'Mansour Moufid'
__copyright__ = 'Copyright 2025, Mansour Moufid'
__license__ = 'GPL'
__version__ = '0.2'
__email__ = 'mansourmoufid@gmail.com'
__status__ = 'Alpha'


__all__ = [
    'AlException',
    'ColorFormat',
    'platform',
    'tobytes',
    'tobytearray',
    'net',
    'camera',
]


import ctypes
import enum
import os
import sys
import sysconfig
# import traceback

if sys.platform == 'darwin':
    # import objc
    pass


libal = None
_dirs = []
if 'LIBAL_LIBRARY_PATH' in os.environ:
    _dirs += os.environ['LIBAL_LIBRARY_PATH'].split(os.pathsep)
_dirs += [sysconfig.get_config_var('LIBDIR')]
_names = ['libal.dll', 'libal.dylib', 'libal.so']
for _lib in [os.path.join(dir, name) for dir in _dirs for name in _names]:
    try:
        libal = ctypes.cdll.LoadLibrary(_lib)
        break
    except OSError:
        # traceback.print_exc(file=sys.stderr)
        continue
if libal is None:
    import importlib.resources
    with importlib.resources.path("libal", "libal.so") as lib_path:
        libal = ctypes.cdll.LoadLibrary(str(lib_path))


class Status(enum.IntEnum):
    OK = 0
    ERROR = 1
    NOTIMPLEMENTED = 2
    NOMEMORY = 3


class AlException(Exception):
    pass


class ColorFormat(enum.IntEnum):
    UNKNOWN = 0
    YUV420SP = 1
    YUV420P = 2
    RGBA = 3


platform = ctypes.c_char_p.in_dll(libal, 'platform').value.decode('utf-8')


def tobytes(p: ctypes.c_void_p, size: int) -> bytes:
    '''Convert a void pointer to an object of the built-in type 'bytes'.'''
    if not p:
        return b''
    return ctypes.string_at(p, size)


def tobytearray(p: ctypes.c_void_p, size: int) -> bytearray:
    '''Convert a void pointer to an object of the built-in type 'bytearray'.'''
    if not p:
        return bytearray(b'')
    type = ctypes.c_char * size
    buffer = ctypes.cast(p, ctypes.POINTER(type)).contents
    return bytearray(buffer)
