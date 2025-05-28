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

#include <Accelerate/Accelerate.h> // vImage_Buffer

#include <CoreVideo/CVImageBuffer.h> // CVImageBufferRef
#include <CoreVideo/CVPixelBuffer.h> // kCVPixelFormatType_*

#include "al.h"

static const OSType color_formats[] = {
    [AL_COLOR_FORMAT_YUV420P] = kCVPixelFormatType_420YpCbCr8Planar,
    [AL_COLOR_FORMAT_YUV420SP] = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange,
    [AL_COLOR_FORMAT_RGBA] = kCVPixelFormatType_32BGRA,
};

enum image_buffer_index {
    Y,
    U,
    V,
    RGBA,
};

#define NUM_IMAGE_BUFFERS 4

enum al_status
_al_darwin_yuv_to_rgba(
    CVImageBufferRef,
    vImage_Buffer [NUM_IMAGE_BUFFERS]
);
