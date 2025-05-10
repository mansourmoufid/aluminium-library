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

#pragma once

#include <errno.h>

#include <android/log.h>

#include <camera/NdkCameraError.h>

#include "common.h" // DEBUG()

static const char acamera_statuses[][64] = {
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_UNKNOWN] = "ACAMERA_ERROR_UNKNOWN",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_INVALID_PARAMETER] = "ACAMERA_ERROR_INVALID_PARAMETER",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_CAMERA_DISCONNECTED] = "ACAMERA_ERROR_CAMERA_DISCONNECTED",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_NOT_ENOUGH_MEMORY] = "ACAMERA_ERROR_NOT_ENOUGH_MEMORY",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_METADATA_NOT_FOUND] = "ACAMERA_ERROR_METADATA_NOT_FOUND",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_CAMERA_DEVICE] = "ACAMERA_ERROR_CAMERA_DEVICE",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_CAMERA_SERVICE] = "ACAMERA_ERROR_CAMERA_SERVICE",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_SESSION_CLOSED] = "ACAMERA_ERROR_SESSION_CLOSED",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_INVALID_OPERATION] = "ACAMERA_ERROR_INVALID_OPERATION",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_STREAM_CONFIGURE_FAIL] = "ACAMERA_ERROR_STREAM_CONFIGURE_FAIL",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_CAMERA_IN_USE] = "ACAMERA_ERROR_CAMERA_IN_USE",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_MAX_CAMERA_IN_USE] = "ACAMERA_ERROR_MAX_CAMERA_IN_USE",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_CAMERA_DISABLED] = "ACAMERA_ERROR_CAMERA_DISABLED",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_PERMISSION_DENIED] = "ACAMERA_ERROR_PERMISSION_DENIED",
    [ACAMERA_ERROR_BASE - ACAMERA_ERROR_UNSUPPORTED_OPERATION] = "ACAMERA_ERROR_UNSUPPORTED_OPERATION",
};

#define DEBUG_ACAMERA(function, status) \
    DEBUG( \
        "%s: %s", \
        function, \
        acamera_statuses[(ACAMERA_ERROR_BASE - status)] \
    )
