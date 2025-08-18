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
#include <inttypes.h> // PRIxPTR
#include <signal.h>
#include <stddef.h>
#include <stdint.h> // uintptr_t
#include <stdlib.h>
#include <string.h> // strsignal

#include <android/log.h>

#include <jni.h>

#include "al.h"

#include "common.h"

const char *const copyright = "Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com>";
const char *const platform = "android";

JavaVM *_jvm = NULL;
jint _sdk = 0;

static struct al_android_activity {
    jobject instance;
    const char *class;
    const char *field;
} _activity = {
    NULL,
    NULL,
    NULL
};

jint
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    (void) reserved;
    _jvm = vm;
    assert(_jvm != NULL && *_jvm != NULL);

    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);
    jclass Build = _class(env, "android/os/Build$VERSION");
    jfieldID sdk_int = (*env)->GetStaticFieldID(env, Build, "SDK_INT", "I");
    assert(sdk_int != NULL);
    _sdk = (*env)->GetStaticIntField(env, Build, sdk_int);
#ifndef NDEBUG
    __android_log_print(ANDROID_LOG_INFO, __FILE__, "sdk = %i", _sdk);
#endif

    return (*env)->GetVersion(env);
}

jobject
_getactivity(JNIEnv *env, const char *classname, const char *fieldname)
{
    assert(env != NULL && *env != NULL);
    if (_activity.instance == NULL) {
        if (classname == NULL || fieldname == NULL)
            return NULL;
        if (classname != NULL)
            _activity.class = classname;
        if (fieldname != NULL)
            _activity.field = fieldname;
        assert(_activity.class != NULL);
        assert(_activity.field != NULL);
        jclass Activity = _class(env, _activity.class);
        assert(Activity != NULL);
        jfieldID field = (*env)->GetStaticFieldID(
            env,
            Activity,
            _activity.field,
            "Landroid/app/Activity;"
        );
        assert(field != NULL);
        jobject instance = (*env)->GetStaticObjectField(env, Activity, field);
        _activity.instance = (*env)->NewGlobalRef(env, instance);
#ifndef NDEBUG
        __android_log_print(
            ANDROID_LOG_INFO,
            __FILE__,
            "%p = _getactivity(%p, %s, %s)\n",
            _activity.instance, (void *) env, _activity.class, _activity.field
        );
#endif
    }
    return _activity.instance;
}

void *
al_android_getactivity(const char *classname, const char *fieldname)
{
    JNIEnv *env = _jnienv();
    return _getactivity(env, classname, fieldname);
}

jobject
_getcontext(JNIEnv *env)
{
    assert(env != NULL && *env != NULL);
    jobject activity = _getactivity(env, NULL, NULL);
    if (activity == NULL)
        return NULL;
    jclass Context = _class(env, "android/content/Context");
    assert(Context != NULL);
    jmethodID get_application_context = _method(
        env,
        Context,
        "getApplicationContext",
        "()Landroid/content/Context;"
    );
    assert(get_application_context != NULL);
    return (*env)->CallObjectMethod(env, activity, get_application_context);
}

jobject
_getconfig(JNIEnv *env, jobject context)
{
    jmethodID get_resources = _method(
        env,
        _class(env, "android/content/Context"),
        "getResources",
        "()Landroid/content/res/Resources;"
    );
    jobject res = (*env)->CallObjectMethod(env, context, get_resources);
    assert(res != NULL);
    jmethodID get_configuration = _method(
        env,
        _class(env, "android/content/res/Resources"),
        "getConfiguration",
        "()Landroid/content/res/Configuration;"
    );
    return (*env)->CallObjectMethod(env, res, get_configuration);
}

static
void
fatal_signal_handler(int signum)
{
    // clean up
    al_camera_cleanup();

    signal(signum, SIG_DFL);
    raise(signum);
}

static
void
catch_fatal_signals(void)
{
    const int signals[] = {
        SIGTERM,
        SIGINT,
        SIGQUIT,
        SIGABRT,
        SIGSEGV,
        SIGFPE,
        SIGILL,
        SIGBUS,
    };
    for (size_t i = 0; i < sizeof signals / sizeof (signals[0]); i++) {
        if (signal(signals[i], &fatal_signal_handler) == SIG_ERR) {
            DEBUG(
                "signal(%s, %#" PRIxPTR ") = SIG_ERR",
                strsignal(signals[i]),
                (uintptr_t) &fatal_signal_handler
            );
        }
    }
}

__attribute__((constructor))
void
init(void)
{
    catch_fatal_signals();
}
