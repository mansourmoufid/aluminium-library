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
#include <stddef.h>

#include <jni.h>

#include "al.h"

#include "common.h"

static inline
jobject
get_display(JNIEnv *env)
{
    jobject display = NULL;
    if (_sdk >= 30) {
        jmethodID get_display = _method(
            env,
            _class(env, "android/content/Context"),
            "getDisplay",
            "()Landroid/view/Display;"
        );
        assert(get_display != NULL);
        display = (*env)->CallObjectMethod(
            env,
            _getactivity(env, NULL, NULL),
            get_display
        );
    } else {
        jmethodID get_window_manager = _method(
            env,
            _class(env, "android/app/Activity"),
            "getWindowManager",
            "()Landroid/view/WindowManager;"
        );
        assert(get_window_manager != NULL);
        jobject window_manager = (*env)->CallObjectMethod(
            env,
            _getactivity(env, NULL, NULL),
            get_window_manager
        );
        if (window_manager == NULL)
            return NULL;
        jmethodID get_default_display = _method(
            env,
            _class(env, "android/view/WindowManager"),
            "getDefaultDisplay",
            "()Landroid/view/Display;"
        );
        assert(get_default_display != NULL);
        display = (*env)->CallObjectMethod(
            env,
            window_manager,
            get_default_display
        );
    }
    return display;
}

static jint _ROTATION_0;
static jint _ROTATION_90;
static jint _ROTATION_180;
static jint _ROTATION_270;

static inline
void
init_constants(JNIEnv *env)
{
    jclass Surface = _class(env, "android/view/Surface");
    assert(Surface != NULL);
    _ROTATION_0 = (*env)->GetStaticIntField(
        env,
        Surface,
        _static_field(env, Surface, "ROTATION_0", "I")
    );
    _ROTATION_90 = (*env)->GetStaticIntField(
        env,
        Surface,
        _static_field(env, Surface, "ROTATION_90", "I")
    );
    _ROTATION_180 = (*env)->GetStaticIntField(
        env,
        Surface,
        _static_field(env, Surface, "ROTATION_180", "I")
    );
    _ROTATION_270 = (*env)->GetStaticIntField(
        env,
        Surface,
        _static_field(env, Surface, "ROTATION_270", "I")
    );
}

int
al_display_orientation(void)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);
    jobject display = get_display(env);
    if (display == NULL)
        return 0;
    jmethodID get_rotation = _method(
        env,
        _class(env, "android/view/Display"),
        "getRotation",
        "()I"
    );
    assert(get_rotation != NULL);
    init_constants(env);
    jint rotation = (*env)->CallIntMethod(env, display, get_rotation);
    if (rotation == _ROTATION_0)
        return 0;
    if (rotation == _ROTATION_90)
        return 90;
    if (rotation == _ROTATION_180)
        return 180;
    if (rotation == _ROTATION_270)
        return 270;
    return 0;
}
