/* Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com> */

#pragma once

typedef void (al_yuv_to_rgb_t)(
    const uint8_t *restrict,
    const uint8_t *,
    const uint8_t *,
    uint32_t *restrict,
    const size_t,
    const size_t,
    const size_t,
    const size_t,
    const size_t,
    const size_t
);

al_yuv_to_rgb_t al_yuv_to_rgba;

typedef void (al_yuv_to_yuv_t)(
    const uint8_t *restrict,
    uint8_t *restrict,
    const size_t,
    const size_t
);

al_yuv_to_yuv_t al_yuv_nv12_to_i420;
al_yuv_to_yuv_t al_yuv_i420_to_nv12;
