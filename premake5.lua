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
    buildoptions { "-std=c++11" }

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
                   "thirdparty/prereq/LabText/src", "thirdparty/prereq/LabJson/src" }
    files {
        "src/Landru/*.h", "src/Landru/*.c", "src/Landru/*.cpp",
        "src/LandruActorVM/*.h", "src/LandruActorVM/*.c", "src/LandruActorVM/*.cpp",
        "src/LandruActorVM/StdLib/*.h", "src/LandruActorVM/StdLib/*.c", "src/LandruActorVM/StdLib/*.cpp",
        "src/LandruAssembler/*.h", "src/LandruAssembler/*.c", "src/LandruAssembler/*.cpp",
        "src/LandruCompiler/*.h", "src/LandruCompiler/*.c", "src/LandruCompiler/*.cpp",
        "src/LandruStd/*.h", "src/LandruStd/*.c", "src/LandruStd/*.cpp",
        "src/LandruVM/*.h", "src/LandruVM/*.c", "src/LandruVM/*.cpp"
    }

    excludes { }

project "landruc"
    kind "ConsoleApp"
    language "C++"

    includedirs {
        "include", "src",
        "thirdparty/prereq/LabText/src"
    }

    files {
        "include/Landru/**.h", "src/LandruC/**.h", "src/LandruC/**.cpp"
    }

    libdirs {
        "thirdparty/prereq/LabJson/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/prereq/LabText/bin/%{cfg.platform}/%{cfg.buildcfg}"
    }

    links { "Landru", "LabJson", "LabText" }

project "landruIDE"
    kind "ConsoleApp"
    language "C++"

    includedirs {
        "thirdparty/include",
        "thirdparty/local/include",
        "thirdparty/prereq/LabText/src",
        "src/LandruIDE/interface/imgui",
        "src/LandruIDE",
        "include", "src"
    }

    files {
        "src/LandruIDE/*.h", "src/LandruIDE/*.cpp",
        "src/LandruIDE/events/*.hpp",
        "src/LandruIDE/interface/*.h", "src/LandruIDE/interface/*.cpp",
        "src/LandruIDE/interface/imgui/*.h", "src/LandruIDE/interface/imgui/*.cpp",
        "src/LandruIDE/src/*.h", "src/LandruIDE/src/*.cpp"
    }

    libdirs {
        "thirdparty/prereq/LabJson/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/prereq/LabText/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/lib/%{cfg.platform}/%{cfg.buildcfg}"
    }

    links { "Landru", "LabJson", "LabText", "LabRender", "LabCmd",
            "glfw3", "glew", "opengl32" }

project "landru_gl"
    kind "SharedLib"
    language "C++"
    includedirs { "extras/LandruGL", "include", "src", "thirdparty/include" }
    files { "extras/LandruGL/**.h", "extras/LandruGL/**.cpp" }
    libdirs {
        "thirdparty/prereq/LabJson/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/prereq/LabText/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/local/lib"
    }
    links { "glfw3", "Landru", "LabText" }

project "landru_audio"
    kind "SharedLib"
    language "C++"
    includedirs {
        "extras/LandruAudio", "include", "src", "thirdparty/include",
        "thirdparty/prereq/labsound-c"
    }
    files { "extras/LandruAudio/**.h", "extras/LandruAudio/**.cpp" }
    libdirs {
        "thirdparty/prereq/LabSound/build/x64/%{cfg.buildcfg}",
        "thirdparty/prereq/LabText/bin/%{cfg.platform}/%{cfg.buildcfg}",
        "thirdparty/lib",
        "thirdparty/prereq/labsound-c/bin/%{cfg.platform}/%{cfg.buildcfg}"
    }
    links { "Landru", "LabText", "labsoundc" }

project "landru_vr"
    kind "SharedLib"
    language "C++"
    includedirs { "extras/LandruVR", "include", "src",
                  "thirdparty/prereq/openvr/headers",
                  "thirdparty/local/include" }
    files { "extras/LandruVR/**.hpp", "extras/LandruVR/**.cpp" }
    libdirs {
        "thirdparty/prereq/openvr/lib/win64",
        "thirdparty/local/lib"
    }
    links {
        "Landru", "openvr_api", "glew", "opengl32"
    }
