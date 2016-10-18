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
    defines { "PLATFORM_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }

filter "system:macosx"
    system "macosx"
    defines { "PLATFORM_DARWIN" }

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

    includedirs { "src", "../LabText/src", "../LabJson/src" }

    files { "src/LandruC/**.h", "src/LandruC/**.cpp" }

    libdirs { "../LabText/bin/%{cfg.buildcfg}",
              "../LabJson/bin/%{cfg.buildcfg}" }

    links { "Landru", "LabJson", "LabText" }
