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

#include <stddef.h>

#include <TargetConditionals.h> // TARGET_OS_IOS, TARGET_OS_OSX

#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
#import <UIKit/UIKit.h>
#endif

#include "al.h"

int
al_display_orientation(void)
{
#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    switch (orientation) {
        case UIDeviceOrientationPortrait:
            return 0;
        case UIDeviceOrientationLandscapeRight:
            return 90;
        case UIDeviceOrientationPortraitUpsideDown:
            return 180;
        case UIDeviceOrientationLandscapeLeft:
            return 270;
        case UIDeviceOrientationFaceUp:
        case UIDeviceOrientationFaceDown:
        case UIDeviceOrientationUnknown:
            return 0;
    }
#endif
    return 0;
}
