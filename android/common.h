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

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <jni.h>

#include <android/log.h>

#include <media/NdkMediaError.h>

// android.media.MediaCodecInfo.CodecCapabilities
#define COLOR_FormatYUV420Planar            19
#define COLOR_FormatYUV420PackedPlanar      20
#define COLOR_FormatYUV420SemiPlanar        21
#define COLOR_FormatYUV420PackedSemiPlanar  39
#define COLOR_FormatYUV420Flexible          2135033992
#define COLOR_Format32bitABGR8888           2130747392
#define COLOR_FormatRGBAFlexible            2134288520

extern JavaVM *_jvm;
extern jint _sdk;

void *al_android_getactivity(const char *, const char *);

jint JNI_OnLoad(JavaVM *, void *);
jobject _getactivity(JNIEnv *, const char *, const char *);
jobject _getcontext(JNIEnv *);
jobject _getconfig(JNIEnv *, jobject);

static inline
jclass
_class(JNIEnv *env, const char *name)
{
    assert(env != NULL && *env != NULL);
    return (*env)->FindClass(env, name);
}

static inline
jmethodID
_static_method(JNIEnv *env, jclass class, const char *name, const char *sig)
{
    assert(env != NULL && *env != NULL);
    return (*env)->GetStaticMethodID(env, class, name, sig);
}

static inline
jmethodID
_method(JNIEnv *env, jclass instance, const char *name, const char *sig)
{
    assert(env != NULL && *env != NULL);
    return (*env)->GetMethodID(env, instance, name, sig);
}

static inline
jfieldID
_static_field(JNIEnv *env, jclass class, const char *name, const char *sig)
{
    assert(env != NULL && *env != NULL);
    return (*env)->GetStaticFieldID(env, class, name, sig);
}

static inline
JNIEnv *
_jnienv(void)
{
    assert(_jvm != NULL && *_jvm != NULL);
    JNIEnv *env = NULL;
    if ((*_jvm)->GetEnv(_jvm, (void **) &env, JNI_VERSION_1_2) != JNI_OK) {
        (void) (*_jvm)->AttachCurrentThread(_jvm, &env, NULL);
    }
    return env;
}

static inline
const char *
_cstring(JNIEnv *env, jstring str)
{
    if (str == NULL)
        return NULL;
    return (*env)->GetStringUTFChars(env, str, NULL);
}

#if defined(NDEBUG)
#define DEBUG(...)
#define DEBUG_ERROR(...)
#else
#define DEBUG(...) __android_log_print( \
        ANDROID_LOG_DEBUG, \
        __FILE__, \
        __VA_ARGS__ \
    )
#define DEBUG_ERRNO(function) DEBUG( \
        function ": errno=%i [%s]", \
        errno, \
        strerror(errno) \
    )
#endif

static inline
const char *
amedia_status_string(media_status_t status)
{
    switch (status) {
        case AMEDIACODEC_ERROR_INSUFFICIENT_RESOURCE: return "AMEDIACODEC_ERROR_INSUFFICIENT_RESOURCE";
        case AMEDIACODEC_ERROR_RECLAIMED: return "AMEDIACODEC_ERROR_RECLAIMED";
        case AMEDIA_DRM_ERROR_BASE: return "AMEDIA_DRM_ERROR_BASE";
        case AMEDIA_DRM_DEVICE_REVOKED: return "AMEDIA_DRM_DEVICE_REVOKED";
        case AMEDIA_DRM_LICENSE_EXPIRED: return "AMEDIA_DRM_LICENSE_EXPIRED";
        case AMEDIA_DRM_NEED_KEY: return "AMEDIA_DRM_NEED_KEY";
        case AMEDIA_DRM_NOT_PROVISIONED: return "AMEDIA_DRM_NOT_PROVISIONED";
        case AMEDIA_DRM_RESOURCE_BUSY: return "AMEDIA_DRM_RESOURCE_BUSY";
        case AMEDIA_DRM_SESSION_NOT_OPENED: return "AMEDIA_DRM_SESSION_NOT_OPENED";
        case AMEDIA_DRM_SHORT_BUFFER: return "AMEDIA_DRM_SHORT_BUFFER";
        case AMEDIA_DRM_TAMPER_DETECTED: return "AMEDIA_DRM_TAMPER_DETECTED";
        case AMEDIA_DRM_VERIFY_FAILED: return "AMEDIA_DRM_VERIFY_FAILED";
        case AMEDIA_ERROR_UNKNOWN: return "AMEDIA_ERROR_UNKNOWN";
        case AMEDIA_ERROR_MALFORMED: return "AMEDIA_ERROR_MALFORMED";
        case AMEDIA_ERROR_UNSUPPORTED: return "AMEDIA_ERROR_UNSUPPORTED";
        case AMEDIA_ERROR_INVALID_OBJECT: return "AMEDIA_ERROR_INVALID_OBJECT";
        case AMEDIA_ERROR_INVALID_PARAMETER: return "AMEDIA_ERROR_INVALID_PARAMETER";
        case AMEDIA_ERROR_INVALID_OPERATION: return "AMEDIA_ERROR_INVALID_OPERATION";
        case AMEDIA_ERROR_END_OF_STREAM: return "AMEDIA_ERROR_END_OF_STREAM";
        case AMEDIA_ERROR_IO: return "AMEDIA_ERROR_IO";
        case AMEDIA_ERROR_WOULD_BLOCK: return "AMEDIA_ERROR_WOULD_BLOCK";
        case AMEDIA_IMGREADER_ERROR_BASE: return "AMEDIA_IMGREADER_ERROR_BASE";
        case AMEDIA_IMGREADER_CANNOT_LOCK_IMAGE: return "AMEDIA_IMGREADER_CANNOT_LOCK_IMAGE";
        case AMEDIA_IMGREADER_CANNOT_UNLOCK_IMAGE: return "AMEDIA_IMGREADER_CANNOT_UNLOCK_IMAGE";
        case AMEDIA_IMGREADER_IMAGE_NOT_LOCKED: return "AMEDIA_IMGREADER_IMAGE_NOT_LOCKED";
        case AMEDIA_IMGREADER_MAX_IMAGES_ACQUIRED: return "AMEDIA_IMGREADER_MAX_IMAGES_ACQUIRED";
        case AMEDIA_IMGREADER_NO_BUFFER_AVAILABLE: return "AMEDIA_IMGREADER_NO_BUFFER_AVAILABLE";
        case AMEDIA_OK: return "AMEDIA_OK";
    }
    return NULL;
}

#define DEBUG_AMEDIA(function, status) \
    DEBUG( \
        "%s: %s", \
        function, \
        amedia_status_string(status) \
    )

void al_camera_cleanup(void);
