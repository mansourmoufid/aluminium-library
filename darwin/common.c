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

#include <CoreFoundation/CFString.h>

#include "al.h"
#include "common.h"

const char *const copyright = "Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com>";
const char *const platform = "darwin";

CFStringEncoding _al_encoding = kCFStringEncodingUTF8;

__attribute__((constructor))
void
init(void)
{
    _al_encoding = CFStringGetSystemEncoding();
    DEBUG("_al_encoding = %s", _cfstringencoding_string(_al_encoding));
    assert(CFStringIsEncodingAvailable(_al_encoding));
}
