
// Derived from arch/defines.h which originally bore this license:
//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef LANDRU_DEFINES_H
#define LANDRU_DEFINES_H

//
// OS
//

#if defined(__linux__)
#  define LANDRU_OS_LINUX
#elif defined(__APPLE__)
#  include "TargetConditionals.h"
#  define LANDRU_OS_DARWIN
#  if TARGET_OS_IPHONE
#    define LANDRU_OS_IOS
#  else
#    define LANDRU_OS_OSX
#  endif
#elif defined(_WIN32) || defined(_WIN64)
#  define LANDRU_OS_WINDOWS
#endif

//
// Processor
//

#if defined(i386) || defined(__i386__) || defined(__x86_64__) || \
    defined(_M_IX86) || defined(_M_X64)
#  define LANDRU_CPU_INTEL
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM)
#  define LANDRU_CPU_ARM
#endif

//
// Bits
//

#if defined(__x86_64__) || defined(__aarch64__) || defined(_M_X64)
#  define LANDRU_BITS_64
#else
#  error "Unsupported architecture.  x86_64 or ARM64 required."
#endif

//
// Compiler
//

#if defined(__clang__)
#  define LANDRU_COMPILER_CLANG
#  define LANDRU_COMPILER_CLANG_MAJOR __clang_major__
#  define LANDRU_COMPILER_CLANG_MINOR __clang_minor__
#  define LANDRU_COMPILER_CLANG_PATCHLEVEL __clang_patchlevel__
#elif defined(__GNUC__)
#  define LANDRU_COMPILER_GCC
#  define LANDRU_COMPILER_GCC_MAJOR __GNUC__
#  define LANDRU_COMPILER_GCC_MINOR __GNUC_MINOR__
#  define LANDRU_COMPILER_GCC_PATCHLEVEL __GNUC_PATCHLEVEL__
#elif defined(__ICC)
#  define LANDRU_COMPILER_ICC
#elif defined(_MSC_VER)
#  define LANDRU_COMPILER_MSVC
#  define LANDRU_COMPILER_MSVC_VERSION	_MSC_VER
#endif

#endif // LANDRU_DEFINES_H
