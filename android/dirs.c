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

static
jobject
getpath(JNIEnv *env, jobject file)
{
    jmethodID get_path = _method(
        env,
        _class(env, "java/io/File"),
        "getPath",
        "()Ljava/lang/String;"
    );
    assert(get_path != NULL);
    jobject path = (*env)->CallObjectMethod(env, file, get_path);
    return path;
}

static
jobject
datadir(JNIEnv *env)
{
    assert(env != NULL && *env != NULL);
    jobject context = _getcontext(env);
    if (context != NULL) {
        jmethodID get_files_dir = _method(
            env,
            _class(env, "android/content/Context"),
            "getFilesDir",
            "()Ljava/io/File;"
        );
        assert(get_files_dir != NULL);
        jobject dir = (*env)->CallObjectMethod(
            env,
            context,
            get_files_dir
        );
        jobject path = getpath(env, dir);
        return path;
    }
    return NULL;
}

const char *
al_datadir(void)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);
    return _cstring(env, datadir(env));
}

const char *
al_libdir(void)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);
    jobject context = _getcontext(env);
    jmethodID get_application_info = _method(
        env,
        _class(env, "android/content/Context"),
        "getApplicationInfo",
        "()Landroid/content/pm/ApplicationInfo;"
    );
    assert(get_application_info != NULL);
    jobject info = (*env)->CallObjectMethod(
        env,
        context,
        get_application_info
    );
    assert(info != NULL);
    jfieldID native_library_dir = (*env)->GetFieldID(
        env,
        _class(env, "android/content/pm/ApplicationInfo"),
        "nativeLibraryDir",
        "Ljava/lang/String;"
    );
    assert(native_library_dir != NULL);
    jobject dir = (*env)->GetObjectField(env, info, native_library_dir);
    assert(dir != NULL);
    return _cstring(env, dir);
}
