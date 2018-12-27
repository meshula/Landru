
#ifndef LANDRU_EXPORT_H
#define LANDRU_EXPORT_H

#include "Landru/defines.h"

#if defined(LANDRU_DYNAMIC)
#   if defined(LANDRU_OS_WINDOWS)
#      if defined(LANDRU_COMPILER_GCC) && LANDRU_COMPILER_GCC_MAJOR >= 4 || defined(LANDRU_COMPILER_CLANG)
#          define LANDRU_EXPORT __attribute__((dllexport))
#          define LANDRU_IMPORT __attribute__((dllimport))
#          define LANDRU_HIDDEN
#      else
#          define LANDRU_EXPORT __declspec(dllexport)
#          define LANDRU_IMPORT __declspec(dllimport)
#          define LANDRU_HIDDEN
#      endif
#   elif defined(LANDRU_COMPILER_GCC) && LANDRU_COMPILER_GCC_MAJOR >= 4 || defined(LANDRU_COMPILER_CLANG)
#      define LANDRU_EXPORT __attribute__((visibility("default")))
#      define LANDRU_IMPORT
#      define LANDRU_HIDDEN __attribute__((visibility("hidden")))
#   endif
#endif
#ifndef LANDRU_EXPORT
#   define LANDRU_EXPORT
#   define LANDRU_IMPORT
#   define LANDRU_HIDDEN
#endif

#endif // LANDRU_EXPORT_H
