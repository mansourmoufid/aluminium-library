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
#include <errno.h>
#include <float.h> // FLT_MAX
#include <inttypes.h> // PRIi64
#include <limits.h> // INT32_MAX
#include <stdatomic.h> // atomic_bool
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h> // int32_t, int64_t
#include <stdlib.h> // calloc
#include <string.h> // strdup, strcmp
#include <strings.h> // strcasecmp
#include <unistd.h> // usleep

#include <android/log.h>
#include <android/native_window.h> // ANativeWindow

#include <camera/NdkCameraCaptureSession.h> // ACameraCaptureSession
#include <camera/NdkCameraDevice.h> // ACameraIdList, ACameraDevice
#include <camera/NdkCameraError.h> // camera_status_t
#include <camera/NdkCameraManager.h> // ACameraManager
#include <camera/NdkCameraMetadata.h> // ACameraMetadata
#include <camera/NdkCameraMetadataTags.h>

#include <media/NdkImageReader.h> // AImageReader
#include <media/NdkMediaError.h> // media_status_t

#include "al.h"

#include "arithmetic.h" // _al_calc_next_multiple, _al_l2norm
#include "camera.h" // DEBUG_ACAMERA
#include "common.h" // DEBUG, DEBUG_AMEDIA, COLOR_Format*
#include "mediacodec.h"
#include "yuv.h"

struct metadata {
    uint8_t facing;
    int32_t orientation;
    int32_t width;
    int32_t height;
};

struct al_camera {
    size_t index;
    ACameraManager *manager;
    ACameraDevice *device;
    char *id;
    struct metadata metadata;
    ACameraManager_AvailabilityCallbacks *availability_callbacks;
    ACameraDevice_StateCallbacks *state_callbacks;
    ACameraCaptureSession_stateCallbacks *session_callbacks;
    ACameraCaptureSession_captureCallbacks *capture_callbacks;
    AImageReader *reader;
    AImageReader_ImageListener *listener;
    ANativeWindow *window;
    ACameraCaptureSession *session;
    ACaptureRequest *request;
    size_t width;
    size_t height;
    size_t stride;
    int32_t image_format;
    int color_format;
    struct al_image yuv420p; // NV12
    struct al_image yuv420sp; // I420
    struct al_image rgba;
    struct al_image image;
    atomic_bool read;
    atomic_bool stop;
};

#define N_CAMERAS 64
static struct al_camera *_al_cameras[N_CAMERAS] = {0};

static
void
on_available(void *context, const char *id)
{
    DEBUG("onCameraAvailable(context=%p, id=%s)", context, id);
}

static
void
on_unavailable(void *context, const char *id)
{
    DEBUG("onCameraUnavailable(context=%p, id=%s)", context, id);
}

static
void
process_image(struct al_camera *cam, AImage *image)
{
    assert(cam != NULL);
    assert(image != NULL);

    int32_t format = 0;
    int32_t y_stride = 0;
    int32_t uv_stride = 0;
    uint8_t *y_pixel = NULL;
    uint8_t *u_pixel = NULL;
    uint8_t *v_pixel = NULL;
    int32_t y_len = 0;
    int32_t u_len = 0;
    int32_t v_len = 0;
    int32_t y_pixel_stride = 0;
    int32_t uv_pixel_stride = 0;
    media_status_t status;
    enum al_status status2;

    int32_t w = 0;
    int32_t h = 0;
    status = AImage_getWidth(image, &w);
    if (status != AMEDIA_OK) {
        DEBUG_AMEDIA("AImage_getWidth", status);
        goto error;
    }
    assert(w > 0 && (size_t) w == cam->width);
    status = AImage_getHeight(image, &h);
    if (status != AMEDIA_OK) {
        DEBUG_AMEDIA("AImage_getHeight", status);
        goto error;
    }
    assert(h > 0 && (size_t) h == cam->height);
    size_t width = (size_t) w;
    size_t height = (size_t) h;

    status = AImage_getFormat(image, &format);
    if (status != AMEDIA_OK) {
        DEBUG_AMEDIA("AImage_getFormat", status);
        goto error;
    }
    switch (format) {
        case AIMAGE_FORMAT_YUV_420_888:
            status = AImage_getPlaneRowStride(image, 0, &y_stride);
            if (status != AMEDIA_OK) {
                DEBUG_AMEDIA("AImage_getPlaneRowStride", status);
                goto error;
            }
            assert(y_stride > 0);
            status = AImage_getPlaneRowStride(image, 1, &uv_stride);
            if (status != AMEDIA_OK) {
                DEBUG_AMEDIA("AImage_getPlaneRowStride", status);
                goto error;
            }
            assert(uv_stride > 0);
            status = AImage_getPlaneData(image, 0, &y_pixel, &y_len);
            if (status != AMEDIA_OK) {
                DEBUG_AMEDIA("AImage_getPlaneData", status);
                goto error;
            }
            assert(y_pixel != NULL && y_len > 0);
            status = AImage_getPlaneData(image, 1, &u_pixel, &u_len);
            if (status != AMEDIA_OK) {
                DEBUG_AMEDIA("AImage_getPlaneData", status);
                goto error;
            }
            assert(u_pixel != NULL && u_len > 0);
            status = AImage_getPlaneData(image, 2, &v_pixel, &v_len);
            if (status != AMEDIA_OK) {
                DEBUG_AMEDIA("AImage_getPlaneData", status);
                goto error;
            }
            assert(v_pixel != NULL && v_len > 0);
            assert(v_len == u_len);
            status = AImage_getPlanePixelStride(image, 0, &y_pixel_stride);
            if (status != AMEDIA_OK) {
                DEBUG_AMEDIA("AImage_getPlanePixelStride", status);
                goto error;
            }
            assert(y_pixel_stride == 1);
            status = AImage_getPlanePixelStride(image, 1, &uv_pixel_stride);
            if (status != AMEDIA_OK) {
                goto error;
                DEBUG_AMEDIA("AImage_getPlanePixelStride", status);
            }
            assert(uv_pixel_stride == 1 || uv_pixel_stride == 2);
            break;
        default:
            goto error;
    }

    if (cam->yuv420sp.width < width || cam->yuv420sp.height < height)
        al_image_free(&cam->yuv420sp);
    if (cam->yuv420sp.data == NULL) {
        cam->yuv420sp.width = width;
        cam->yuv420sp.height = height;
        cam->yuv420sp.stride = width;
        cam->yuv420sp.format = AL_COLOR_FORMAT_YUV420SP;
        status2 = al_image_alloc(&cam->yuv420sp);
        assert(status2 == AL_OK);
    }
    if (cam->yuv420p.width < width || cam->yuv420p.height < height)
        al_image_free(&cam->yuv420p);
    if (cam->yuv420p.data == NULL) {
        cam->yuv420p.width = width;
        cam->yuv420p.height = height;
        cam->yuv420p.stride = width;
        cam->yuv420p.format = AL_COLOR_FORMAT_YUV420P;
        status2 = al_image_alloc(&cam->yuv420p);
        assert(status2 == AL_OK);
    }
    if (cam->rgba.width < width || cam->rgba.height < height)
        al_image_free(&cam->rgba);
    if (cam->rgba.data == NULL) {
        cam->rgba.width = width;
        cam->rgba.height = height;
        cam->rgba.stride = width;
        cam->rgba.format = AL_COLOR_FORMAT_RGBA;
        status2 = al_image_alloc(&cam->rgba);
        assert(status2 == AL_OK);
    }

    switch (format) {
        case AIMAGE_FORMAT_YUV_420_888:
            switch (uv_pixel_stride) {
                case 1:
                    cam->color_format = COLOR_FormatYUV420Planar;
                    break;
                case 2:
                    cam->color_format = COLOR_FormatYUV420SemiPlanar;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    switch (cam->color_format) {
        case COLOR_FormatYUV420Planar:
            // YV12 to I420
            {
            uint8_t *const y = cam->yuv420p.data;
            for (size_t i = 0; i < height; i++) {
                memcpy(&(y[i * width]), &(y_pixel[i * y_stride]), width);
            }
            uint8_t *const u = (uint8_t *) cam->yuv420p.data + height * width;
            uint8_t *const v = u + (height / 2) * (width / 2);
            for (size_t i = 0; i < height / 2; i++) {
                memcpy(&(u[i * width / 2]), &(u_pixel[i * uv_stride]), width / 2);
                memcpy(&(v[i * width / 2]), &(v_pixel[i * uv_stride]), width / 2);
            }
            }
            break;
        case COLOR_FormatYUV420SemiPlanar:
            // NV21 to NV12
            {
            for (size_t i = 0; i < height; i++) {
                uint8_t *y = cam->yuv420sp.data;
                memcpy(&(y[i * width]), &(y_pixel[i * y_stride]), width);
            }
            uint8_t *uv = (uint8_t *) cam->yuv420sp.data + height * width;
            assert(u_pixel == v_pixel + 1);
            for (size_t i = 0; i < height / 2; i++) {
                for (size_t j = 0; j < width / 2; j++) {
                    uv[i * width + j * 2 + 0] = u_pixel[i * uv_stride + j * 2];
                    uv[i * width + j * 2 + 1] = v_pixel[i * uv_stride + j * 2];
                }
            }
            }
            break;
        default:
            break;
    }

    switch (cam->color_format) {
        case COLOR_FormatYUV420Planar:
            cam->image.format = AL_COLOR_FORMAT_YUV420P;
            break;
        case COLOR_FormatYUV420SemiPlanar:
            cam->image.format = AL_COLOR_FORMAT_YUV420SP;
            break;
        default:
            break;
    }

    if (cam->image.width < width || cam->image.height < height)
        al_image_free(&cam->image);
    if (cam->image.data == NULL) {
        status2 = al_image_alloc(&cam->image);
        assert(status2 == AL_OK);
    }

    switch (cam->color_format) {
        case COLOR_FormatYUV420Planar:
            status2 = al_image_copy(
                &((const struct al_image) {
                    .width = cam->width,
                    .height = cam->height,
                    .stride = cam->width,
                    .data = cam->yuv420p.data,
                    .format = AL_COLOR_FORMAT_YUV420P,
                }),
                &cam->image
            );
            assert(status2 == AL_OK);
            break;
        case COLOR_FormatYUV420SemiPlanar:
            status2 = al_image_copy(
                &((const struct al_image) {
                    .width = cam->width,
                    .height = cam->height,
                    .stride = cam->width,
                    .data = cam->yuv420sp.data,
                    .format = AL_COLOR_FORMAT_YUV420SP,
                }),
                &cam->image
            );
            assert(status2 == AL_OK);
            break;
        default:
            break;
    }

    switch (cam->color_format) {
        case COLOR_FormatYUV420Planar:
        case COLOR_FormatYUV420SemiPlanar:
            al_yuv_to_rgba(
                y_pixel,
                u_pixel,
                v_pixel,
                cam->rgba.data,
                cam->image.width,
                cam->image.height,
                y_stride,
                uv_stride,
                y_pixel_stride,
                uv_pixel_stride
            );
            break;
        default:
            break;
    }

    cam->read = true;

error:
    return;
}

static
void
on_image_available(void *context, AImageReader *reader)
{
    /*
    DEBUG(
        "onImageAvailable(context=%p, reader=%p)",
        context,
        (void *) reader
    );
    */
    struct al_camera *cam = context;
    if (cam == NULL)
        return;
    if (cam->stop)
        return;
    AImage *image = NULL;
    int32_t max_images = 0;
    media_status_t status = AImageReader_getMaxImages(reader, &max_images);
    if (status != AMEDIA_OK) {
        DEBUG_AMEDIA("AImageReader_getMaxImages", status);
        max_images = 1;
    }
    if (max_images == 1) {
        status = AImageReader_acquireNextImage(reader, &image);
        if (status != AMEDIA_OK) {
            DEBUG_AMEDIA("AImageReader_acquireNextImage", status);
            return;
        }
    } else {
        status = AImageReader_acquireLatestImage(reader, &image);
        if (status != AMEDIA_OK) {
            DEBUG_AMEDIA("AImageReader_acquireLatestImage", status);
            return;
        }
    }
    /*
    media_status_t status = AImageReader_acquireLatestImage(reader, &image);
    if (status != AMEDIA_OK) {
        DEBUG_AMEDIA("AImageReader_acquireLatestImage", status);
        return;
    }
    */
    if (image == NULL)
        return;
    process_image(cam, image);
    AImage_delete(image);
}

static inline
ACameraManager *
new_camera_manager(struct al_camera *cam)
{
    assert(cam != NULL);
    assert(cam->availability_callbacks != NULL);
    ACameraManager *manager = ACameraManager_create();
    if (manager == NULL)
        return NULL;
    camera_status_t status = ACameraManager_registerAvailabilityCallback(
        manager,
        cam->availability_callbacks
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraManager_registerAvailabilityCallback", status);
        ACameraManager_delete(manager);
        return NULL;
    }
    return manager;
}

static
void
on_disconnected(void *context, ACameraDevice *device)
{
    DEBUG(
        "onDisconnected(context=%p, device=%p)",
        context,
        (void *) device
    );
}

static
void
on_error(void *context, ACameraDevice *device, int error)
{
    const char codes[][64] = {
        [ERROR_CAMERA_IN_USE] = "ERROR_CAMERA_IN_USE",
        [ERROR_MAX_CAMERAS_IN_USE] = "ERROR_MAX_CAMERAS_IN_USE",
        [ERROR_CAMERA_DISABLED] = "ERROR_CAMERA_DISABLED",
        [ERROR_CAMERA_DEVICE] = "ERROR_CAMERA_DEVICE",
        [ERROR_CAMERA_SERVICE] = "ERROR_CAMERA_SERVICE",
    };
    DEBUG(
        "onError(context=%p, device=%p, error=%i [%s])",
        context,
        (void *) device,
        error,
        codes[error]
    );
}

static inline
char *
get_camera_id(struct al_camera *cam, const size_t index)
{
    assert(cam != NULL);
    assert(cam->manager != NULL);
    ACameraIdList *ids = NULL;
    camera_status_t status = ACameraManager_getCameraIdList(
        cam->manager,
        &ids
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraManager_getCameraIdList", status);
        return NULL;
    }
    char *id = NULL;
    if (index < (size_t) ids->numCameras) {
        errno = 0;
        id = strdup(ids->cameraIds[index]);
        if (id == NULL) {
            DEBUG("strdup: errno=%i [%s]", errno, strerror(errno));
        }
    }
    ACameraManager_deleteCameraIdList(ids);
    return id;
}

static inline
ACameraDevice *
open_camera_device(struct al_camera *cam)
{
    assert(cam != NULL);
    assert(cam->manager != NULL);
    assert(cam->id != NULL);
    assert(cam->state_callbacks != NULL);
    ACameraDevice *device = NULL;
    camera_status_t status = ACameraManager_openCamera(
        cam->manager,
        cam->id,
        cam->state_callbacks,
        &device
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraManager_openCamera", status);
        return NULL;
    }
    return device;
}

static inline
AImageReader *
new_reader(struct al_camera *cam)
{
    assert(cam != NULL);
    assert(cam->width > 0 && cam->width <= INT32_MAX);
    assert(cam->height > 0 && cam->height <= INT32_MAX);
    assert(cam->image_format != 0);
    AImageReader *reader = NULL;
    media_status_t status = AImageReader_new(
        (int32_t) cam->width,
        (int32_t) cam->height,
        cam->image_format,
        1,
        &reader
    );
    if (status != AMEDIA_OK) {
        DEBUG_AMEDIA("AImageReader_new", status);
        goto error0;
    }

    errno = 0;
    AImageReader_ImageListener *listener = calloc(
        1,
        sizeof (AImageReader_ImageListener)
    );
    if (listener == NULL) {
        DEBUG("calloc: errno=%i [%s]", errno, strerror(errno));
        goto error1;
    }
    listener->context = cam;
    listener->onImageAvailable = &on_image_available;
    status = AImageReader_setImageListener(reader, listener);
    if (status != AMEDIA_OK) {
        DEBUG_AMEDIA("AImageReader_setImageListener", status);
        goto error2;
    }
    cam->listener = listener;

    ANativeWindow *window = NULL;
    status = AImageReader_getWindow(reader, &window);
    if (status != AMEDIA_OK) {
        DEBUG_AMEDIA("AImageReader_getWindow", status);
        goto error3;
    }
    ANativeWindow_acquire(window);
    cam->window = window;

    return reader;

error3:
error2:
    free(cam->listener);
    cam->listener = NULL;
error1:
    AImageReader_delete(reader);
    cam->image_format = 0;
error0:
    return NULL;
}

static
void
on_session_closed(void *context, ACameraCaptureSession *session)
{
    DEBUG(
        "onClosed(context=%p, session=%p)",
        context,
        (void *) session
    );
}

static
void
on_session_ready(void *context, ACameraCaptureSession *session)
{
    DEBUG(
        "onReady(context=%p, session=%p)",
        context,
        (void *) session
    );
    // struct al_camera *cam = context;
}

static
void
on_session_active(void *context, ACameraCaptureSession *session)
{
    DEBUG(
        "onActive(context=%p, session=%p)",
        context,
        (void *) session
    );
}

static inline
ACameraCaptureSession *
new_session(struct al_camera *cam)
{
    assert(cam != NULL);

    assert(cam->window != NULL);
    ACaptureSessionOutput *output = NULL;
    camera_status_t status = ACaptureSessionOutput_create(
        cam->window,
        &output
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACaptureSessionOutput_create", status);
        goto error0;
    }

    ACaptureSessionOutputContainer *container = NULL;
    status = ACaptureSessionOutputContainer_create(&container);
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACaptureSessionOutputContainer_create", status);
        goto error1;
    }

    status = ACaptureSessionOutputContainer_add(container, output);
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACaptureSessionOutputContainer_add", status);
        goto error2;
    }

    assert(cam->device != NULL);
    assert(cam->session_callbacks != NULL);
    ACameraCaptureSession *session = NULL;
    status = ACameraDevice_createCaptureSession(
        cam->device,
        container,
        cam->session_callbacks,
        &session
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraDevice_createCaptureSession", status);
        goto error3;
    }

    return session;

error3:
    (void) ACaptureSessionOutputContainer_remove(container, output);
error2:
    ACaptureSessionOutputContainer_free(container);
error1:
    ACaptureSessionOutput_free(output);
error0:
    return NULL;
}

static inline
ACaptureRequest *
new_request(struct al_camera *cam)
{
    assert(cam != NULL);
    assert(cam->device != NULL);
    ACaptureRequest *request = NULL;
    camera_status_t status = ACameraDevice_createCaptureRequest(
        cam->device,
        // TEMPLATE_PREVIEW,
        TEMPLATE_RECORD,
        &request
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraDevice_createCaptureRequest", status);
        goto error0;
    }
    assert(cam->window != NULL);
    ACameraOutputTarget *output = NULL;
    status = ACameraOutputTarget_create(
        cam->window,
        &output
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraOutputTarget_create", status);
        goto error1;
    }
    status = ACaptureRequest_addTarget(request, output);
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACaptureRequest_addTarget", status);
        goto error2;
    }
    return request;
error2:
    ACameraOutputTarget_free(output);
error1:
    ACaptureRequest_free(request);
error0:
    return NULL;
}

static inline
struct metadata
get_camera_metadata(struct al_camera *cam)
{
    assert(cam != NULL);
    assert(cam->manager != NULL);
    assert(cam->id != NULL);
    struct metadata metadata = {
        .facing = ACAMERA_LENS_FACING_BACK,
        .orientation = 0,
        .width = 640,
        .height = 480
    };
    ACameraMetadata *camera_metadata = NULL;
    camera_status_t status = ACameraManager_getCameraCharacteristics(
        cam->manager,
        cam->id,
        &camera_metadata
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraManager_getCameraCharacteristics", status);
        goto error0;
    }
    ACameraMetadata_const_entry lens = {0};
    status = ACameraMetadata_getConstEntry(
        camera_metadata,
        ACAMERA_LENS_FACING,
        &lens
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraMetadata_getConstEntry", status);
    } else {
        metadata.facing = lens.data.u8[0];
    }
    ACameraMetadata_const_entry orientation = {0};
    status = ACameraMetadata_getConstEntry(
        camera_metadata,
        ACAMERA_SENSOR_ORIENTATION,
        &orientation
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraMetadata_getConstEntry", status);
    } else {
        metadata.orientation = orientation.data.i32[0];
    }
    ACameraMetadata_const_entry configurations;
    status = ACameraMetadata_getConstEntry(
        camera_metadata,
        ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
        &configurations
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraMetadata_getConstEntry", status);
    } else {
        size_t index = 0;
        float distance = FLT_MAX;
        for (size_t i = 0; i < configurations.count; i++) {
            int32_t format = configurations.data.i32[4 * i + 0];
            int32_t width = configurations.data.i32[4 * i + 1];
            int32_t height = configurations.data.i32[4 * i + 2];
            int32_t input = configurations.data.i32[4 * i + 3];
            if (input != 0)
                continue;
            if (format != cam->image_format)
                continue;
            float norm = _al_l2norm(
                (float) cam->width,
                (float) cam->height,
                (float) width,
                (float) height
            );
            if (norm < distance) {
                index = i;
                distance = norm;
            }
        }
        if (distance < FLT_MAX) {
            metadata.width = configurations.data.i32[4 * index + 1];
            metadata.height = configurations.data.i32[4 * index + 2];
        }
    }
error0:
    if (camera_metadata != NULL)
        ACameraMetadata_free(camera_metadata);
    return metadata;
}

enum al_status
al_camera_new(
    struct al_camera **cam,
    size_t index,
    size_t width,
    size_t height
) {
    assert(cam != NULL);
    enum al_status ret = AL_ERROR;

    if (_sdk < 24)
        goto error0;

    errno = 0;
    *cam = calloc(1, sizeof (struct al_camera));
    if (*cam == NULL) {
        DEBUG("calloc: errno=%i [%s]", errno, strerror(errno));
        ret = AL_NOMEMORY;
        goto error0;
    }

    (*cam)->index = index;
    if (index < N_CAMERAS)
        _al_cameras[index] = *cam;

    errno = 0;
    (*cam)->availability_callbacks = calloc(
        1,
        sizeof (ACameraManager_AvailabilityCallbacks)
    );
    if ((*cam)->availability_callbacks == NULL) {
        DEBUG("calloc: errno=%i [%s]", errno, strerror(errno));
        ret = AL_NOMEMORY;
        goto error1;
    }
    (*cam)->availability_callbacks->context = *cam;
    (*cam)->availability_callbacks->onCameraAvailable = &on_available;
    (*cam)->availability_callbacks->onCameraUnavailable = &on_unavailable;

    (*cam)->manager = new_camera_manager(*cam);
    if ((*cam)->manager == NULL)
        goto error2;

    (*cam)->id = get_camera_id(*cam, index);
    if ((*cam)->id == NULL)
        goto error3;

    (*cam)->image_format = AIMAGE_FORMAT_YUV_420_888;
    (*cam)->width = width;
    (*cam)->height = height;
    (*cam)->metadata = get_camera_metadata(*cam);
    assert((*cam)->metadata.width > 0);
    assert((*cam)->metadata.height > 0);
    (*cam)->width = (size_t) (*cam)->metadata.width;
    (*cam)->height = (size_t) (*cam)->metadata.height;

    errno = 0;
    (*cam)->state_callbacks = calloc(
        1,
        sizeof (ACameraDevice_StateCallbacks)
    );
    if ((*cam)->state_callbacks == NULL) {
        DEBUG("calloc: errno=%i [%s]", errno, strerror(errno));
        ret = AL_NOMEMORY;
        goto error5;
    }
    (*cam)->state_callbacks->context = *cam;
    (*cam)->state_callbacks->onDisconnected = &on_disconnected;
    (*cam)->state_callbacks->onError = &on_error;

    (*cam)->device = open_camera_device(*cam);
    if ((*cam)->device == NULL)
        goto error6;

    (*cam)->reader = new_reader(*cam);
    if ((*cam)->reader == NULL)
        goto error7;

    errno = 0;
    (*cam)->session_callbacks = calloc(
        1,
        sizeof (ACameraCaptureSession_stateCallbacks)
    );
    if ((*cam)->session_callbacks == NULL) {
        DEBUG("calloc: errno=%i [%s]", errno, strerror(errno));
        ret = AL_NOMEMORY;
        goto error8;
    }
    // (*cam)->session_callbacks->context = *cam;
    (*cam)->session_callbacks->context = NULL;
    (*cam)->session_callbacks->onClosed = &on_session_closed;
    (*cam)->session_callbacks->onReady = &on_session_ready;
    (*cam)->session_callbacks->onActive = &on_session_active;

    (*cam)->session = new_session(*cam);
    if ((*cam)->session == NULL)
        goto error9;

    (*cam)->request = new_request(*cam);
    if ((*cam)->request == NULL)
        goto error10;

    int orientation = (*cam)->metadata.orientation;
    orientation = 0;
    switch (orientation) {
        case 0:
        case 180:
            (*cam)->image.width = (*cam)->width;
            (*cam)->image.height = (*cam)->height;
            break;
        case 90:
        case 270:
            (*cam)->image.width = (*cam)->height;
            (*cam)->image.height = (*cam)->width;
            break;
        default:
            break;
    }
    // (*cam)->image.stride = _al_calc_next_multiple((*cam)->width, 32);
    (*cam)->image.stride = (*cam)->width;

    ret = AL_OK;
    return ret;

error10:
    (void) ACameraCaptureSession_abortCaptures((*cam)->session);
    ACameraCaptureSession_close((*cam)->session);
    (*cam)->session = NULL;
error9:
    free((*cam)->session_callbacks);
    (*cam)->session_callbacks = NULL;
error8:
    AImageReader_delete((*cam)->reader);
    (*cam)->reader = NULL;
    free((*cam)->listener);
    (*cam)->listener = NULL;
    ANativeWindow_release((*cam)->window);
    (*cam)->window = NULL;
error7:
    (void) ACameraDevice_close((*cam)->device);
    (*cam)->device = NULL;
error6:
    free((*cam)->state_callbacks);
    (*cam)->state_callbacks = NULL;
error5:
    free((*cam)->id);
    (*cam)->id = NULL;
error3:
    (void) ACameraManager_unregisterAvailabilityCallback(
        (*cam)->manager,
        (*cam)->availability_callbacks
    );
    ACameraManager_delete((*cam)->manager);
    (*cam)->manager = NULL;
error2:
    free((*cam)->availability_callbacks);
    (*cam)->availability_callbacks = NULL;
error1:
    free(*cam);
    *cam = NULL;
error0:
    return ret;
}

void
al_camera_free(struct al_camera *cam)
{
    if (cam == NULL)
        return;
    if (cam->session != NULL)
        (void) ACameraCaptureSession_abortCaptures(cam->session);
    if (cam->request != NULL)
        ACaptureRequest_free(cam->request);
    cam->request = NULL;
    free(cam->capture_callbacks);
    cam->capture_callbacks = NULL;
    if (cam->session != NULL)
        ACameraCaptureSession_close(cam->session);
    cam->session = NULL;
    free(cam->session_callbacks);
    cam->session_callbacks = NULL;
    if (cam->reader != NULL)
        AImageReader_delete(cam->reader);
    cam->reader = NULL;
    al_image_free(&cam->image);
    al_image_free(&cam->rgba);
    al_image_free(&cam->yuv420sp);
    al_image_free(&cam->yuv420p);
    free(cam->listener);
    cam->listener = NULL;
    if (cam->window != NULL)
        ANativeWindow_release(cam->window);
    cam->window = NULL;
    if (cam->device != NULL)
        (void) ACameraDevice_close(cam->device);
    cam->device = NULL;
    free(cam->state_callbacks);
    cam->state_callbacks = NULL;
    free(cam->id);
    cam->id = NULL;
    if (cam->manager != NULL) {
        (void) ACameraManager_unregisterAvailabilityCallback(
            cam->manager,
            cam->availability_callbacks
        );
        ACameraManager_delete(cam->manager);
    }
    cam->manager = NULL;
    free(cam->availability_callbacks);
    cam->availability_callbacks = NULL;
    if (cam->index < N_CAMERAS)
        _al_cameras[cam->index] = NULL;
    free(cam);
}

static
void
on_capture_started(
    void *context,
    ACameraCaptureSession *session,
    const ACaptureRequest* request,
    int64_t timestamp
) {
    (void) context;
    (void) session;
    (void) request;
    (void) timestamp;
    /*
    DEBUG(
        "onCaptureStarted("
            "context=%p, session=%p, request=%p, timestamp=%" PRIi64
        ")",
        context,
        (void *) session,
        (const void *) request,
        timestamp
    );
    */
}

static
void
on_capture_progressed(
    void *context,
    ACameraCaptureSession *session,
    ACaptureRequest *request,
    const ACameraMetadata* result
) {
    (void) context;
    (void) session;
    (void) request;
    (void) result;
    /*
    DEBUG(
        "onCaptureProgressed(context=%p, session=%p, request=%p, result=%p)",
        context,
        (void *) session,
        (void *) request,
        (const void *) result
    );
    */
}

static
void
on_capture_completed(
    void *context,
    ACameraCaptureSession *session,
    ACaptureRequest *request,
    const ACameraMetadata* result
) {
    (void) context;
    (void) session;
    (void) request;
    (void) result;
    /*
    DEBUG(
        "onCaptureCompleted(context=%p, session=%p, request=%p, result=%p)",
        context,
        (void *) session,
        (void *) request,
        (const void *) result
    );
    */
}

static
void
on_capture_failed(
    void *context,
    ACameraCaptureSession *session,
    ACaptureRequest *request,
    ACameraCaptureFailure *failure
) {
    const char failures[][64] = {
        [CAPTURE_FAILURE_REASON_ERROR] = "CAPTURE_FAILURE_REASON_ERROR",
        [CAPTURE_FAILURE_REASON_FLUSHED] = "CAPTURE_FAILURE_REASON_FLUSHED",
    };
    DEBUG(
        "onCaptureFailed("
            "context=%p, session=%p, request=%p, failure=%s"
        ")",
        context,
        (void *) session,
        (void *) request,
        failures[failure->reason]
    );
}

static
void
on_capture_sequence_completed(
    void *context,
    ACameraCaptureSession *session,
    int sequenceId,
    int64_t frameNumber
) {
    DEBUG(
        "onCaptureSequenceCompleted("
            "context=%p, session=%p, sequenceId=%i, frameNumber=%" PRIi64
        ")",
        context,
        (void *) session,
        sequenceId,
        frameNumber
    );
}

static
void
on_capture_sequence_aborted(
    void *context,
    ACameraCaptureSession *session,
    int sequenceId
) {
    DEBUG(
        "onCaptureSequenceAborted(context=%p, session=%p, sequenceId=%i)",
        context,
        (void *) session,
        sequenceId
    );
}

static
void
on_capture_buffer_lost(
    void *context,
    ACameraCaptureSession *session,
    ACaptureRequest *request,
    ACameraWindowType *window,
    int64_t frameNumber
) {
    DEBUG(
        "onCaptureBufferLost("
            "context=%p, session=%p, request=%p, window=%p, "
            "frameNumber=%" PRIi64
        ")",
        context,
        (void *) session,
        (void *) request,
        (void *) window,
        frameNumber
    );
}

void
al_camera_start(struct al_camera *cam)
{
    assert(cam != NULL);
    if (cam->capture_callbacks == NULL) {
        errno = 0;
        ACameraCaptureSession_captureCallbacks *callbacks = calloc(
            1,
            sizeof (ACameraCaptureSession_captureCallbacks)
        );
        if (callbacks == NULL) {
            DEBUG("calloc: errno=%i [%s]", errno, strerror(errno));
            goto error0;
        }
        callbacks->context = cam;
        callbacks->onCaptureStarted = &on_capture_started;
        callbacks->onCaptureProgressed = &on_capture_progressed;
        callbacks->onCaptureCompleted = &on_capture_completed;
        callbacks->onCaptureFailed = &on_capture_failed;
        callbacks->onCaptureSequenceCompleted = 
            &on_capture_sequence_completed;
        callbacks->onCaptureSequenceAborted = &on_capture_sequence_aborted;
        callbacks->onCaptureBufferLost = &on_capture_buffer_lost;
        cam->capture_callbacks = callbacks;
    }
    if (cam->session == NULL || cam->request == NULL)
        goto error1;
    camera_status_t status = ACameraCaptureSession_setRepeatingRequest(
        cam->session,
        cam->capture_callbacks,
        1,
        (ACaptureRequest *[]) {cam->request},
        NULL
    );
    if (status != ACAMERA_OK) {
        DEBUG_ACAMERA("ACameraCaptureSession_setRepeatingRequest", status);
        goto error2;
    }
    cam->stop = false;
    return;
error2:
error1:
    free(cam->capture_callbacks);
    cam->capture_callbacks = NULL;
error0:
    return;
}

void
al_camera_stop(struct al_camera *cam)
{
    assert(cam != NULL);
    cam->stop = true;
    if (cam->session != NULL) {
        (void) ACameraCaptureSession_stopRepeating(cam->session);
        usleep(200e3);
    }
}

enum al_status
al_camera_get_id(struct al_camera *cam, const char **id)
{
    assert(cam != NULL);
    assert(id != NULL);
    *id = cam->id;
    return AL_OK;
}

enum al_status
al_camera_get_color_format(struct al_camera *cam, enum al_color_format *format)
{
    assert(cam != NULL);
    assert(format != NULL);
    *format = cam->image.format;
    return AL_OK;
}

enum al_status
al_camera_get_width(struct al_camera *cam, size_t *width)
{
    assert(cam != NULL);
    assert(width != NULL);
    *width = cam->image.width;
    return AL_OK;
}

enum al_status
al_camera_get_height(struct al_camera *cam, size_t *height)
{
    assert(cam != NULL);
    assert(height != NULL);
    *height = cam->image.height;
    return AL_OK;
}

enum al_status
al_camera_get_data(struct al_camera *cam, enum al_color_format format, void **data)
{
    assert(cam != NULL);
    assert(data != NULL);
    if (cam->image.format == format) {
        *data = cam->image.data;
        return AL_OK;
    }
    switch (format) {
        case AL_COLOR_FORMAT_YUV420SP:
            switch (cam->color_format) {
                case COLOR_FormatYUV420Planar:
                    al_yuv_i420_to_nv12(
                        cam->yuv420p.data,
                        cam->yuv420sp.data,
                        cam->width,
                        cam->height
                    );
                    *data = cam->yuv420sp.data;
                    return AL_OK;
                case COLOR_FormatYUV420SemiPlanar:
                    *data = cam->yuv420sp.data;
                    return AL_OK;
                default:
                    break;
            }
            break;
        case AL_COLOR_FORMAT_YUV420P:
            switch (cam->color_format) {
                case COLOR_FormatYUV420Planar:
                    *data = cam->yuv420p.data;
                    return AL_OK;
                case COLOR_FormatYUV420SemiPlanar:
                    al_yuv_nv12_to_i420(
                        cam->yuv420sp.data,
                        cam->yuv420p.data,
                        cam->width,
                        cam->height
                    );
                    *data = cam->yuv420p.data;
                    return AL_OK;
                default:
                    break;
            }
            break;
    }
    return AL_ERROR;
}

enum al_status
al_camera_get_rgba(struct al_camera *cam, void **data)
{
    assert(cam != NULL);
    assert(data != NULL);
    if (!cam->read) {
        *data = NULL;
        return AL_OK;
    }
    cam->read = false;
    *data = cam->rgba.data;
    return AL_OK;
}

enum al_status
al_camera_get_facing(struct al_camera *cam, enum al_camera_facing *facing)
{
    assert(cam != NULL);
    assert(facing != NULL);
    switch (cam->metadata.facing) {
        case 0:
            *facing = AL_CAMERA_FACING_FRONT;
            return AL_OK;
        case 1:
            *facing = AL_CAMERA_FACING_BACK;
            return AL_OK;
        default:
            break;
    }
    return AL_ERROR;
}

enum al_status
al_camera_get_orientation(struct al_camera *cam, int *orientation)
{
    assert(cam != NULL);
    assert(orientation != NULL);
    *orientation = cam->metadata.orientation;
    return AL_OK;
}

enum al_status
al_camera_set_stride(struct al_camera *cam, size_t stride)
{
    assert(cam != NULL);
    assert(stride >= cam->image.width);
    assert(stride % 16 == 0);
    void *data = NULL;
    size_t size = 0;
    switch (cam->image.format) {
        case AL_COLOR_FORMAT_YUV420P:
        case AL_COLOR_FORMAT_YUV420SP:
            size = stride * cam->image.height * 3 / 2 * sizeof (uint8_t);
            break;
        case AL_COLOR_FORMAT_RGBA:
            size = stride * cam->image.height * sizeof (uint32_t);
            break;
        case AL_COLOR_FORMAT_UNKNOWN:
            break;
    }
    if (size > 0)
        data = realloc(cam->image.data, size);
    if (data == NULL)
        return AL_NOMEMORY;
    cam->image.data = data;
    cam->image.stride = stride;
    return AL_OK;
}

void
al_camera_cleanup(void)
{
    for (size_t i = 0; i < N_CAMERAS; i++) {
        if (_al_cameras[i] != NULL) {
            al_camera_stop(_al_cameras[i]);
        }
    }
}
