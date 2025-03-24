Aluminium Library is a cross-platform computer vision library.

# Building

## macOS

    DEBUG=1 make -f darwin/Makefile so

The dynamic library will be found in the `build` directory:

    build/arm64-apple-darwin/libal.dylib
    build/x86_64-apple-darwin/libal.dylib
    build/arm64-apple-ios/libal.dylib
    build/armv7-apple-ios/libal.dylib

## Android

First, install Android Studio and the [androidenv][] library:

    pip install androidenv

then

    DEBUG=1 make -f android/Makefile so

The dynamic library will be found in the `build` directory:

    build/arm-linux-androideabi/libal.so
    build/aarch64-linux-android/libal.so


[androidenv]: https://github.com/mansourmoufid/python-androidenv
