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
#include <stdlib.h>

#include <android/log.h>

#include <jni.h>

#include "al.h"

#include "common.h"

static
jobject
getlocaleobject(JNIEnv *env)
{
    assert(env != NULL && *env != NULL);
    jclass Locale = _class(env, "java/util/Locale");
    if (Locale == NULL)
        return NULL;
    jmethodID get_default = _static_method(
        env,
        Locale,
        "getDefault",
        "()Ljava/util/Locale;"
    );
    if (get_default == NULL)
        return NULL;
    return (*env)->CallStaticObjectMethod(env, Locale, get_default);
}

static
const char *
getlocalestring(JNIEnv *env)
{
    assert(env != NULL && *env != NULL);
    jclass Locale = _class(env, "java/util/Locale");
    if (Locale == NULL)
        goto error;
    jobject loc = getlocaleobject(env);
    jmethodID to_string = _method(
        env,
        Locale,
        "toString",
        "()Ljava/lang/String;"
    );
    if (to_string == NULL)
        goto error;
    jstring str = (*env)->CallObjectMethod(env, loc, to_string);
    const char *cstr = _cstring(env, str);
    if (cstr == NULL)
        goto error;
#ifndef NDEBUG
    __android_log_print(ANDROID_LOG_INFO, __FILE__, "locale = %s", cstr);
#endif
    return cstr;
error:
    return NULL;
}

const char *
al_locale(void)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);
    return getlocalestring(env);
}
