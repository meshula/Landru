
cmake_minimum_required (VERSION 3.4)
project(Landru)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(LANDRU_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH 
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(Utilities)
include(Settings)
find_package(LabText)

add_subdirectory(third-party/entt)

lab_library(LandruCore
    TYPE STATIC
    ALIAS Landru::Core

    PUBLIC_HEADERS
        include/Landru/defines.h
        include/Landru/export.h
        include/Landru/Landru.h

    INCLUDE_DIRS
        ${LANDRU_ROOT}/include
        src

    PRIVATE_HEADERS
        src/LandruLang/LandruLexParse.h
        src/LandruLang/ParseExpression.h

    CPPFILES
        src/LandruLang/LandruAstPrint.cpp
        src/LandruLang/LandruLexParse.cpp
        src/LandruLang/ParseExpression.cpp
        src/LandruVMVI/VirtualMachineCompile.cpp
        src/LandruVMVI/VirtualMachineLoad.cpp
        src/LandruVMVI/VirtualMachinePrint.cpp
        src/LandruVMVI/VirtualMachineRuntime.cpp

    LIBRARIES
        Lab::Text
)

target_link_libraries(LandruCore EnTT)


add_executable(landruc
    src/LandruC/landruc.cpp
    src/LandruC/OptionParser.h
    src/LandruC/OptionParser.cpp)
target_link_libraries(landruc Landru::Core)
target_include_directories(landruc PRIVATE "${LANDRU_ROOT}/include")


add_executable(landru-test
    src/tests/main.cpp)
target_link_libraries(landru-test Landru::Core)
target_include_directories(landruc PRIVATE "${LANDRU_ROOT}/include")

install (TARGETS landruc
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

set_property(TARGET landruc PROPERTY FOLDER "apps")
set_property(TARGET landru-test PROPERTY FOLDER "tests")
