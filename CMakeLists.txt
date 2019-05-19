
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
        src/LandruActorVM/ConcurrentQueue.h
        src/LandruActorVM/Exception.h
        src/LandruActorVM/Fiber.h
        src/LandruActorVM/FnContext.h
        src/LandruActorVM/Generator.h
        src/LandruActorVM/Library.h
        src/LandruActorVM/MachineDefinition.h
        src/LandruActorVM/Property.h
        src/LandruActorVM/State.h
        src/LandruActorVM/VMContext.h
        src/LandruActorVM/StdLib/FiberLib.h
        src/LandruActorVM/StdLib/IntLib.h
        src/LandruActorVM/StdLib/IoLib.h
        src/LandruActorVM/StdLib/RealLib.h
        src/LandruActorVM/StdLib/StdLib.h
        src/LandruActorVM/StdLib/StringLib.h
        src/LandruActorVM/StdLib/TimeLib.h
        src/LandruAssembler/AssemblerBase.h
        src/LandruAssembler/LandruActorAssembler.h
        src/LandruAssembler/LandruAssembler.h
        src/LandruCompiler/AST.h
        src/LandruCompiler/Exception.h
        src/LandruCompiler/lcRaiseError.h
        src/LandruCompiler/ParseExpression.h
        src/LandruCompiler/Parser.h
        src/LandruCompiler/TokenDefs.h
        src/LandruCompiler/Tokens.h
        src/LandruLang/LandruLexParse.h

    CPPFILES
        src/LandruCompiler/Parser.cpp
        src/LandruActorVM/Fiber.cpp
        src/LandruActorVM/FnContext.cpp
        src/LandruActorVM/Library.cpp
        src/LandruActorVM/MachineDefinition.cpp
        src/LandruActorVM/Property.cpp
        src/LandruActorVM/State.cpp
        src/LandruActorVM/VMContext.cpp
        src/LandruActorVM/StdLib/FiberLib.cpp
        src/LandruActorVM/StdLib/IntLib.cpp
        src/LandruActorVM/StdLib/IoLib.cpp
        src/LandruActorVM/StdLib/RealLib.cpp
        src/LandruActorVM/StdLib/StdLib.cpp
        src/LandruActorVM/StdLib/StringLib.cpp
        src/LandruActorVM/StdLib/TimeLib.cpp
        src/LandruAssembler/AssemblerBase.cpp
        src/LandruAssembler/LandruActorAssembler.cpp
        src/LandruAssembler/LandruAssembler.cpp
        src/LandruCompiler/AST.cpp
        src/LandruCompiler/lcRaiseError.cpp
        src/LandruCompiler/ParseExpression.cpp
        src/LandruCompiler/Parser.cpp
        src/LandruCompiler/Tokens.cpp
        src/LandruLang/LandruAstPrint.cpp
        src/LandruLang/LandruLexParse.cpp
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
