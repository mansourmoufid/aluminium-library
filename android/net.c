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

#include <arpa/inet.h> // inet_addr
#include <assert.h>
#include <netinet/in.h> // in_addr_t, INET6_ADDRSTRLEN
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <jni.h>

#include "al.h"

#include "common.h"
#include "net.h"

static jobject _al_multicast_lock = NULL;

static
jobject
_get_system_service(JNIEnv *env, const char *service)
{
    jclass Context = _class(env, "android/content/Context");
    assert(Context != NULL);
    jmethodID get_system_service = _method(
        env,
        _class(env, "android/content/Context"),
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;"
    );
    assert(get_system_service != NULL);
    jstring serv = (*env)->GetStaticObjectField(
        env,
        Context,
        _static_field(env, Context, service, "Ljava/lang/String;")
    );
    if (serv == NULL)
        return NULL;
    return (*env)->CallObjectMethod(
        env,
        _getactivity(env, NULL, NULL),
        get_system_service,
        serv
    );
}

static inline
jobject
_get_wifi_manager(JNIEnv *env)
{
    return _get_system_service(env, "WIFI_SERVICE");
}

static inline
jobject
_get_connectivity_manager(JNIEnv *env)
{
    return _get_system_service(env, "CONNECTIVITY_SERVICE");
}

static char _al_ip[INET6_ADDRSTRLEN] = {0};

static inline
char *
parse_ip(const char *x)
{
    memset(_al_ip, 0, sizeof _al_ip);
    char *slash = strchr(x, '/');
    if (slash == NULL)
        strncpy(_al_ip, x, sizeof _al_ip);
    else
        strncpy(_al_ip, x, (size_t) (slash - x));
    return _al_ip;
}

static inline
bool
private_ip(const char *ip)
{
    in_addr_t addr = inet_addr(ip);
    int a = (addr >> 0) & 0xff;
    int b = (addr >> 8) & 0xff;
    // RFC 1918
    return (a == 10)
        || (a == 172 && (b >= 16 && b <= 31))
        || (a == 192 && b == 168);
}

char *
al_net_get_local_ip_address(void)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);

    if (_sdk >= 31) {

        jobject connectivity_manager = _get_connectivity_manager(env);
        if (connectivity_manager == NULL)
            return NULL;
        jmethodID get_active_network = _method(
            env,
            _class(env, "android/net/ConnectivityManager"),
            "getActiveNetwork",
            "()Landroid/net/Network;"
        );
        assert(get_active_network != NULL);
        jobject network = (*env)->CallObjectMethod(
            env,
            connectivity_manager,
            get_active_network
        );
        if (network == NULL)
            return NULL;
        jmethodID get_link_properties = _method(
            env,
            _class(env, "android/net/ConnectivityManager"),
            "getLinkProperties",
            "(Landroid/net/Network;)Landroid/net/LinkProperties;"
        );
        assert(get_link_properties != NULL);
        jobject properties = (*env)->CallObjectMethod(
            env,
            connectivity_manager,
            get_link_properties,
            network
        );
        if (properties == NULL)
            return NULL;
        jmethodID get_link_addresses = _method(
            env,
            _class(env, "android/net/LinkProperties"),
            "getLinkAddresses",
            "()Ljava/util/List;"
        );
        assert(get_link_addresses != NULL);
        jobject addresses = (*env)->CallObjectMethod(
            env,
            properties,
            get_link_addresses
        );
        if (addresses == NULL)
            return NULL;
        jmethodID size = _method(
            env,
            _class(env, "java/util/List"),
            "size",
            "()I"
        );
        assert(size != NULL);
        jint n = (*env)->CallIntMethod(env, addresses, size);
        if (n == 0)
            return NULL;
        jmethodID get = _method(
            env,
            _class(env, "java/util/List"),
            "get",
            "(I)Ljava/lang/Object;"
        );
        assert(get != NULL);
        jmethodID to_string = _method(
            env,
            _class(env, "android/net/LinkAddress"),
            "toString",
            "()Ljava/lang/String;"
        );
        assert(to_string != NULL);
        for (jint i = 0; i < n; i++) {
            jobject address = (*env)->CallObjectMethod(
                env,
                addresses,
                get,
                i
            );
            if (address == NULL)
                continue;
            jstring ip = (*env)->CallObjectMethod(env, address, to_string);
            if (ip == NULL)
                continue;
            const char *s = _cstring(env, ip);
            if (private_ip(parse_ip(s)))
                return strdup(parse_ip(s));
        }
        return NULL;

    } else {

        jobject wifi_manager = _get_wifi_manager(env);
        if (wifi_manager == NULL)
            return NULL;
        jmethodID get_connection_info = _method(
            env,
            _class(env, "android/net/wifi/WifiManager"),
            "getConnectionInfo",
            "()Landroid/net/wifi/WifiInfo;"
        );
        assert(get_connection_info != NULL);
        jobject info = (*env)->CallObjectMethod(
            env,
            wifi_manager,
            get_connection_info
        );
        if (info == NULL)
            return NULL;
        jmethodID get_ip_address = _method(
            env,
            _class(env, "android/net/wifi/WifiInfo"),
            "getIpAddress",
            "()I"
        );
        assert(get_ip_address != NULL);
        jint ip = (*env)->CallIntMethod(env, info, get_ip_address);
        snprintf(
            _al_ip,
            sizeof _al_ip,
            "%i.%i.%i.%i",
            (ip >> 0) & 0xff,
            (ip >> 8) & 0xff,
            (ip >> 16) & 0xff,
            (ip >> 24) & 0xff
        );
        return strdup(_al_ip);

    }
}

int
al_android_multicast_lock_acquire(void)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);

    if (_al_multicast_lock == NULL) {
        jobject wifi_manager = _get_wifi_manager(env);
        if (wifi_manager == NULL)
            return AL_ERROR;
        jmethodID create_multicast_lock = _method(
            env,
            _class(env, "android/net/wifi/WifiManager"),
            "createMulticastLock",
            "(Ljava/lang/String;)"
                "Landroid/net/wifi/WifiManager$MulticastLock;"
        );
        assert(create_multicast_lock != NULL);
        jobject lock = (*env)->CallObjectMethod(
            env,
            wifi_manager,
            create_multicast_lock,
            (*env)->NewStringUTF(env, "LibAl")
        );
        _al_multicast_lock = (*env)->NewGlobalRef(env, lock);
    }
    if (_al_multicast_lock == NULL)
        return AL_ERROR;

    jmethodID acquire = _method(
        env,
        _class(env, "android/net/wifi/WifiManager$MulticastLock"),
        "acquire",
        "()V"
    );
    assert(acquire != NULL);
    (*env)->CallVoidMethod(env, _al_multicast_lock, acquire);

    return AL_OK;
}

int
al_android_multicast_lock_release(void)
{
    JNIEnv *env = _jnienv();
    assert(env != NULL && *env != NULL);

    if (_al_multicast_lock == NULL)
        return AL_ERROR;

    jmethodID release = _method(
        env,
        _class(env, "android/net/wifi/WifiManager$MulticastLock"),
        "release",
        "()V"
    );
    assert(release != NULL);
    (*env)->CallVoidMethod(env, _al_multicast_lock, release);
    (*env)->DeleteGlobalRef(env, _al_multicast_lock);
    _al_multicast_lock = NULL;

    return AL_OK;
}
