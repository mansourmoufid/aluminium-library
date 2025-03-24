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
#include <string.h>

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFBase.h> // Boolean
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFString.h>

#include "al.h"
#include "common.h"

static char _al_locale[64] = {0};

const char *
al_locale(void)
{
    if (strnlen(_al_locale, sizeof _al_locale) > 0)
        return _al_locale;

    // CFLocaleRef locale = CFLocaleCopyCurrent();
    CFArrayRef languages = NULL;
    CFStringRef language = NULL;
    CFDictionaryRef locale_components = NULL;
    CFLocaleIdentifier id = NULL;

    languages = CFLocaleCopyPreferredLanguages();
    if (languages == NULL)
        goto error;
    if (CFArrayGetCount(languages) == 0) {
        goto error;
    }
    language = CFArrayGetValueAtIndex(languages, 0);
    if (language == NULL) {
        goto error;
    }
    locale_components = CFLocaleCreateComponentsFromLocaleIdentifier(
        NULL,
        language
    );
    if (locale_components == NULL) {
        goto error;
    }
    id = CFLocaleCreateLocaleIdentifierFromComponents(
        NULL,
        locale_components
    );
    if (id == NULL) {
        goto error;
    }
    Boolean status = CFStringGetCString(
        id,
        _al_locale,
        sizeof _al_locale,
        _al_encoding
    );
    if (!status) {
        DEBUG(
            "CFStringGetCString(%p, ...) = %s\n",
            (const void *) language,
            status ? "true" : "false"
        );
        goto error;
    }
    CFRelease(id);
    CFRelease(locale_components);
    CFRelease(languages);

    return _al_locale;

error:
    memset(_al_locale, 0, sizeof _al_locale);
    if (id != NULL)
        CFRelease(id);
    if (locale_components != NULL)
        CFRelease(locale_components);
    if (languages != NULL)
        CFRelease(languages);
    return NULL;
}
