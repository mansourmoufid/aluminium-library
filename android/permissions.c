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

static jint PERMISSION_GRANTED;
static jint PERMISSION_DENIED;

static inline
jint
have_permission(JNIEnv *env, const char *permission)
{
    jclass PackageManager = _class(env, "android/content/pm/PackageManager");
    PERMISSION_GRANTED = (*env)->GetStaticIntField(
        env,
        PackageManager,
        _static_field(env, PackageManager, "PERMISSION_GRANTED", "I")
    );
    PERMISSION_DENIED = (*env)->GetStaticIntField(
        env,
        PackageManager,
        _static_field(env, PackageManager, "PERMISSION_DENIED", "I")
    );
    if (_sdk >= 23) {
        jobject activity = _getactivity(env, NULL, NULL);
        if (activity != NULL) {
            jmethodID check_self_permission = _method(
                env,
                _class(env, "android/content/ContextWrapper"),
                "checkSelfPermission",
                "(Ljava/lang/String;)I"
            );
            assert(check_self_permission != NULL);
            return (*env)->CallIntMethod(
                env,
                _getactivity(env, NULL, NULL),
                check_self_permission,
                (*env)->NewStringUTF(env, permission)
            );
        }
    } else {
        jobject context = _getcontext(env);
        if (context != NULL) {
            jclass ContextCompat = _class(
                env,
                "androidx/core/content/ContextCompat"
            );
            if (ContextCompat != NULL) {
                jmethodID check_self_permission = _static_method(
                    env,
                    ContextCompat,
                    "checkSelfPermission",
                    "(Landroid/content/Context;Ljava/lang/String;)I"
                );
                assert(check_self_permission != NULL);
                return (*env)->CallStaticIntMethod(
                    env,
                    ContextCompat,
                    check_self_permission,
                    context,
                    (*env)->NewStringUTF(env, permission)
                );
            }
        }
    }
    return PERMISSION_DENIED;
}

static inline
void
request_permission(JNIEnv *env, const char *permission)
{
    assert(env != NULL && *env != NULL);
    jstring value = (*env)->NewStringUTF(env, permission);
    jobjectArray permissions = (*env)->NewObjectArray(
        env,
        1,
        _class(env, "java/lang/String"),
        NULL
    );
    (*env)->SetObjectArrayElement(env, permissions, 0, value);
    if (_sdk >= 23) {
        jmethodID request_permissions = _method(
            env,
            _class(env, "android/app/Activity"),
            "requestPermissions",
            "([Ljava/lang/String;I)V"
        );
        assert(request_permissions != NULL);
        (*env)->CallVoidMethod(
            env,
            _getactivity(env, NULL, NULL),
            request_permissions,
            permissions,
            0
        );
    } else {
        jclass ActivityCompat = _class(
            env,
            "androidx/core/app/ActivityCompat"
        );
        if (ActivityCompat != NULL) {
            jmethodID request_permissions = _static_method(
                env,
                ActivityCompat,
                "requestPermissions",
                "(Landroid/app/Activity;[Ljava/lang/String;I)V"
            );
            assert(request_permissions != NULL);
            (*env)->CallStaticVoidMethod(
                env,
                ActivityCompat,
                request_permissions,
                _getactivity(env, NULL, NULL),
                permissions,
                0
            );
        }
    }
}

int
al_permissions_have(const char *permission)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);
    return have_permission(env, permission) == PERMISSION_GRANTED;
}

void
al_permissions_request(const char *permission)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);
    request_permission(env, permission);
}
