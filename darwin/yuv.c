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

#include <assert.h>
#include <limits.h> // UINT8_MAX, ULLONG_MAX
#include <stdlib.h> // free

#include <Accelerate/Accelerate.h> // vImage_Buffer

#include <CoreGraphics/CGGeometry.h> // CGSize

#include <CoreVideo/CVPixelBuffer.h> // kCVPixelFormatType_*

#include "al.h"
#include "common.h"
#include "yuv.h"

// typedef unsigned long vImagePixelCount;
#define VIMAGEPIXELCOUNT_MAX ((vImagePixelCount) ULLONG_MAX)
#define VIMAGEPIXELCOUNT_MAX_SQRT \
    (((vImagePixelCount) 1) << (8 * sizeof(vImagePixelCount) / 2))

enum al_status
_al_darwin_yuv_to_rgba(
    CVImageBufferRef image_buffer,
    vImage_Buffer image_buffers[NUM_IMAGE_BUFFERS]
) {
    assert(image_buffer != NULL);
    assert(image_buffers != NULL);

    vImage_Flags flags = kvImageNoFlags;
#if !defined(NDEBUG)
    flags |= kvImagePrintDiagnosticsToConsole;
#endif

    CGSize size = CVImageBufferGetEncodedSize(image_buffer);
    vImagePixelCount height = (vImagePixelCount) size.height;
    vImagePixelCount width = (vImagePixelCount) size.width;
    // DEBUG("height = %lu", height);
    // DEBUG("width = %lu", width);
    assert(height < VIMAGEPIXELCOUNT_MAX_SQRT);
    assert(width < VIMAGEPIXELCOUNT_MAX_SQRT);

    if (image_buffers[RGBA].data == NULL) {
        vImage_Error error = vImageBuffer_Init(
            &(image_buffers[RGBA]),
            height,
            width,
            8 * 4 /* bits */,
            flags
        );
        if (error != kvImageNoError) {
            DEBUG_VIMAGE("vImageBuffer_Init", error);
            goto error;
        }
    }

    OSType image_format = CVPixelBufferGetPixelFormatType(image_buffer);
    if (image_format == color_formats[AL_COLOR_FORMAT_RGBA]) {
        vImage_Error error = vImageCopyBuffer(
            &((vImage_Buffer) {
                .data = CVPixelBufferGetBaseAddress(image_buffer),
                .height = CVPixelBufferGetHeight(image_buffer),
                .width = CVPixelBufferGetWidth(image_buffer),
                .rowBytes = CVPixelBufferGetBytesPerRow(image_buffer),
            }),
            &(image_buffers[RGBA]),
            4, /* bytes per pixel */
            kvImageNoFlags
        );
        if (error == kvImageNoError)
            goto done;
        else
            goto error;
    }

    vImage_YpCbCrPixelRange pixel_range = {0};
    switch (image_format) {
        case kCVPixelFormatType_420YpCbCr8Planar:
            pixel_range = (vImage_YpCbCrPixelRange) {
                .Yp_bias = 16,
                .CbCr_bias = 128,
                .YpRangeMax = 235,
                .CbCrRangeMax = 240,
                .YpMax = 255,
                .YpMin = 0,
                .CbCrMax = 255,
                .CbCrMin = 1,
            };
            break;
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_Lossy_420YpCbCr8BiPlanarVideoRange:
            pixel_range = (vImage_YpCbCrPixelRange) {
                .Yp_bias = 16,
                .CbCr_bias = 128,
                .YpRangeMax = 265,
                .CbCrRangeMax = 240,
                .YpMax = 235,
                .YpMin = 16,
                .CbCrMax = 240,
                .CbCrMin = 16,
            };
            break;
        case kCVPixelFormatType_420YpCbCr8PlanarFullRange:
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
        case kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarFullRange:
        case kCVPixelFormatType_Lossy_420YpCbCr8BiPlanarFullRange:
            pixel_range = (vImage_YpCbCrPixelRange) {
                .Yp_bias = 0,
                .CbCr_bias = 128,
                .YpRangeMax = 255,
                .CbCrRangeMax = 255,
                .YpMax = 255,
                .YpMin = 1,
                .CbCrMax = 255,
                .CbCrMin = 0,
            };
            break;
        default:
            goto error;
    }

    vImageYpCbCrType type = 0;
    switch (image_format) {
        case kCVPixelFormatType_420YpCbCr8Planar:
        case kCVPixelFormatType_420YpCbCr8PlanarFullRange:
            type = kvImage420Yp8_Cb8_Cr8;
            break;
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
        case kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarFullRange:
        case kCVPixelFormatType_Lossy_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_Lossy_420YpCbCr8BiPlanarFullRange:
            type = kvImage420Yp8_CbCr8;
            break;
        default:
            goto error;
    }

    vImage_YpCbCrToARGB conversion_info = {0};
    vImage_Error error = vImageConvert_YpCbCrToARGB_GenerateConversion(
        // kvImage_YpCbCrToARGBMatrix_ITU_R_601_4,
        kvImage_YpCbCrToARGBMatrix_ITU_R_709_2,
        &pixel_range,
        &conversion_info,
        type,
        kvImageARGB8888,
        kvImageNoFlags
    );
    if (error != kvImageNoError) {
        DEBUG_VIMAGE("vImageConvert_YpCbCrToARGB_GenerateConversion", error);
        goto error;
    }

    image_buffers[Y] = (vImage_Buffer) {
        .data = CVPixelBufferGetBaseAddressOfPlane(image_buffer, 0),
        .height = CVPixelBufferGetHeightOfPlane(image_buffer, 0),
        .width = CVPixelBufferGetWidthOfPlane(image_buffer, 0),
        .rowBytes = CVPixelBufferGetBytesPerRowOfPlane(image_buffer, 0),
    };
    if (type == kvImage420Yp8_Cb8_Cr8) {
        image_buffers[U] = (vImage_Buffer) {
            .data = CVPixelBufferGetBaseAddressOfPlane(image_buffer, 1),
            .height = CVPixelBufferGetHeightOfPlane(image_buffer, 1),
            .width = CVPixelBufferGetWidthOfPlane(image_buffer, 1),
            .rowBytes = CVPixelBufferGetBytesPerRowOfPlane(image_buffer, 1),
        };
        image_buffers[V] = (vImage_Buffer) {
            .data = CVPixelBufferGetBaseAddressOfPlane(image_buffer, 2),
            .height = CVPixelBufferGetHeightOfPlane(image_buffer, 2),
            .width = CVPixelBufferGetWidthOfPlane(image_buffer, 2),
            .rowBytes = CVPixelBufferGetBytesPerRowOfPlane(image_buffer, 2),
        };
        error = vImageConvert_420Yp8_Cb8_Cr8ToARGB8888(
            &(image_buffers[Y]),
            &(image_buffers[U]),
            &(image_buffers[V]),
            &(image_buffers[RGBA]),
            &conversion_info,
            (const uint8_t [4]) {1, 2, 3, 0},
            UINT8_MAX,
            flags
        );
        if (error != kvImageNoError) {
            DEBUG_VIMAGE("vImageConvert_420Yp8_Cb8_Cr8ToARGB8888", error);
            goto error;
        }
    } else if (type == kvImage420Yp8_CbCr8) {
        image_buffers[U] = (vImage_Buffer) {
            .data = CVPixelBufferGetBaseAddressOfPlane(image_buffer, 1),
            .height = CVPixelBufferGetHeightOfPlane(image_buffer, 1),
            .width = CVPixelBufferGetWidthOfPlane(image_buffer, 1),
            .rowBytes = CVPixelBufferGetBytesPerRowOfPlane(image_buffer, 1),
        };
        error = vImageConvert_420Yp8_CbCr8ToARGB8888(
            &(image_buffers[Y]),
            &(image_buffers[U]),
            &(image_buffers[RGBA]),
            &conversion_info,
            (const uint8_t [4]) {1, 2, 3, 0},
            UINT8_MAX,
            flags
        );
        if (error != kvImageNoError) {
            DEBUG_VIMAGE("vImageConvert_420Yp8_CbCr8ToARGB8888", error);
            goto error;
        }
    }

done:
    return AL_OK;

error:
    free(image_buffers[RGBA].data);
    image_buffers[RGBA].data = NULL;
    for (enum image_buffer_index i = 0; i < NUM_IMAGE_BUFFERS; i++) {
        image_buffers[i] = (vImage_Buffer) {0};
    }
    return AL_ERROR;
}
