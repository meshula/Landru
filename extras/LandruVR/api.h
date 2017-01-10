//
// Copyright 2016 Nick Porcino
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
#ifndef LANDRUVR_API_H
#define LANDRUVR_API_H

#include "Landru/export.h"

#if defined(LANDRUVR_STATIC)
#   define LANDRUVR_API
#   define LANDRUVR_LOCAL
#else
#   if defined(LANDRUVR_EXPORTS)
#       define LANDRUVR_API ARCH_EXPORT
#       define LANDRUVR_API_TEMPLATE_CLASS(...)
#       define LANDRUVR_API_TEMPLATE_STRUCT(...)
#   else
#       define LANDRUVR_API ARCH_IMPORT
#       define LANDRUVR_API_TEMPLATE_CLASS(...) extern template class LANDRUVR_API __VA_ARGS__
#       define LANDRUVR_API_TEMPLATE_STRUCT(...) extern template struct LANDRUVR_API __VA_ARGS__
#   endif
#   define LANDRUVR_LOCAL ARCH_HIDDEN
#endif

#endif
