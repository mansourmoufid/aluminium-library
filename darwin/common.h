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

#pragma once

#include <stdio.h>
#include <unistd.h> // isatty, STDERR_FILENO

#include <os/log.h>
#include <MacTypes.h> // OSStatus, OSType

#include <CoreFoundation/CFString.h>
// #include <CoreFoundation/CFStringEncodingExt.h>

#include <CoreMedia/CMFormatDescription.h>
#include <CoreMedia/CMFormatDescriptionBridge.h>
#include <CoreMedia/CMBlockBuffer.h>
#include <CoreMedia/CMSampleBuffer.h>

#include <CoreVideo/CVReturn.h>

#include <VideoToolbox/VTErrors.h>

#include <Accelerate/Accelerate.h> // vImage_Error

#if defined(NDEBUG)
#define DEBUG(...) 
#else
#define DEBUG(...) { \
    if (isatty(STDERR_FILENO) != 0) { \
        fprintf(stderr, "%s: ", __func__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } else { \
        os_log_debug(OS_LOG_DEFAULT, "%s: ", __func__); \
        os_log_debug(OS_LOG_DEFAULT, __VA_ARGS__); \
    } \
}
#endif

extern CFStringEncoding _al_encoding;

// CoreFoundation/CFString.h
static inline
const char *
_cfstringencoding_string(CFStringEncoding encoding)
{
    switch (encoding) {
        case kCFStringEncodingASCII: return "kCFStringEncodingASCII";
        case kCFStringEncodingISOLatin1: return "kCFStringEncodingISOLatin1";
        case kCFStringEncodingMacRoman: return "kCFStringEncodingMacRoman";
        case kCFStringEncodingNextStepLatin: return "kCFStringEncodingNextStepLatin";
        case kCFStringEncodingNonLossyASCII: return "kCFStringEncodingNonLossyASCII";
        case kCFStringEncodingUTF16: return "kCFStringEncodingUTF16";
        case kCFStringEncodingUTF16BE: return "kCFStringEncodingUTF16BE";
        case kCFStringEncodingUTF16LE: return "kCFStringEncodingUTF16LE";
        case kCFStringEncodingUTF32: return "kCFStringEncodingUTF32";
        case kCFStringEncodingUTF32BE: return "kCFStringEncodingUTF32BE";
        case kCFStringEncodingUTF32LE: return "kCFStringEncodingUTF32LE";
        case kCFStringEncodingUTF8: return "kCFStringEncodingUTF8";
        // case kCFStringEncodingUnicode: return "kCFStringEncodingUnicode";
        case kCFStringEncodingWindowsLatin1: return "kCFStringEncodingWindowsLatin1";
        case kCFStringEncodingInvalidId: return "kCFStringEncodingInvalidId";
        default: return NULL;
    }
}

static
const char *
_cm_status_string(const OSStatus status)
{
    switch (status) {
        case kCMFormatDescriptionError_AllocationFailed: return "kCMFormatDescriptionError_AllocationFailed";
        case kCMFormatDescriptionError_InvalidParameter: return "kCMFormatDescriptionError_InvalidParameter";
        case kCMFormatDescriptionError_ValueNotAvailable: return "kCMFormatDescriptionError_ValueNotAvailable";
        case kCMFormatDescriptionBridgeError_InvalidParameter: return "kCMFormatDescriptionBridgeError_InvalidParameter";
        case kCMFormatDescriptionBridgeError_AllocationFailed: return "kCMFormatDescriptionBridgeError_AllocationFailed";
        case kCMFormatDescriptionBridgeError_InvalidSerializedSampleDescription: return "kCMFormatDescriptionBridgeError_InvalidSerializedSampleDescription";
        case kCMFormatDescriptionBridgeError_InvalidFormatDescription: return "kCMFormatDescriptionBridgeError_InvalidFormatDescription";
        case kCMFormatDescriptionBridgeError_IncompatibleFormatDescription: return "kCMFormatDescriptionBridgeError_IncompatibleFormatDescription";
        case kCMFormatDescriptionBridgeError_UnsupportedSampleDescriptionFlavor: return "kCMFormatDescriptionBridgeError_UnsupportedSampleDescriptionFlavor";
        case kCMFormatDescriptionBridgeError_InvalidSlice: return "kCMFormatDescriptionBridgeError_InvalidSlice";
        case kCMBlockBufferStructureAllocationFailedErr: return "kCMBlockBufferStructureAllocationFailedErr";
        case kCMBlockBufferBlockAllocationFailedErr: return "kCMBlockBufferBlockAllocationFailedErr";
        case kCMBlockBufferBadCustomBlockSourceErr: return "kCMBlockBufferBadCustomBlockSourceErr";
        case kCMBlockBufferBadOffsetParameterErr: return "kCMBlockBufferBadOffsetParameterErr";
        case kCMBlockBufferBadLengthParameterErr: return "kCMBlockBufferBadLengthParameterErr";
        case kCMBlockBufferBadPointerParameterErr: return "kCMBlockBufferBadPointerParameterErr";
        case kCMBlockBufferEmptyBBufErr: return "kCMBlockBufferEmptyBBufErr";
        case kCMBlockBufferUnallocatedBlockErr: return "kCMBlockBufferUnallocatedBlockErr";
        case kCMBlockBufferInsufficientSpaceErr: return "kCMBlockBufferInsufficientSpaceErr";
        case kCMSampleBufferError_AllocationFailed: return "kCMSampleBufferError_AllocationFailed";
        case kCMSampleBufferError_RequiredParameterMissing: return "kCMSampleBufferError_RequiredParameterMissing";
        case kCMSampleBufferError_AlreadyHasDataBuffer: return "kCMSampleBufferError_AlreadyHasDataBuffer";
        case kCMSampleBufferError_BufferNotReady: return "kCMSampleBufferError_BufferNotReady";
        case kCMSampleBufferError_SampleIndexOutOfRange: return "kCMSampleBufferError_SampleIndexOutOfRange";
        case kCMSampleBufferError_BufferHasNoSampleSizes: return "kCMSampleBufferError_BufferHasNoSampleSizes";
        case kCMSampleBufferError_BufferHasNoSampleTimingInfo: return "kCMSampleBufferError_BufferHasNoSampleTimingInfo";
        case kCMSampleBufferError_ArrayTooSmall: return "kCMSampleBufferError_ArrayTooSmall";
        case kCMSampleBufferError_InvalidEntryCount: return "kCMSampleBufferError_InvalidEntryCount";
        case kCMSampleBufferError_CannotSubdivide: return "kCMSampleBufferError_CannotSubdivide";
        case kCMSampleBufferError_SampleTimingInfoInvalid: return "kCMSampleBufferError_SampleTimingInfoInvalid";
        case kCMSampleBufferError_InvalidMediaTypeForOperation: return "kCMSampleBufferError_InvalidMediaTypeForOperation";
        case kCMSampleBufferError_InvalidSampleData: return "kCMSampleBufferError_InvalidSampleData";
        case kCMSampleBufferError_InvalidMediaFormat: return "kCMSampleBufferError_InvalidMediaFormat";
        case kCMSampleBufferError_Invalidated: return "kCMSampleBufferError_Invalidated";
        case kCMSampleBufferError_DataFailed: return "kCMSampleBufferError_DataFailed";
        case kCMSampleBufferError_DataCanceled: return "kCMSampleBufferError_DataCanceled";
        default: break;
    }
    return "";
}

static
const char *
_cv_status_string(const OSStatus status)
{
    switch (status) {
        case kCVReturnSuccess: return "kCVReturnSuccess";
        case kCVReturnError: return "kCVReturnError";
        case kCVReturnInvalidArgument: return "kCVReturnInvalidArgument";
        case kCVReturnAllocationFailed: return "kCVReturnAllocationFailed";
        case kCVReturnUnsupported: return "kCVReturnUnsupported";
        case kCVReturnInvalidDisplay: return "kCVReturnInvalidDisplay";
        case kCVReturnDisplayLinkAlreadyRunning: return "kCVReturnDisplayLinkAlreadyRunning";
        case kCVReturnDisplayLinkNotRunning: return "kCVReturnDisplayLinkNotRunning";
        case kCVReturnDisplayLinkCallbacksNotSet: return "kCVReturnDisplayLinkCallbacksNotSet";
        case kCVReturnInvalidPixelFormat: return "kCVReturnInvalidPixelFormat";
        case kCVReturnInvalidSize: return "kCVReturnInvalidSize";
        case kCVReturnInvalidPixelBufferAttributes: return "kCVReturnInvalidPixelBufferAttributes";
        case kCVReturnPixelBufferNotOpenGLCompatible: return "kCVReturnPixelBufferNotOpenGLCompatible";
        case kCVReturnPixelBufferNotMetalCompatible: return "kCVReturnPixelBufferNotMetalCompatible";
        case kCVReturnWouldExceedAllocationThreshold: return "kCVReturnWouldExceedAllocationThreshold";
        case kCVReturnPoolAllocationFailed: return "kCVReturnPoolAllocationFailed";
        case kCVReturnInvalidPoolAttributes: return "kCVReturnInvalidPoolAttributes";
        case kCVReturnRetry: return "kCVReturnRetry";
        default: break;
    }
    return "";
}

static
const char *
_cv_pixel_format_string(OSType format)
{
    switch (format) {
        case kCVPixelFormatType_1Monochrome: return "kCVPixelFormatType_1Monochrome";
        case kCVPixelFormatType_2Indexed: return "kCVPixelFormatType_2Indexed";
        case kCVPixelFormatType_4Indexed: return "kCVPixelFormatType_4Indexed";
        case kCVPixelFormatType_8Indexed: return "kCVPixelFormatType_8Indexed";
        case kCVPixelFormatType_1IndexedGray_WhiteIsZero: return "kCVPixelFormatType_1IndexedGray_WhiteIsZero";
        case kCVPixelFormatType_2IndexedGray_WhiteIsZero: return "kCVPixelFormatType_2IndexedGray_WhiteIsZero";
        case kCVPixelFormatType_4IndexedGray_WhiteIsZero: return "kCVPixelFormatType_4IndexedGray_WhiteIsZero";
        case kCVPixelFormatType_8IndexedGray_WhiteIsZero: return "kCVPixelFormatType_8IndexedGray_WhiteIsZero";
        case kCVPixelFormatType_16BE555: return "kCVPixelFormatType_16BE555";
        case kCVPixelFormatType_16LE555: return "kCVPixelFormatType_16LE555";
        case kCVPixelFormatType_16LE5551: return "kCVPixelFormatType_16LE5551";
        case kCVPixelFormatType_16BE565: return "kCVPixelFormatType_16BE565";
        case kCVPixelFormatType_16LE565: return "kCVPixelFormatType_16LE565";
        case kCVPixelFormatType_24RGB: return "kCVPixelFormatType_24RGB";
        case kCVPixelFormatType_24BGR: return "kCVPixelFormatType_24BGR";
        case kCVPixelFormatType_32ARGB: return "kCVPixelFormatType_32ARGB";
        case kCVPixelFormatType_32BGRA: return "kCVPixelFormatType_32BGRA";
        case kCVPixelFormatType_32ABGR: return "kCVPixelFormatType_32ABGR";
        case kCVPixelFormatType_32RGBA: return "kCVPixelFormatType_32RGBA";
        case kCVPixelFormatType_64ARGB: return "kCVPixelFormatType_64ARGB";
        case kCVPixelFormatType_64RGBALE: return "kCVPixelFormatType_64RGBALE";
        case kCVPixelFormatType_48RGB: return "kCVPixelFormatType_48RGB";
        case kCVPixelFormatType_32AlphaGray: return "kCVPixelFormatType_32AlphaGray";
        case kCVPixelFormatType_16Gray: return "kCVPixelFormatType_16Gray";
        case kCVPixelFormatType_30RGB: return "kCVPixelFormatType_30RGB";
        case kCVPixelFormatType_422YpCbCr8: return "kCVPixelFormatType_422YpCbCr8";
        case kCVPixelFormatType_4444YpCbCrA8: return "kCVPixelFormatType_4444YpCbCrA8";
        case kCVPixelFormatType_4444YpCbCrA8R: return "kCVPixelFormatType_4444YpCbCrA8R";
        case kCVPixelFormatType_4444AYpCbCr8: return "kCVPixelFormatType_4444AYpCbCr8";
        case kCVPixelFormatType_4444AYpCbCr16: return "kCVPixelFormatType_4444AYpCbCr16";
        case kCVPixelFormatType_444YpCbCr8: return "kCVPixelFormatType_444YpCbCr8";
        case kCVPixelFormatType_422YpCbCr16: return "kCVPixelFormatType_422YpCbCr16";
        case kCVPixelFormatType_422YpCbCr10: return "kCVPixelFormatType_422YpCbCr10";
        case kCVPixelFormatType_444YpCbCr10: return "kCVPixelFormatType_444YpCbCr10";
        case kCVPixelFormatType_420YpCbCr8Planar: return "kCVPixelFormatType_420YpCbCr8Planar";
        case kCVPixelFormatType_420YpCbCr8PlanarFullRange: return "kCVPixelFormatType_420YpCbCr8PlanarFullRange";
        case kCVPixelFormatType_422YpCbCr_4A_8BiPlanar: return "kCVPixelFormatType_422YpCbCr_4A_8BiPlanar";
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange: return "kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange";
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange: return "kCVPixelFormatType_420YpCbCr8BiPlanarFullRange";
        case kCVPixelFormatType_422YpCbCr8BiPlanarVideoRange: return "kCVPixelFormatType_422YpCbCr8BiPlanarVideoRange";
        case kCVPixelFormatType_422YpCbCr8BiPlanarFullRange: return "kCVPixelFormatType_422YpCbCr8BiPlanarFullRange";
        case kCVPixelFormatType_444YpCbCr8BiPlanarVideoRange: return "kCVPixelFormatType_444YpCbCr8BiPlanarVideoRange";
        case kCVPixelFormatType_444YpCbCr8BiPlanarFullRange: return "kCVPixelFormatType_444YpCbCr8BiPlanarFullRange";
        case kCVPixelFormatType_422YpCbCr8_yuvs: return "kCVPixelFormatType_422YpCbCr8_yuvs";
        case kCVPixelFormatType_422YpCbCr8FullRange: return "kCVPixelFormatType_422YpCbCr8FullRange";
        case kCVPixelFormatType_OneComponent8: return "kCVPixelFormatType_OneComponent8";
        case kCVPixelFormatType_TwoComponent8: return "kCVPixelFormatType_TwoComponent8";
        case kCVPixelFormatType_30RGBLEPackedWideGamut: return "kCVPixelFormatType_30RGBLEPackedWideGamut";
        case kCVPixelFormatType_ARGB2101010LEPacked: return "kCVPixelFormatType_ARGB2101010LEPacked";
        case kCVPixelFormatType_40ARGBLEWideGamut: return "kCVPixelFormatType_40ARGBLEWideGamut";
        case kCVPixelFormatType_40ARGBLEWideGamutPremultiplied: return "kCVPixelFormatType_40ARGBLEWideGamutPremultiplied";
        case kCVPixelFormatType_OneComponent10: return "kCVPixelFormatType_OneComponent10";
        case kCVPixelFormatType_OneComponent12: return "kCVPixelFormatType_OneComponent12";
        case kCVPixelFormatType_OneComponent16: return "kCVPixelFormatType_OneComponent16";
        case kCVPixelFormatType_TwoComponent16: return "kCVPixelFormatType_TwoComponent16";
        case kCVPixelFormatType_OneComponent16Half: return "kCVPixelFormatType_OneComponent16Half";
        case kCVPixelFormatType_OneComponent32Float: return "kCVPixelFormatType_OneComponent32Float";
        case kCVPixelFormatType_TwoComponent16Half: return "kCVPixelFormatType_TwoComponent16Half";
        case kCVPixelFormatType_TwoComponent32Float: return "kCVPixelFormatType_TwoComponent32Float";
        case kCVPixelFormatType_64RGBAHalf: return "kCVPixelFormatType_64RGBAHalf";
        case kCVPixelFormatType_128RGBAFloat: return "kCVPixelFormatType_128RGBAFloat";
        case kCVPixelFormatType_14Bayer_GRBG: return "kCVPixelFormatType_14Bayer_GRBG";
        case kCVPixelFormatType_14Bayer_RGGB: return "kCVPixelFormatType_14Bayer_RGGB";
        case kCVPixelFormatType_14Bayer_BGGR: return "kCVPixelFormatType_14Bayer_BGGR";
        case kCVPixelFormatType_14Bayer_GBRG: return "kCVPixelFormatType_14Bayer_GBRG";
        case kCVPixelFormatType_DisparityFloat16: return "kCVPixelFormatType_DisparityFloat16";
        case kCVPixelFormatType_DisparityFloat32: return "kCVPixelFormatType_DisparityFloat32";
        case kCVPixelFormatType_DepthFloat16: return "kCVPixelFormatType_DepthFloat16";
        case kCVPixelFormatType_DepthFloat32: return "kCVPixelFormatType_DepthFloat32";
        case kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange: return "kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange";
        case kCVPixelFormatType_422YpCbCr10BiPlanarVideoRange: return "kCVPixelFormatType_422YpCbCr10BiPlanarVideoRange";
        case kCVPixelFormatType_444YpCbCr10BiPlanarVideoRange: return "kCVPixelFormatType_444YpCbCr10BiPlanarVideoRange";
        case kCVPixelFormatType_420YpCbCr10BiPlanarFullRange: return "kCVPixelFormatType_420YpCbCr10BiPlanarFullRange";
        case kCVPixelFormatType_422YpCbCr10BiPlanarFullRange: return "kCVPixelFormatType_422YpCbCr10BiPlanarFullRange";
        case kCVPixelFormatType_444YpCbCr10BiPlanarFullRange: return "kCVPixelFormatType_444YpCbCr10BiPlanarFullRange";
        case kCVPixelFormatType_420YpCbCr8VideoRange_8A_TriPlanar: return "kCVPixelFormatType_420YpCbCr8VideoRange_8A_TriPlanar";
        case kCVPixelFormatType_16VersatileBayer: return "kCVPixelFormatType_16VersatileBayer";
        case kCVPixelFormatType_64RGBA_DownscaledProResRAW: return "kCVPixelFormatType_64RGBA_DownscaledProResRAW";
        case kCVPixelFormatType_422YpCbCr16BiPlanarVideoRange: return "kCVPixelFormatType_422YpCbCr16BiPlanarVideoRange";
        case kCVPixelFormatType_444YpCbCr16BiPlanarVideoRange: return "kCVPixelFormatType_444YpCbCr16BiPlanarVideoRange";
        case kCVPixelFormatType_444YpCbCr16VideoRange_16A_TriPlanar: return "kCVPixelFormatType_444YpCbCr16VideoRange_16A_TriPlanar";
        case kCVPixelFormatType_Lossless_32BGRA: return "kCVPixelFormatType_Lossless_32BGRA";
        case kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange: return "kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange";
        case kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarFullRange: return "kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarFullRange";
        case kCVPixelFormatType_Lossless_420YpCbCr10PackedBiPlanarVideoRange: return "kCVPixelFormatType_Lossless_420YpCbCr10PackedBiPlanarVideoRange";
        case kCVPixelFormatType_Lossless_422YpCbCr10PackedBiPlanarVideoRange: return "kCVPixelFormatType_Lossless_422YpCbCr10PackedBiPlanarVideoRange";
        case kCVPixelFormatType_Lossy_32BGRA: return "kCVPixelFormatType_Lossy_32BGRA";
        case kCVPixelFormatType_Lossy_420YpCbCr8BiPlanarVideoRange: return "kCVPixelFormatType_Lossy_420YpCbCr8BiPlanarVideoRange";
        case kCVPixelFormatType_Lossy_420YpCbCr8BiPlanarFullRange: return "kCVPixelFormatType_Lossy_420YpCbCr8BiPlanarFullRange";
        case kCVPixelFormatType_Lossy_420YpCbCr10PackedBiPlanarVideoRange: return "kCVPixelFormatType_Lossy_420YpCbCr10PackedBiPlanarVideoRange";
        case kCVPixelFormatType_Lossy_422YpCbCr10PackedBiPlanarVideoRange: return "kCVPixelFormatType_Lossy_422YpCbCr10PackedBiPlanarVideoRange";
        default: break;
    }
    return "";
}

static
const char *
_vimage_errors(vImage_Error error)
{
    switch (error) {
        case kvImageNoError: return "kvImageNoError";
        case kvImageRoiLargerThanInputBuffer: return "kvImageRoiLargerThanInputBuffer";
        case kvImageInvalidKernelSize: return "kvImageInvalidKernelSize";
        case kvImageInvalidEdgeStyle: return "kvImageInvalidEdgeStyle";
        case kvImageInvalidOffset_X: return "kvImageInvalidOffset_X";
        case kvImageInvalidOffset_Y: return "kvImageInvalidOffset_Y";
        case kvImageMemoryAllocationError: return "kvImageMemoryAllocationError";
        case kvImageNullPointerArgument: return "kvImageNullPointerArgument";
        case kvImageInvalidParameter: return "kvImageInvalidParameter";
        case kvImageBufferSizeMismatch: return "kvImageBufferSizeMismatch";
        case kvImageUnknownFlagsBit: return "kvImageUnknownFlagsBit";
        case kvImageInternalError: return "kvImageInternalError";
        case kvImageInvalidRowBytes: return "kvImageInvalidRowBytes";
        case kvImageInvalidImageFormat: return "kvImageInvalidImageFormat";
        case kvImageColorSyncIsAbsent: return "kvImageColorSyncIsAbsent";
        case kvImageOutOfPlaceOperationRequired: return "kvImageOutOfPlaceOperationRequired";
        case kvImageInvalidImageObject: return "kvImageInvalidImageObject";
        case kvImageInvalidCVImageFormat: return "kvImageInvalidCVImageFormat";
        case kvImageUnsupportedConversion: return "kvImageUnsupportedConversion";
        case kvImageCoreVideoIsAbsent: return "kvImageCoreVideoIsAbsent";
        default: break;
    }
    return "";
}

static
const char *
_vt_status_string(const OSStatus status)
{
    switch (status) {
        case kVTPropertyNotSupportedErr: return "kVTPropertyNotSupportedErr";
        case kVTPropertyReadOnlyErr: return "kVTPropertyReadOnlyErr";
        case kVTParameterErr: return "kVTParameterErr";
        case kVTInvalidSessionErr: return "kVTInvalidSessionErr";
        case kVTAllocationFailedErr: return "kVTAllocationFailedErr";
        case kVTPixelTransferNotSupportedErr: return "kVTPixelTransferNotSupportedErr";
        case kVTCouldNotFindVideoDecoderErr: return "kVTCouldNotFindVideoDecoderErr";
        case kVTCouldNotCreateInstanceErr: return "kVTCouldNotCreateInstanceErr";
        case kVTCouldNotFindVideoEncoderErr: return "kVTCouldNotFindVideoEncoderErr";
        case kVTVideoDecoderBadDataErr: return "kVTVideoDecoderBadDataErr";
        case kVTVideoDecoderUnsupportedDataFormatErr: return "kVTVideoDecoderUnsupportedDataFormatErr";
        case kVTVideoDecoderMalfunctionErr: return "kVTVideoDecoderMalfunctionErr";
        case kVTVideoEncoderMalfunctionErr: return "kVTVideoEncoderMalfunctionErr";
        case kVTVideoDecoderNotAvailableNowErr: return "kVTVideoDecoderNotAvailableNowErr";
        case kVTImageRotationNotSupportedErr: return "kVTImageRotationNotSupportedErr";
        case kVTVideoEncoderNotAvailableNowErr: return "kVTVideoEncoderNotAvailableNowErr";
        case kVTFormatDescriptionChangeNotSupportedErr: return "kVTFormatDescriptionChangeNotSupportedErr";
        case kVTInsufficientSourceColorDataErr: return "kVTInsufficientSourceColorDataErr";
        case kVTCouldNotCreateColorCorrectionDataErr: return "kVTCouldNotCreateColorCorrectionDataErr";
        case kVTColorSyncTransformConvertFailedErr: return "kVTColorSyncTransformConvertFailedErr";
        case kVTVideoDecoderAuthorizationErr: return "kVTVideoDecoderAuthorizationErr";
        case kVTVideoEncoderAuthorizationErr: return "kVTVideoEncoderAuthorizationErr";
        case kVTColorCorrectionPixelTransferFailedErr: return "kVTColorCorrectionPixelTransferFailedErr";
        case kVTMultiPassStorageIdentifierMismatchErr: return "kVTMultiPassStorageIdentifierMismatchErr";
        case kVTMultiPassStorageInvalidErr: return "kVTMultiPassStorageInvalidErr";
        case kVTFrameSiloInvalidTimeStampErr: return "kVTFrameSiloInvalidTimeStampErr";
        case kVTFrameSiloInvalidTimeRangeErr: return "kVTFrameSiloInvalidTimeRangeErr";
        case kVTCouldNotFindTemporalFilterErr: return "kVTCouldNotFindTemporalFilterErr";
        case kVTPixelTransferNotPermittedErr: return "kVTPixelTransferNotPermittedErr";
        case kVTColorCorrectionImageRotationFailedErr: return "kVTColorCorrectionImageRotationFailedErr";
        case kVTVideoDecoderRemovedErr: return "kVTVideoDecoderRemovedErr";
        case kVTSessionMalfunctionErr: return "kVTSessionMalfunctionErr";
        case kVTVideoDecoderNeedsRosettaErr: return "kVTVideoDecoderNeedsRosettaErr";
        case kVTVideoEncoderNeedsRosettaErr: return "kVTVideoEncoderNeedsRosettaErr";
        case kVTVideoDecoderReferenceMissingErr: return "kVTVideoDecoderReferenceMissingErr";
        case kVTVideoDecoderCallbackMessagingErr: return "kVTVideoDecoderCallbackMessagingErr";
        default: break;
    }
    return "";
}

#define DEBUG_CM(function, status) DEBUG( \
        "%s: %i [%s]", \
        function, \
        (int) status, \
        _cm_status_string(status) \
    )

#define DEBUG_CV(function, status) DEBUG( \
        "%s: %i [%s]", \
        function, \
        (int) status, \
        _cv_status_string(status) \
    )

#define DEBUG_VIMAGE(function, error) DEBUG( \
        "%s: %li [%s]", \
        function, \
        error, \
        _vimage_errors(error) \
    )

#define DEBUG_VT(function, status) DEBUG( \
        "%s: %i [%s]", \
        function, \
        (int) status, \
        _vt_status_string(status) \
    )

void al_camera_cleanup(void);
