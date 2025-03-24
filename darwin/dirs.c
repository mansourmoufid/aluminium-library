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
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <stdio.h>

#include <sysdir.h>

#include <CoreFoundation/CFBase.h> // Boolean
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>

#include "al.h"
#include "common.h"

static const CFStringRef _CFBundleIdentifier = CFSTR("CFBundleIdentifier");
static const CFStringRef _CFBundleExecutable = CFSTR("CFBundleExecutable");
static const CFStringRef _tilde = CFSTR("~");

static inline
const void *
bundle_dict(CFStringRef key)
{
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle == NULL)
        return NULL;
    CFDictionaryRef info = CFBundleGetInfoDictionary(bundle);
    if (info == NULL)
        return NULL;
    const void *value = CFDictionaryGetValue(info, key);
    return value;
}

static inline
int
_mkdir(const char *path)
{
    if (path == NULL)
        return -1;
    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    return mkdir(path, mode);
}

const char *
al_datadir(void)
{
    const char *result = NULL;

    const char *id = CFStringGetCStringPtr(
        bundle_dict(_CFBundleIdentifier),
        _al_encoding
    );
    if (id == NULL)
        id = getprogname();

    sysdir_search_path_enumeration_state state;
    state = sysdir_start_search_path_enumeration(
        SYSDIR_DIRECTORY_APPLICATION_SUPPORT,
        SYSDIR_DOMAIN_MASK_USER
    );
    while (true) {
        char dir[PATH_MAX] = {0};
        state = sysdir_get_next_search_path_enumeration(state, dir);
        if (state == 0)
            break;
        CFMutableStringRef path = CFStringCreateMutable(NULL, 0);
        CFStringAppendCString(path, dir, _al_encoding);
        CFRange match = CFStringFind(path, _tilde, 0);
        if (match.location != kCFNotFound) {
            CFStringRef home = CFStringCreateWithCString(
                NULL,
                getenv("HOME"),
                _al_encoding
            );
            CFStringReplace(path, match, home);
            CFRelease(home);
        }
        const char *s = CFStringGetCStringPtr(path, _al_encoding);
        if (s != NULL)
            (void) _mkdir(s);
        CFStringAppendCString(path, "/", _al_encoding);
        CFStringAppendCString(path, id, _al_encoding);
        s = CFStringGetCStringPtr(path, _al_encoding);
        if (s != NULL)
            (void) _mkdir(s);
        result = s == NULL ? NULL : strdup(s);
        CFRelease(path);
        break;
    }

    return result;
}

const char *
al_libdir(void)
{
    CFURLRef url = NULL;
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle != NULL) {
        url = CFBundleCopyExecutableURL(bundle);
        if (url != NULL)
            CFRetain(url);
    }
    if (url == NULL) {
        CFStringRef exe = bundle_dict(_CFBundleExecutable);
        if (exe != NULL) {
            url = CFURLCreateWithFileSystemPath(
                NULL,
                exe,
                kCFURLPOSIXPathStyle,
                false
            );
        }
    }
    assert(url != NULL);
    /*
    if (url == NULL) {
        exe = CFStringCreateWithCString(
            NULL,
            getprogname(),
            _al_encoding
        );
    }
    */
    CFURLRef dir = CFURLCreateCopyDeletingLastPathComponent(NULL, url);
    if (url != NULL)
        CFRelease(url);
    char path[PATH_MAX] = {0};
    Boolean result = CFURLGetFileSystemRepresentation(
        dir,
        true,
        (UInt8 *) path,
        sizeof path
    );
    CFRelease(dir);
    if (result)
        return strdup(path);
    return NULL;
}
