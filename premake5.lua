workspace "Landru"

configurations { "Debug", "Release" }
architecture "x86_64"

--objdir ("../build/obj/%{cfg.longname}/%{prj.name}")

platforms {
        "linux",
        "macosx",
        "windows"
    }

filter "system:linux"
    system "linux"
    defines { "PLATFORM_LINUX" }
    buildoptions { "-std=c++11" }

filter "system:windows"
    system "windows"
    defines {  "PLATFORM_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }
    characterset ("MBCS")

filter "system:macosx"
    system "macosx"
    defines {  "PLATFORM_DARWIN" }

filter {}

filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

filter {}

project "Landru"
    kind "StaticLib"
    language "C++"

    -- targetdir ("local/lib/%{cfg.longname}")

    includedirs { "src",
                   "../LabText/src", "../LabJson/src" }
    files { "src/**.h", "src/**.cpp", "src/**.c" }
    excludes { }

project "landruc"
    kind "ConsoleApp"
    language "C++"

    includedirs { "include", "src" }

    files { "include/Landru/**.h", "src/LandruC/**.h", "src/LandruC/**.cpp" }

    libdirs {
        "thirdparty/src/LabJson/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/src/LabText/bin/%{cfg.platform}/%{cfg.buildcfg}"
    }

    links { "Landru", "LabJson", "LabText" }

project "landru_gl"
    kind "SharedLib"
    language "C++"
    includedirs { "extras/LandruGL", "include", "src", "thirdparty/include" }
    files { "extras/LandruGL/**.h", "extras/LandruGL/**.cpp" }
    libdirs {
        "thirdparty/src/LabJson/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/src/LabText/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/lib"
    }
    links { "glfw3", "Landru", "LabText" }

project "landru_audio"
    kind "SharedLib"
    language "C++"
    includedirs { "extras/LandruAudio", "include", "src", "thirdparty/include" }
    files { "extras/LandruAudio/**.h", "extras/LandruAudio/**.cpp" }
    libdirs {
        "thirdparty/src/LabSound/build/x64/%{cfg.buildcfg}",
        "thirdparty/lib"
    }
    links { "glfw3", "Landru", "LabText" }
