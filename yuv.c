/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

/* Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com> */

#if defined(DEBUG)
#undef NDEBUG
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#if defined(DEBUG)
#include <stdio.h>
#include <string.h>
#endif

#include "yuv.h"

static inline
__attribute__((const))
int32_t
min(int32_t a, int32_t b)
{
    if (a < b)
        return a;
    else
        return b;
}

static inline
__attribute__((const))
int32_t
max(int32_t a, int32_t b)
{
    if (a > b)
        return a;
    else
        return b;
}

static inline
__attribute__((const))
uint32_t
yuv_to_rgb(int32_t y, int32_t u, int32_t v)
{
    y -= 16;
    u -= 128;
    v -= 128;
    if (y < 0)
        y = 0;
    int32_t r = 1192 * y + 1634 * v;
    int32_t g = 1192 * y - 833 * v - 400 * u;
    int32_t b = 1192 * y + 2066 * u;
    int32_t a = 0xff;
    r = min(262143, max(0, r));
    g = min(262143, max(0, g));
    b = min(262143, max(0, b));
    r = (r >> 10) & 0xff;
    g = (g >> 10) & 0xff;
    b = (b >> 10) & 0xff;
    return (uint32_t) ((a << 24) | (b << 16) | (g << 8) | r);
}

void
al_yuv_to_rgba(
    const uint8_t *restrict y_data,
    const uint8_t *u_data,
    const uint8_t *v_data,
    uint32_t *restrict output,
    const size_t width,
    const size_t height,
    const size_t y_stride,
    const size_t uv_stride,
    const size_t y_pixel_stride,
    const size_t uv_pixel_stride
) {
    assert(y_data != NULL);
    assert(u_data != NULL);
    assert(v_data != NULL);
    assert(output != NULL);
    assert(y_pixel_stride == 1);
    assert(uv_pixel_stride == 1 || uv_pixel_stride == 2);
    if (uv_pixel_stride == 1) {
        for (size_t i = 0; i < height; i++) {
            uint32_t *out = output + i * width;
            const uint8_t *y = y_data + i * y_stride;
            const uint8_t *u = u_data + (i / 2) * uv_stride;
            const uint8_t *v = v_data + (i / 2) * uv_stride;
            for (size_t j = 0; j < width / 2; j++) {
                *out++ = yuv_to_rgb(*y++, *u, *v);
                *out++ = yuv_to_rgb(*y++, *u, *v);
                u += 1;
                v += 1;
            }
        }
    } else if (uv_pixel_stride == 2) {
        for (size_t i = 0; i < height; i++) {
            uint32_t *out = output + i * width;
            const uint8_t *y = y_data + i * y_stride;
            const uint8_t *u = u_data + (i / 2) * uv_stride;
            const uint8_t *v = v_data + (i / 2) * uv_stride;
            for (size_t j = 0; j < width / 2; j++) {
                *out++ = yuv_to_rgb(*y++, *u, *v);
                *out++ = yuv_to_rgb(*y++, *u, *v);
                u += 2;
                v += 2;
            }
        }
    }
}

#if !defined(DEBUG)
#define _al_debug_buffer(...) 
#else
static inline
void
_al_debug_buffer(const uint8_t *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        printf("%02u ", buffer[i]);
    }
    printf("\n");
}
#endif

/*
 *  NV12                →   I420
 *
 *  Y0Y1Y2Y3Y4Y5Y6Y7        Y0Y1Y2Y3Y4Y5Y6Y7
 *  Y8Y9᠁                   Y8Y9᠁
 *  U0V0U1V1U2V2U3V3        U0U1U2U3
 *                          V0V1V2V3
 */
void
al_yuv_nv12_to_i420(
    const uint8_t *restrict nv12_data,
    uint8_t *restrict i420_data,
    const size_t width,
    const size_t height
) {
    assert(nv12_data != NULL);
    assert(i420_data != NULL);
    for (size_t i = 0; i < height; i++) {
        const uint8_t *y_nv12 = nv12_data + width * i;
        const uint8_t *uv_nv12 = nv12_data + width * height;
        const uint8_t *u_nv12 = uv_nv12 + width * (i / 2);
        const uint8_t *v_nv12 = uv_nv12 + width * (i / 2) + 1;
        uint8_t *y_i420 = i420_data + width * i;
        uint8_t *uv_i420 = i420_data + width * height;
        uint8_t *u_i420 = uv_i420 + (width / 2) * (i / 2);
        uint8_t *v_i420 = uv_i420 + (width / 2) * (height / 2 + i / 2);
        for (size_t j = 0; j < width / 2; j++) {
            _al_debug_buffer(i420_data, width * height * 3 / 2);
            *y_i420++ = *y_nv12++;
            *y_i420++ = *y_nv12++;
            *u_i420++ = *u_nv12++; u_nv12++;
            *v_i420++ = *v_nv12++; v_nv12++;
            _al_debug_buffer(i420_data, width * height * 3 / 2);
        }
    }
}

/*
 *  NV12                ←   I420
 *
 *  Y0Y1Y2Y3Y4Y5Y6Y7        Y0Y1Y2Y3Y4Y5Y6Y7
 *  Y8Y9᠁                   Y8Y9᠁
 *  U0V0U1V1U2V2U3V3        U0U1U2U3
 *                          V0V1V2V3
 */
void
al_yuv_i420_to_nv12(
    const uint8_t *restrict i420_data,
    uint8_t *restrict nv12_data,
    const size_t width,
    const size_t height
) {
    assert(nv12_data != NULL);
    assert(i420_data != NULL);
    for (size_t i = 0; i < height; i++) {
        const uint8_t *y_i420 = i420_data + width * i;
        const uint8_t *uv_i420 = i420_data + width * height;
        const uint8_t *u_i420 = uv_i420 + (width / 2) * (i / 2);
        const uint8_t *v_i420 = uv_i420 + (width / 2) * (height / 2 + i / 2);
        uint8_t *y_nv12 = nv12_data + width * i;
        uint8_t *uv_nv12 = nv12_data + width * height;
        uint8_t *u_nv12 = uv_nv12 + width * (i / 2);
        uint8_t *v_nv12 = uv_nv12 + width * (i / 2) + 1;
        for (size_t j = 0; j < width / 2; j++) {
            _al_debug_buffer(nv12_data, width * height * 3 / 2);
            *y_nv12++ = *y_i420++;
            *y_nv12++ = *y_i420++;
            *u_nv12++ = *u_i420++; u_nv12++;
            *v_nv12++ = *v_i420++; v_nv12++;
            _al_debug_buffer(nv12_data, width * height * 3 / 2);
        }
    }
}

#if defined(DEBUG)
int
main(void)
{
    const uint8_t nv12[] = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16,
        17, 18, 19, 20,
        21, 22, 23, 24,
    };
    const uint8_t i420[] = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16,
        17, 19,
        21, 23,
        18, 20,
        22, 24,
    };
    uint8_t buffer[32];
    memset(buffer, 0, sizeof(buffer));
    al_nv12_to_i420(nv12, buffer, 4, 4);
    assert(memcmp(buffer, i420, sizeof(i420)) == 0);
    memset(buffer, 0, sizeof(buffer));
    al_i420_to_nv12(i420, buffer, 4, 4);
    assert(memcmp(buffer, nv12, sizeof(nv12)) == 0);
    return 0;
}
#endif
