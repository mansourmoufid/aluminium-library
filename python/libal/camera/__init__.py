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


'''The Aluminium Library camera module.'''


__all__ = [
    'AlExceptionUnsupportedColorFormat',
    'Facing',
    'Camera',
]


import ctypes
import enum
import sys
# import traceback

if sys.platform == 'darwin':
    # import objc
    pass

from .. import (
    AlException,
    ColorFormat,
    libal,
    Status,
    tobytes,
    tobytearray,
)


class AlExceptionUnsupportedColorFormat(AlException):
    pass


class Facing(enum.IntEnum):
    FONT = 0
    BACK = 1


# struct al_camera;
class _AlCamera(ctypes.Structure):
    pass


# enum al_status al_camera_new(struct al_camera **, size_t, size_t, size_t);
_al_camera_new = libal.al_camera_new
_al_camera_new.restype = ctypes.c_int
_al_camera_new.argtypes = [
    ctypes.POINTER(ctypes.POINTER(_AlCamera)),
    ctypes.c_size_t,
    ctypes.c_size_t,
    ctypes.c_size_t,
]

# void al_camera_free(struct al_camera *);
_al_camera_free = libal.al_camera_free
_al_camera_free.restype = None
_al_camera_free.argtypes = [
    ctypes.POINTER(_AlCamera),
]

# void al_camera_start(struct al_camera *);
_al_camera_start = libal.al_camera_start
_al_camera_start.restype = None
_al_camera_start.argtypes = [
    ctypes.POINTER(_AlCamera),
]

# void al_camera_stop(struct al_camera *);
_al_camera_stop = libal.al_camera_stop
_al_camera_stop.restype = None
_al_camera_stop.argtypes = [
    ctypes.POINTER(_AlCamera),
]

# enum al_status al_camera_get_id(struct al_camera *, const char **);
_al_camera_get_id = libal.al_camera_get_id
_al_camera_get_id.restype = ctypes.c_int
_al_camera_get_id.argtypes = [
    ctypes.POINTER(_AlCamera),
    ctypes.POINTER(ctypes.c_char_p),
]

# enum al_status al_camera_get_color_format(
#   struct al_camera *,
#   enum al_color_format *
# );
_al_camera_get_color_format = libal.al_camera_get_color_format
_al_camera_get_color_format.restype = ctypes.c_int
_al_camera_get_color_format.argtypes = [
    ctypes.POINTER(_AlCamera),
    ctypes.POINTER(ctypes.c_int),
]

# enum al_status al_camera_get_width(struct al_camera *, size_t *);
_al_camera_get_width = libal.al_camera_get_width
_al_camera_get_width.restype = ctypes.c_int
_al_camera_get_width.argtypes = [
    ctypes.POINTER(_AlCamera),
    ctypes.POINTER(ctypes.c_size_t),
]

# enum al_status al_camera_get_height(struct al_camera *, size_t *);
_al_camera_get_height = libal.al_camera_get_height
_al_camera_get_height.restype = ctypes.c_int
_al_camera_get_height.argtypes = [
    ctypes.POINTER(_AlCamera),
    ctypes.POINTER(ctypes.c_size_t),
]

# enum al_status al_camera_get_data(
#   struct al_camera *,
#   enum al_color_format,
#   void **
# );
_al_camera_get_data = libal.al_camera_get_data
_al_camera_get_data.restype = ctypes.c_int
_al_camera_get_data.argtypes = [
    ctypes.POINTER(_AlCamera),
    ctypes.c_int,
    ctypes.POINTER(ctypes.c_void_p),
]

# enum al_status al_camera_get_rgba(struct al_camera *, void **);
_al_camera_get_rgba = libal.al_camera_get_rgba
_al_camera_get_rgba.restype = ctypes.c_int
_al_camera_get_rgba.argtypes = [
    ctypes.POINTER(_AlCamera),
    ctypes.POINTER(ctypes.c_void_p),
]

# enum al_status al_camera_get_facing(
#   struct al_camera *,
#   enum al_camera_facing *
# );
_al_camera_get_facing = libal.al_camera_get_facing
_al_camera_get_facing.restype = ctypes.c_int
_al_camera_get_facing.argtypes = [
    ctypes.POINTER(_AlCamera),
    ctypes.POINTER(ctypes.c_int),
]

# enum al_status al_camera_get_orientation(struct al_camera *, int *);
_al_camera_get_orientation = libal.al_camera_get_orientation


class Camera:

    def __init__(self, index: int, width: int, height: int) -> None:

        self.index: int = 0
        self._cam: ctypes.POINTER(_AlCamera) = ctypes.POINTER(_AlCamera)()

        status = _al_camera_new(
            ctypes.byref(self._cam),
            index,
            width,
            height,
        )
        if not status == Status.OK:
            raise AlException(str(status))
        self.index = index

    def start(self) -> None:
        '''Start reading frames.'''
        assert self._cam
        _al_camera_start(self._cam)

    def stop(self) -> None:
        '''Stop reading frames.'''
        assert self._cam
        _al_camera_stop(self._cam)

    @property
    def id(self) -> str:
        '''Return an identifier for the camera device.'''
        assert self._cam
        id = ctypes.c_char_p()
        status = _al_camera_get_id(self._cam, ctypes.byref(id))
        if not status == Status.OK:
            raise AlException(str(status))
        return id.value.decode('utf-8')

    @property
    def color_format(self) -> ColorFormat:
        '''Return the camera's native color format.'''
        assert self._cam
        format = ctypes.c_int()
        status = _al_camera_get_color_format(self._cam, ctypes.byref(format))
        if not status == Status.OK:
            raise AlException(str(status))
        return ColorFormat(format.value)

    @property
    def width(self) -> int:
        '''Return the width of the frames in pixels.

        Returns zero if no frame has been read yet,
        and raises libal.AlException if there was an error.
        '''
        assert self._cam
        w = ctypes.c_size_t()
        status = _al_camera_get_width(self._cam, ctypes.byref(w))
        if not status == Status.OK:
            raise AlException(str(status))
        return w.value

    @property
    def height(self) -> int:
        '''Return the height of the frames in pixels.

        Returns zero if no frame has been read yet,
        and raises AlException if there was an error.
        '''
        assert self._cam
        h = ctypes.c_size_t()
        status = _al_camera_get_height(self._cam, ctypes.byref(h))
        if not status == Status.OK:
            raise AlException(str(status))
        return h.value

    @property
    def data(self) -> ctypes.c_void_p:
        '''Return the latest read frame, in the camera's native format.

            The native format is given by Camera.color_format.

            📝 Note: always check if the returned value is NULL (`c_void_p(0)`).
            This only returns a non-NULL pointer when a *new* frame is read.
            Before the first frame, and in between frames, this returns NULL.

            Keep a local copy of the result as this does not return
            the same frame twice.
        '''
        assert self._cam
        raise NotImplementedError

    @property
    def _data_size(self) -> int:
        if self.color_format == ColorFormat.YUV420P:
            size = self.width * self.height * 3 / 2
        elif self.color_format == ColorFormat.YUV420SP:
            size = self.width * self.height * 3 / 2
        elif self.color_format == ColorFormat.RGBA:
            size = self.width * self.height * 4
        else:
            raise AlExceptionUnsupportedColorFormat
        return int(size)

    @property
    def data_bytes(self) -> bytes:
        '''Similar to Camera.data but returns a read-only bytes object.'''
        p = self.data
        if not p:
            size = 0
        else:
            size = self._data_size
        return tobytes(p, size)

    @property
    def data_bytearray(self) -> bytearray:
        '''Similar to Camera.data but returns a read-write bytearray object.'''
        p = self.data
        if not p:
            size = 0
        else:
            size = self._data_size
        return tobytearray(p, size)

    @property
    def rgba(self) -> ctypes.c_void_p:
        '''Return the latest read frame, in RGBA format.

            See the notes for Camera.data.
        '''
        assert self._cam
        rgba = ctypes.c_void_p()
        status = _al_camera_get_rgba(self._cam, ctypes.byref(rgba))
        if not status == Status.OK:
            raise AlException(str(status))
        return rgba

    @property
    def rgba_bytes(self) -> bytes:
        '''Similar to Camera.rgba but returns a read-only bytes object.'''
        p = self.rgba
        if not p:
            size = 0
        else:
            size = self.width * self.height * 4
        return tobytes(p, size)

    @property
    def rgba_bytearray(self) -> bytearray:
        '''Similar to Camera.rgba but returns a read-write bytearray object.'''
        p = self.rgba
        if not p:
            size = 0
        else:
            size = self.width * self.height * 4
        return tobytearray(p, size)

    @property
    def facing(self) -> Facing:
        assert self._cam
        raise NotImplementedError

    def __repr__(self) -> str:
        return '<Camera {} id={} color_format={} width={} height={}>'.format(
            # self._cam,
            hex(ctypes.cast(self._cam, ctypes.c_void_p).value),
            self.id,
            str(self.color_format),
            self.width,
            self.height,
        )

    def __del__(self) -> None:
        self.stop()
        if self._cam:
            _al_camera_free(self._cam)
