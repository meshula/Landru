
project "Landru"
    kind "StaticLib"
    language "C++"
    platforms { "x32", "x64" }

    includedirs { "src", "../LabText/src", "../LabJson/src" }
    files { "src/**.h", "src/**.cpp", "src/**.c" }
    excludes { }

    configuration "Debug"
        targetdir "build/Debug"
        defines {  "DEBUG", "PLATFORM_DARWIN" }
        flags { "Symbols" }

    configuration "Release"
        targetdir "build/Release"
        defines { "NDEBUG", "PLATFORM_DARWIN" }
        flags { "Optimize" }

    configuration "macosx"
        buildoptions { "-std=c++11", "-stdlib=libc++" }
