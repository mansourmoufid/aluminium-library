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

#if defined(DEBUG)
#undef NDEBUG
#endif

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h> // calloc
#include <string.h> // memmove
#if defined(DEBUG)
#include <stdio.h>
#endif

#include "al.h"

struct yuv {
    size_t width;
    size_t height;
    size_t stride;
    uint8_t *restrict y;
    uint8_t *u;
    uint8_t *v;
};

#if defined(DEBUG)
static inline
void
_al_dump(struct al_image *x)
{
    for (size_t i = 0; i < x->height; i++) {
        for (size_t j = 0; j < x->width; j++) {
            printf("\t%i", x->data[i * x->width + j]);
        }
        printf("\n");
    }
}
#endif

static
void
_rotate_yuv420sp(
    uint8_t *restrict src_y,
    uint8_t *restrict src_uv,
    size_t src_width,
    size_t src_height,
    size_t src_stride,
    uint8_t *restrict dst_y,
    uint8_t *restrict dst_uv,
    size_t dst_width,
    size_t dst_height,
    size_t dst_stride,
    int degrees
) {
    const struct yuv x = {
        .width = src_width,
        .height = src_height,
        .stride = src_stride,
        .y = src_y,
        .u = src_uv,
    };
    const struct yuv y = {
        .width = dst_width,
        .height = dst_height,
        .stride = dst_stride,
        .y = dst_y,
        .u = dst_uv,
    };
    switch (degrees) {
        case 0:
            memmove(y.y, x.y, x.height * x.stride);
            memmove(y.u, x.u, (x.height / 2) * x.width);
            break;
        case 90:
            for (size_t i = 0; i < x.height; i++) {
                for (size_t j = 0; j < x.width; j++) {
                    const size_t i_ = j;
                    const size_t j_ = (x.height - 1) - i;
                    y.y[i_ * y.stride + j_] = x.y[i * x.stride + j];
                }
            }
            break;
        default:
            break;
    }
}

enum al_status
al_image_rotate(struct al_image *src, struct al_image *dst, int degrees)
{
    assert(src != NULL);
    assert(dst != NULL);
    if (src == dst) {
        assert(src->width == src->height);
        assert(dst->width == dst->height);
    }
    degrees = degrees % 360;
    assert(degrees % 90 == 0);
    switch (degrees) {
        case 0:
        case 180:
            assert(src->width == dst->width);
            assert(src->height == dst->height);
            break;
        case 90:
        case 270:
            assert(src->width == dst->height);
            assert(src->height == dst->width);
            break;
        default:
            break;
    }
    assert(src->data != NULL);
    assert(dst->data != NULL);
    assert(src->format == dst->format);

    switch (src->format) {
        case AL_COLOR_FORMAT_YUV420SP:
            _rotate_yuv420sp(
                src->data,
                (uint8_t *) src->data + src->height * src->stride,
                src->width,
                src->height,
                src->stride,
                dst->data,
                (uint8_t *) dst->data + dst->height * dst->stride,
                dst->width,
                dst->height,
                dst->stride,
                degrees
            );
            break;
        case AL_COLOR_FORMAT_YUV420P:
        case AL_COLOR_FORMAT_RGBA:
        case AL_COLOR_FORMAT_UNKNOWN:
            break;
    }

    return AL_OK;
}

#if defined(DEBUG)
int
main(void)
{
    const uint8_t data[][3] = {
        { 1,  2,  3},
        { 4,  5,  6},
        { 7,  8,  9},
        {10, 11, 12},
    };
    struct al_image x = {
        .width = 3,
        .height = 4,
        .stride = 3,
        .data = data,
    };
    struct al_image y = {
        .width = x.height,
        .height = x.width,
        .stride = x.height,
        .data = calloc(x.height, x.width),
    };
    _al_dump(&x);
    assert(al_image_rotate(&x, &y, 90) == AL_OK);
    _al_dump(&y);
    return 0;
}
#endif






















