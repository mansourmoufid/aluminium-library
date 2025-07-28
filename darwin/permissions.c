/* Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com> */

/*
 * This file is part of Aluminium Library.
 *
 * Aluminium Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Aluminium Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with Aluminium Library. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include "al.h"
#include "camera.h"

bool
al_permissions_have(const char *permission)
{
    if (permission == NULL)
        return false;
    if (strcmp(permission, "com.apple.security.device.camera") == 0)
        return _al_camera_have_authorization();
    return false;
}

void
al_permissions_request(const char *permission)
{
    if (permission == NULL)
        return;
    if (strcmp(permission, "com.apple.security.device.camera") == 0)
        _al_camera_request_authorization();
}
