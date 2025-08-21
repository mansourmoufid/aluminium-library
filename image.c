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
#include <stdlib.h> // abort, calloc, posix_memalign
#include <string.h> // memcpy, memmove, strerror
#if defined(DEBUG)
#include <stdio.h>
#endif

#include "al.h"
#include "arithmetic.h" // _al_calc_next_multiple, SIZE_MAX_SQRT
#include "common.h"

enum al_status
al_image_alloc(struct al_image *x)
{
    assert(x != NULL);
    if (!(x->width > 0 && x->height > 0))
        return AL_ERROR;
    if (!(x->height < SIZE_MAX_SQRT && x->stride < SIZE_MAX_SQRT))
        return AL_ERROR;
    if (x->stride == 0)
        x->stride = _al_calc_next_multiple(x->width, 32);
    if (!(x->stride >= x->width))
        return AL_ERROR;
    switch (x->format) {
        case AL_COLOR_FORMAT_YUV420SP:
        case AL_COLOR_FORMAT_YUV420P:
        case AL_COLOR_FORMAT_RGBA:
            break;
        case AL_COLOR_FORMAT_UNKNOWN:
            return AL_ERROR;
    }
    if (x->data != NULL) {
        free(x->data);
        x->data = NULL;
    }
    size_t size = 0;
    switch (x->format) {
        case AL_COLOR_FORMAT_YUV420SP:
        case AL_COLOR_FORMAT_YUV420P:
            size = x->stride * x->height * 3 / 2 * sizeof (uint8_t);
            break;
        case AL_COLOR_FORMAT_RGBA:
            size = x->stride * x->height * sizeof (uint32_t);
            break;
        case AL_COLOR_FORMAT_UNKNOWN:
            abort();
    }
    assert(size > 0);
    void *data = NULL;
    if (posix_memalign(&data, 32, size) != 0) {
        DEBUG("posix_memalign: errno=%i [%s]", errno, strerror(errno));
        return AL_NOMEMORY;
    }
    x->data = data;
    return AL_OK;
}

void
al_image_free(struct al_image *x)
{
    assert(x != NULL);
    x->width = 0;
    x->height = 0;
    x->stride = 0;
    if (x->data != NULL)
        free(x->data);
    x->data = NULL;
    x->format = AL_COLOR_FORMAT_UNKNOWN;
}

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
    uint8_t *data = x->data;
    for (size_t i = 0; i < x->height; i++) {
        for (size_t j = 0; j < x->width; j++) {
            printf("\t%i", data[i * x->width + j]);
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
            return AL_NOTIMPLEMENTED;
    }

    return AL_OK;
}

static inline
enum al_status
_copy_yuv420sp(const struct al_image *src, struct al_image *dst)
{
    assert(src != NULL);
    assert(dst != NULL);
    if (src->data == NULL || dst->data == NULL)
        return AL_ERROR;
    uint8_t *src_data = src->data;
    uint8_t *dst_data = dst->data;
    memcpy(
        dst->data,
        src->data,
        src->height * src->stride
    );
    memcpy(
        &(dst_data[dst->height * dst->stride]),
        &(src_data[src->height * src->stride]),
        (src->height / 2) * src->stride * sizeof (uint8_t)
    );
    return AL_OK;
}

static inline
enum al_status
_copy_rgba(const struct al_image *src, struct al_image *dst)
{
    assert(src != NULL);
    assert(dst != NULL);
    if (src->data == NULL || dst->data == NULL)
        return AL_ERROR;
    uint8_t *src_data = src->data;
    uint8_t *dst_data = dst->data;
    for (size_t i = 0; i < src->height; i++) {
        memcpy(
            &(dst_data[i * dst->stride]),
            &(src_data[i * src->stride]),
            dst->width * sizeof (uint32_t)
        );
    }
    return AL_OK;
}

enum al_status
al_image_copy(const struct al_image *src, struct al_image *dst)
{
    assert(src != NULL);
    assert(dst != NULL);
    assert(src->format == dst->format);
    switch (src->format) {
        case AL_COLOR_FORMAT_RGBA:
            return _copy_rgba(src, dst);
        case AL_COLOR_FORMAT_YUV420SP:
            return _copy_yuv420sp(src, dst);
        case AL_COLOR_FORMAT_YUV420P:
        case AL_COLOR_FORMAT_UNKNOWN:
            return AL_NOTIMPLEMENTED;
    }
}

#if defined(TEST)

#include <stdint.h> // uint8_t
#include <stdlib.h> // calloc

#include "test.h"

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
        .data = calloc(x.height, x.stride * sizeof (uint8_t)),
    };
    _al_dump(&x);
    enum al_status status = al_image_rotate(&x, &y, 90);
    _al_dump(&y);
    dump_status(status);
    assert(status == AL_OK);

    return 0;
}

#endif
