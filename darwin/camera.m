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
#include <errno.h>
#include <float.h> // FLT_MAX
#include <math.h> // sqrtf
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h> // calloc
#include <string.h> // strerror

#include <dispatch/dispatch.h> // dispatch_queue_t
#include <objc/objc.h> // BOOL, YES, NO
#include <Availability.h> // *_VERSION_MIN_REQUIRED
#include <AvailabilityVersions.h>
#include <MacTypes.h> // OSType
#include <TargetConditionals.h> // TARGET_OS_IOS, TARGET_OS_OSX

#import <CoreFoundation/CFBase.h> // Boolean, CFRetain, CFRelease

#import <Foundation/NSDictionary.h>
#import <Foundation/NSError.h>
#import <Foundation/NSNotification.h> // NSNotificationCenter
#import <Foundation/NSObjCRuntime.h> // NSLog, NSUIntegerMax
#import <Foundation/NSValue.h> // NSNumber

#import <CoreMedia/CMSampleBuffer.h> // CMSampleBufferRef
// typedef struct opaqueCMSampleBuffer *CMSampleBufferRef;
typedef const struct opaqueCMSampleBuffer *ConstCMSampleBufferRef;

#import <CoreVideo/CVImageBuffer.h> // CVImageBufferRef
#import <CoreVideo/CVPixelBuffer.h> // kCVPixelFormatType_*

#import <AVFoundation/AVCaptureDevice.h>
#import <AVFoundation/AVCaptureInput.h>
#import <AVFoundation/AVCaptureSession.h>
#import <AVFoundation/AVCaptureVideoDataOutput.h>
#import <AVFoundation/AVMediaFormat.h> // AVMediaTypeVideo
#import <AVFoundation/AVVideoSettings.h> // AVVideoCodecTypeH264

#include "al.h"

#include "common.h"
#include "camera.h"

struct al_camera {
    AlCameraController *controller;
    AVCaptureDevice *device;
    AVCaptureSession *session;
    AVCaptureDeviceInput *input;
    AVCaptureVideoDataOutput *output;
    dispatch_queue_t queue;
    AlOutputDelegate *output_delegate;
    CMSampleBufferRef sample_buffer;
    CVImageBufferRef image_buffer;
    OSType pixel_format;
    struct al_image image;
    size_t width;
    size_t height;
};

@interface AlCameraController : NSObject
@property(atomic) struct al_camera *camera;
- (instancetype) initWithCamera: (struct al_camera *)cam;
- (void) dealloc;
- (void) handleAVCaptureSessionDidStartRunning: (NSNotification *)noti;
- (void) handleAVCaptureSessionDidStopRunning: (NSNotification *)noti;
- (void) handleAVCaptureSessionRuntimeError: (NSNotification *)noti;
@end

@implementation AlCameraController

@synthesize camera;

- (instancetype) initWithCamera: (struct al_camera *)cam
{
    self = [super init];
    self.camera = cam;
    return self;
}

- (void) dealloc {
    [[NSNotificationCenter defaultCenter]
        removeObserver:self.camera->controller
    ];
    self.camera = NULL;
    [super dealloc];
}

- (void) handleAVCaptureSessionDidStartRunning: (NSNotification *)noti {
    DEBUG("AVCaptureSessionDidStartRunningNotification");
    DEBUG("%s", [noti.description UTF8String]);
}

- (void) handleAVCaptureSessionDidStopRunning: (NSNotification *)noti {
    DEBUG("AVCaptureSessionDidStopRunningNotification");
    DEBUG("%s", [noti.description UTF8String]);
}

- (void) handleAVCaptureSessionRuntimeError: (NSNotification *)noti {
    DEBUG("AVCaptureSessionRuntimeErrorNotification");
    DEBUG("%s", [noti.description UTF8String]);
}

@end

static inline
AVCaptureDevice *
get_device(struct al_camera *cam, NSUInteger index)
{
    assert(cam != NULL);
    if (@available(iOS 10.0, macOS 10.15, *)) {
        NSMutableArray *types = [[NSMutableArray alloc] init];
        assert(types != nil);
        [types addObject:AVCaptureDeviceTypeBuiltInWideAngleCamera];
#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
        if (@available(iOS 10.0, *))
            [types addObject:AVCaptureDeviceTypeBuiltInTelephotoCamera];
        if (@available(iOS 13.0, *))
            [types addObject:AVCaptureDeviceTypeBuiltInUltraWideCamera];
        if (@available(iOS 17.0, *))
            [types addObject:AVCaptureDeviceTypeExternal];
#endif
#if defined(TARGET_OS_OSX) && TARGET_OS_OSX
        if (@available(macOS 10.15, *)) {
            if (@available(macOS 14.0, *))
                [types addObject:AVCaptureDeviceTypeExternal];
            else
                [types addObject:AVCaptureDeviceTypeExternalUnknown];
        }
        // if (@available(macOS 13.0, *))
        //     [types addObject:AVCaptureDeviceTypeDeskViewCamera];
        // if (@available(macOS 14.0, *))
        //     [types addObject:AVCaptureDeviceTypeContinuityCamera];
#endif
        AVCaptureDeviceDiscoverySession *discovery_session = [
            AVCaptureDeviceDiscoverySession
            discoverySessionWithDeviceTypes:types
            mediaType:AVMediaTypeVideo
            position:AVCaptureDevicePositionUnspecified
        ];
        [types release];
        if (discovery_session == nil)
            return nil;
        NSArray *devices = [discovery_session devices];
        if (index + 1 > [devices count])
            return nil;
        AVCaptureDevice *device = devices[index];
        return device;
    } else {
        // AVCaptureDevice.devicesWithMediaType
    }
    return nil;
}

static inline
bool
_have_authorization(void)
{
    if (@available(iOS 7.0, macOS 10.14, *)) {
        AVAuthorizationStatus status = [
            AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo
        ];
        switch (status) {
            case AVAuthorizationStatusNotDetermined:
                DEBUG("AVAuthorizationStatusNotDetermined");
                break;
            case AVAuthorizationStatusRestricted:
                DEBUG("AVAuthorizationStatusRestricted");
                break;
            case AVAuthorizationStatusDenied:
                DEBUG("AVAuthorizationStatusDenied");
                break;
            case AVAuthorizationStatusAuthorized:
                DEBUG("AVAuthorizationStatusAuthorized");
                break;
        }
        if (status == AVAuthorizationStatusAuthorized) {
            return true;
        }
    }
    return false;
}

static inline
void
_request_authorization(void)
{
    if (@available(iOS 7.0, macOS 10.14, *)) {
        [AVCaptureDevice
            requestAccessForMediaType:AVMediaTypeVideo
            completionHandler:^(BOOL granted) {
                DEBUG("authorization: %s", granted ? "granted" : "not granted");
            }
        ];
    }
}

static inline
AVCaptureDeviceInput *
new_input(struct al_camera *cam)
{
    NSError *error = nil;
    AVCaptureDeviceInput *input = [AVCaptureDeviceInput
        deviceInputWithDevice:cam->device
        error:&error
    ];
    if (error != nil) {
        DEBUG("%s", [error.description UTF8String]);
        return nil;
    }
    return input;
}

static inline
AVCaptureSession *
get_session(struct al_camera *cam)
{
    (void) cam;
    return [[AVCaptureSession alloc] init];
}

static inline
float
l2norm(float x, float y, float a, float b)
{
    return sqrtf((x - a) * (x - a) + (y - b) * (y - b));
}

static inline
AVCaptureSessionPreset
nearest_preset(size_t width, size_t height)
{
    NSMutableArray *presets = [[NSMutableArray alloc] init];
    assert(presets != nil);
    [presets addObjectsFromArray:@[
        @{
            @"preset": AVCaptureSessionPreset640x480,
            @"width": @640,
            @"height": @480,
        },
        @{
            @"preset": AVCaptureSessionPreset1280x720,
            @"width": @1280,
            @"height": @720,
        },
    ]];
    if (@available(iOS 5.0, macOS 10.15, *)) {
        [presets addObject:@{
            @"preset": AVCaptureSessionPreset1920x1080,
            @"width": @1920,
            @"height": @1080,
        }];
    }
    if (@available(iOS 9.0, macOS 10.15, *)) {
        [presets addObject:@{
            @"preset": AVCaptureSessionPreset3840x2160,
            @"width": @3840,
            @"height": @2160,
        }];
    }
    size_t nearest = 0;
    float distance = FLT_MAX;
    for (size_t i = 0; i < [presets count]; i++) {
        NSDictionary *preset = [presets objectAtIndex:i];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-messaging-id"
        float norm = l2norm(
            (float) width,
            (float) height,
            [preset[@"width"] floatValue],
            [preset[@"height"] floatValue]
        );
#pragma clang diagnostic pop
        if (norm < distance) {
            nearest = i;
            distance = norm;
        }
    }
    AVCaptureSessionPreset preset = nil;
    if (distance < FLT_MAX)
        preset = presets[nearest][@"preset"];
    [presets release];
    return preset;
}

static inline
AVCaptureVideoDataOutput *
new_output(struct al_camera *cam)
{
    (void) cam;
    AVCaptureVideoDataOutput *output = [
        [AVCaptureVideoDataOutput alloc] init
    ];
    NSArray<NSNumber *> *available = [output availableVideoCVPixelFormatTypes];
    DEBUG(
        "[%s availableVideoCVPixelFormatTypes] =",
        [output.description UTF8String]
    );
    for (NSNumber *fmt in available) {
        DEBUG("    %s", _cv_pixel_format_string([fmt unsignedIntValue]));
    }
    NSMutableArray<NSNumber *> *preferred = [NSMutableArray arrayWithArray:@[
        [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8Planar],
        [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8PlanarFullRange],
        [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange],
        [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange],
    ]];
    for (NSNumber *fmt in [preferred reverseObjectEnumerator]) {
        if ([available containsObject:fmt] != YES)
            [preferred removeObjectIdenticalTo:fmt];
    }
    DEBUG("preferred format types:");
    for (NSNumber *fmt in preferred) {
        DEBUG("    %s", _cv_pixel_format_string([fmt unsignedIntValue]));
    }
    [available release];
    [output setVideoSettings:@{
        (__bridge NSString *) kCVPixelBufferPixelFormatTypeKey: preferred,
        /*
        (__bridge NSString *) kCVPixelBufferPixelFormatTypeKey: @[
            [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8Planar],
            [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8PlanarFullRange],
            [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange],
            [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange],
        ],
        */
    }];
    [preferred release];
    return output;
}

static
void
process_image(struct al_camera *cam, CVImageBufferRef image)
{
    assert(cam != NULL);
    assert(image != NULL);

    // CGSize size = CVImageBufferGetEncodedSize(image);
    // cam->image.width = (size_t) size.width;
    // cam->image.height = (size_t) size.height;

    assert(CFGetTypeID(image) == CVPixelBufferGetTypeID());
    CVPixelBufferRef pixel_buffer = image;
    if (pixel_buffer == NULL)
        goto error0;
    CVPixelBufferRetain(pixel_buffer);

    CVPixelBufferLockFlags lock = kCVPixelBufferLock_ReadOnly;
    CVReturn status = CVPixelBufferLockBaseAddress(pixel_buffer, lock);
    if (status != kCVReturnSuccess) {
        DEBUG_CV("CVPixelBufferLockBaseAddress", status);
        goto error1;
    }

    OSType format = CVPixelBufferGetPixelFormatType(pixel_buffer);
    // DEBUG("%s", _cv_pixel_format_string(format));
    cam->pixel_format = format;
    switch (cam->pixel_format) {
        case kCVPixelFormatType_420YpCbCr8Planar:
        case kCVPixelFormatType_420YpCbCr8PlanarFullRange:
            cam->image.format = AL_COLOR_FORMAT_YUV420P;
            break;
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
            cam->image.format = AL_COLOR_FORMAT_YUV420SP;
            break;
        case kCVPixelFormatType_32BGRA:
            cam->image.format = AL_COLOR_FORMAT_RGBA;
            break;
        default:
            DEBUG("%s", _cv_pixel_format_string(cam->pixel_format));
            cam->image.format = AL_COLOR_FORMAT_UNKNOWN;
            break;
    }
    // .. .
    switch (cam->image.format) {
        case AL_COLOR_FORMAT_YUV420P:
            break;
        case AL_COLOR_FORMAT_YUV420SP:
            break;
        case AL_COLOR_FORMAT_RGBA:
        case AL_COLOR_FORMAT_UNKNOWN:
            goto error2;
    }

#if defined(TARGET_OS_OSX) && TARGET_OS_OSX
    CGColorSpaceRef color_space = CVImageBufferGetColorSpace(pixel_buffer);
#endif

    Boolean planar = CVPixelBufferIsPlanar(pixel_buffer);
    if (planar) {
        size_t width = CVPixelBufferGetWidth(pixel_buffer);
        size_t height = CVPixelBufferGetHeight(pixel_buffer);
        size_t stride = CVPixelBufferGetBytesPerRow(pixel_buffer);
        void *data = CVPixelBufferGetBaseAddress(pixel_buffer);
        assert(data != NULL);
        cam->image.width = width;
        cam->image.height = height;
    } else {
        size_t n = CVPixelBufferGetPlaneCount(pixel_buffer);
        size_t i = 0;
        void *y = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, i);
        assert(y != NULL);
        size_t width = CVPixelBufferGetWidthOfPlane(pixel_buffer, i);
        size_t height = CVPixelBufferGetHeightOfPlane(pixel_buffer, i);
        size_t stride = CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, i);
        cam->image.width = width;
        cam->image.height = height;
    }

error:
error2:
    status = CVPixelBufferUnlockBaseAddress(pixel_buffer, lock);
    if (status != kCVReturnSuccess) {
        DEBUG_CV("CVPixelBufferUnlockBaseAddress", status);
    }
error1:
    CVPixelBufferRelease(pixel_buffer);
error0:
    return;
}

@interface AlOutputDelegate<AVCaptureVideoDataOutputSampleBufferDelegate>
    : NSObject
@property(atomic) struct al_camera *camera;
- (instancetype) initWithCamera: (struct al_camera *)cam;
- (void) dealloc;
- (void) captureOutput: (AVCaptureOutput *)output 
    didOutputSampleBuffer: (CMSampleBufferRef)sampleBuffer 
    fromConnection: (AVCaptureConnection *)connection;
- (void) captureOutput: (AVCaptureOutput *)output 
    didDropSampleBuffer: (CMSampleBufferRef)sampleBuffer 
    fromConnection: (AVCaptureConnection *)connection;
@end

@implementation AlOutputDelegate

@synthesize camera;

- (instancetype) initWithCamera: (struct al_camera *)cam
{
    self = [super init];
    self.camera = cam;
    return self;
}

- (void) dealloc {
    self.camera = NULL;
    [super dealloc];
}

- (void) captureOutput: (AVCaptureOutput *)output 
    didOutputSampleBuffer: (CMSampleBufferRef)sampleBuffer 
    fromConnection: (AVCaptureConnection *)connection
{
    // DEBUG("didOutputSampleBuffer: %p", sampleBuffer);
    if (sampleBuffer == NULL)
        return;
    if (self.camera->sample_buffer != NULL)
        CFRelease(self.camera->sample_buffer);
    self.camera->sample_buffer = CFRetain(sampleBuffer);
    CVImageBufferRef image = CMSampleBufferGetImageBuffer(
        self.camera->sample_buffer
    );
    if (image == NULL)
        return;
    CFRetain(image);
    process_image(self.camera, image);
    CFRelease(image);
}

- (void) captureOutput: (AVCaptureOutput *)output 
    didDropSampleBuffer: (CMSampleBufferRef)sampleBuffer 
    fromConnection: (AVCaptureConnection *)connection
{
    DEBUG("didDropSampleBuffer");
}

@end

enum al_status
al_camera_new(
    struct al_camera **cam,
    size_t index,
    size_t width,
    size_t height
) {
    assert(cam != NULL);
    assert(index <= NSUIntegerMax);
    (void) width;
    (void) height;

    errno = 0;
    *cam = calloc(1, sizeof (struct al_camera));
    if (*cam == NULL) {
        DEBUG("calloc: errno=%i [%s]", errno, strerror(errno));
        goto error0;
    }

    (*cam)->controller = [[AlCameraController alloc] initWithCamera:(*cam)];
    if ((*cam)->controller == nil)
        goto error1;

    (*cam)->device = get_device(*cam, index);
    if ((*cam)->device == nil)
        goto error2;

    if (!_have_authorization()) {
        _request_authorization();
        goto error3;
    }

    (*cam)->session = get_session(*cam);
    if ((*cam)->session == nil)
        goto error4;

    [[NSNotificationCenter defaultCenter]
        addObserver:(*cam)->controller
        selector:@selector(handleAVCaptureSessionDidStartRunning:)
        name:AVCaptureSessionDidStartRunningNotification
        object:(*cam)->session
    ];
    [[NSNotificationCenter defaultCenter]
        addObserver:(*cam)->controller
        selector:@selector(handleAVCaptureSessionDidStopRunning:)
        name:AVCaptureSessionDidStopRunningNotification
        object:(*cam)->session
    ];
    [[NSNotificationCenter defaultCenter]
        addObserver:(*cam)->controller
        selector:@selector(handleAVCaptureSessionRuntimeError:)
        name:AVCaptureSessionRuntimeErrorNotification
        object:(*cam)->session
    ];

    [(*cam)->session beginConfiguration];
    AVCaptureSessionPreset preset = nearest_preset(width, height);
    DEBUG("%s", [preset.description UTF8String]);
    if (preset == nil)
        preset = AVCaptureSessionPresetHigh;
    if ([(*cam)->session canSetSessionPreset:preset]) {
        (*cam)->session.sessionPreset = preset;
        (*cam)->width = width;
        (*cam)->height = height;
    }
    [(*cam)->session commitConfiguration];

    (*cam)->input = new_input(*cam);
    if ((*cam)->input == nil)
        goto error5;
    if (![(*cam)->session canAddInput:(*cam)->input])
        goto error6;
    [(*cam)->session addInput:(*cam)->input];

    (*cam)->queue = dispatch_queue_create(NULL, DISPATCH_QUEUE_SERIAL);
    if ((*cam)->queue == NULL)
        goto error7;
    dispatch_retain((*cam)->queue);

    (*cam)->output = new_output(*cam);
    if ((*cam)->output == nil)
        goto error8;
    if (![(*cam)->session canAddOutput:(*cam)->output])
        goto error9;
    [(*cam)->session addOutput:(*cam)->output];
    (*cam)->output_delegate = [[AlOutputDelegate alloc] initWithCamera:(*cam)];
    if ((*cam)->output_delegate == nil)
        goto error10;
    [(*cam)->output
        setSampleBufferDelegate:((id) (*cam)->output_delegate)
        queue:(*cam)->queue
    ];

    /*
    NSArray *codecs = [(*cam)->output availableVideoCodecTypes];
    if (@available(iOS 11.0, macOS 10.13, *)) {
        if (![codecs containsObject:AVVideoCodecTypeH264])
            goto error11;
    } else {
        if (![codecs containsObject:AVVideoCodecH264])
            goto error11;
    }
    */

    return AL_OK;

error11:
    if ((*cam)->sample_buffer != NULL)
        CFRelease((*cam)->sample_buffer);
    (*cam)->sample_buffer = NULL;
    [(*cam)->output_delegate release];
    (*cam)->output_delegate = nil;
error10:
error9:
    [(*cam)->output setSampleBufferDelegate:nil queue:NULL];
    if ([[(*cam)->session outputs] containsObject:(*cam)->output])
        [(*cam)->session removeOutput:(*cam)->output];
    [(*cam)->output release];
    (*cam)->output = nil;
error8:
    dispatch_release((*cam)->queue);
    (*cam)->queue = NULL;
error7:
error6:
    if ([[(*cam)->session inputs] containsObject:(*cam)->input])
        [(*cam)->session removeInput:(*cam)->input];
    [(*cam)->input release];
    (*cam)->input = nil;
error5:
    [(*cam)->session release];
    (*cam)->session = nil;
error4:
error3:
    [(*cam)->device release];
    (*cam)->device = nil;
error2:
    [(*cam)->controller release];
    (*cam)->controller = nil;
error1:
    free(*cam);
    *cam = NULL;
error0:
    return AL_ERROR;
}

void
al_camera_free(struct al_camera *cam)
{
    if (cam == NULL)
        return;
    if (cam->session != nil) {
        if ([cam->session isRunning])
            [cam->session stopRunning];
    }
    if (cam->sample_buffer != NULL)
        CFRelease(cam->sample_buffer);
    cam->sample_buffer = NULL;
    if (cam->output_delegate != nil)
        [cam->output_delegate release];
    cam->output_delegate = nil;
    if (cam->output != nil) {
        [cam->output setSampleBufferDelegate:nil queue:NULL];
        if ([[cam->session outputs] containsObject:cam->output])
            [cam->session removeOutput:cam->output];
        [cam->output release];
    }
    cam->output = nil;
    if (cam->queue != NULL)
        dispatch_release(cam->queue);
    cam->queue = NULL;
    if (cam->input != nil) {
        if ([[cam->session inputs] containsObject:cam->input])
            [cam->session removeInput:cam->input];
        [cam->input release];
    }
    cam->input = nil;
    if (cam->session != nil)
        [cam->session release];
    cam->session = nil;
    if (cam->device != nil)
        [cam->device release];
    cam->device = nil;
    if (cam->controller != nil)
        [cam->controller release];
    cam->controller = nil;
    free(cam);
}

void
al_camera_start(struct al_camera *cam)
{
    assert(cam != NULL);
    assert(cam->session != nil);
    [cam->session startRunning];
}

void
al_camera_stop(struct al_camera *cam)
{
    assert(cam != NULL);
    assert(cam->session != nil);
    if ([cam->session isRunning])
        [cam->session stopRunning];
}

enum al_status
al_camera_get_id(struct al_camera *cam, const char **id)
{
    NSString *name = [cam->device localizedName];
    if (name != nil) {
        *id = [name UTF8String];
        return AL_OK;
    }
    return AL_ERROR;
}

enum al_status
al_camera_get_color_format(struct al_camera *cam, enum al_color_format *format)
{
    assert(cam != NULL);
    assert(format != NULL);
    *format = cam->image.format;
    return AL_OK;
}

enum al_status
al_camera_get_width(struct al_camera *cam, size_t *width)
{
    assert(cam != NULL);
    assert(width != NULL);
    *width = cam->image.width;
    return AL_OK;
}

enum al_status
al_camera_get_height(struct al_camera *cam, size_t *height)
{
    assert(cam != NULL);
    assert(height != NULL);
    *height = cam->image.height;
    return AL_OK;
}

enum al_status
al_camera_get_data(struct al_camera *cam, enum al_color_format format, void **data)
{
    assert(cam != NULL);
    (void) format;
    assert(data != NULL);
    return AL_NOTIMPLEMENTED;
}

enum al_status
al_camera_get_rgba(struct al_camera *cam, void **data)
{
    assert(cam != NULL);
    assert(data != NULL);
    return AL_NOTIMPLEMENTED;
}

enum al_status
al_camera_get_facing(struct al_camera *cam, enum al_camera_facing *facing)
{
    assert(cam != NULL);
    assert(facing != NULL);
    return AL_NOTIMPLEMENTED;
}

enum al_status
al_camera_get_orientation(struct al_camera *cam, int *orientation)
{
    assert(cam != NULL);
    assert(orientation != NULL);
    return AL_NOTIMPLEMENTED;
}

enum al_status
al_camera_set_stride(struct al_camera *cam, size_t stride)
{
    return AL_NOTIMPLEMENTED;
}
