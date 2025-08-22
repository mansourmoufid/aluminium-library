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


'''The Aluminium Library networking module.'''


__all__ = [
    'get_local_ip_address',
]


import ctypes
import typing

from .. import libal


# char *al_net_get_local_ip_address(void);
_al_net_get_local_ip_address = libal.al_net_get_local_ip_address
_al_net_get_local_ip_address.restype = ctypes.c_char_p
_al_net_get_local_ip_address.argtypes = []


def get_local_ip_address() -> typing.Optional[str]:
    '''Return the local IP address.'''
    ip = _al_net_get_local_ip_address()
    if ip:
        return ip.decode('utf-8')
    return None
