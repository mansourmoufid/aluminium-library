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

#include <stddef.h>

enum al_status {
    AL_OK = 0,
    AL_ERROR = 1,
    AL_NOTIMPLEMENTED = 2,
};

enum al_color_format {
    AL_COLOR_FORMAT_UNKNOWN = 0,
    AL_COLOR_FORMAT_YUV420SP = 1,
    AL_COLOR_FORMAT_YUV420P = 2,
    AL_COLOR_FORMAT_RGBA = 3,
};

enum al_camera_facing {
    AL_CAMERA_FACING_FRONT = 0,
    AL_CAMERA_FACING_BACK = 1,
};

__attribute__((constructor)) void init(void);

extern const char *const copyright;
extern const char *const platform;
const char *al_locale(void);
const char *al_datadir(void);
const char *al_libdir(void);

int al_permissions_have(const char *);
void al_permissions_request(const char *);

int al_display_orientation(void);

char *al_net_get_local_ip_address(void);

struct al_camera;
enum al_status al_camera_new(struct al_camera **, size_t, size_t, size_t);
void al_camera_free(struct al_camera *);
void al_camera_start(struct al_camera *);
void al_camera_stop(struct al_camera *);
enum al_status al_camera_get_id(struct al_camera *, const char **);
enum al_status al_camera_get_color_format(struct al_camera *, enum al_color_format *);
enum al_status al_camera_get_width(struct al_camera *, size_t *);
enum al_status al_camera_get_height(struct al_camera *, size_t *);
enum al_status al_camera_get_data(struct al_camera *, enum al_color_format, void **);
enum al_status al_camera_get_rgba(struct al_camera *, void **);
enum al_status al_camera_get_facing(struct al_camera *, enum al_camera_facing *);
enum al_status al_camera_get_orientation(struct al_camera *, int *);
enum al_status al_camera_set_stride(struct al_camera *, size_t);

struct al_image {
    size_t width;
    size_t height;
    size_t stride;
    void *restrict data;
    enum al_color_format format;
};
enum al_status al_image_alloc(struct al_image *);
void al_image_free(struct al_image *);
enum al_status al_image_convert(struct al_image *, struct al_image *);
enum al_status al_image_rotate(struct al_image *, struct al_image *, int);
enum al_status al_image_copy(struct al_image *, struct al_image *);
