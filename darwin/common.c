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
#include <stdint.h> // uintptr_t
#include <string.h> // strsignal

#include <CoreFoundation/CFString.h>
#include <TargetConditionals.h> // TARGET_OS_IOS

#include "al.h"
#include "common.h"

const char *const copyright = "Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com>";
#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
const char *const platform = "ios";
#else
const char *const platform = "darwin";
#endif

CFStringEncoding _al_encoding = kCFStringEncodingUTF8;

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
al_init(void)
{
    _al_encoding = CFStringGetSystemEncoding();
    DEBUG("_al_encoding = %s", _cfstringencoding_string(_al_encoding));
    assert(CFStringIsEncodingAvailable(_al_encoding));
    catch_fatal_signals();
}
