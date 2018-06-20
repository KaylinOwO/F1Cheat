
require("premake_modules/export-compile-commands")

-- premake5.lua
workspace "F1Cheat"
    configurations { "Debug", "Release" }
    platforms { "x32" }

    location "premake"
    
    filter "system:windows"
        characterset "MBCS"
        toolset "msc-v141"
        buildoptions{ "--driver-mode=cl -Bv" }

    filter {}

    filter "platforms:x32"
        architecture "x32"

    filter "configurations:Debug"
        cppdialect "C++17"
    
        defines { "DEBUG" }
        symbols "On"
        vectorextensions "SSE2"
        ignoredefaultlibraries {"libcpmt"}

    filter "configurations:Release"
        cppdialect "C++17"
    
        defines { "NDEBUG" }
        optimize "Full"
        floatingpoint "Fast"
        vectorextensions "SSE2"
        flags {"LinkTimeOptimization", "NoBufferSecurityCheck", "MultiProcessorCompile" }

    filter "files:SDK/sourcesdk/**"
        flags { "NoPCH" }
        filter {}

    project "F1Cheat"
        kind "SharedLib"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"

        filter "system:linux"
            pchheader "F1/stdafx.hh"
        filter "system:windows"
            pchheader "stdafx.hh"
        filter {}

        pchsource "F1/stdafx.cc"

        libdirs {"lib/%{cfg.buildcfg}", "SDK/sourcesdk/lib/public", "SDK/"}

		links {"F1-SDK", "tier0", "tier1", "tier2", "tier3",
                "vstdlib", "vgui_controls", "steamclient",
                "legacy_stdio_definitions", "mathlib", "steam_api"}

        prebuildcommands {"{DELETE} %{cfg.objdir}/Panels.obj"}

        files { "F1/*.hh", "F1/*.cc", "SDK/sourcesdk/tier1/convar.cpp", "SDK/sourcesdk/public/tier0/memoverride.cpp" }

        includedirs {"SDK", "SDK/steam", "SDK/sourcesdk/public", "SDK/sourcesdk/shared",
                    "SDK/sourcesdk/public/vgui", "SDK/sourcesdk/public/vgui_controls",
                    "SDK/sourcesdk/game", "SDK/sourcesdk/public/tier0", "SDK/sourcesdk/public/tier1"}

        defines {"WIN32", "_WINDOWS"}

    
    project "F1-SDK"
        filter "system:windows"
            toolset "msc-v141"
        filter {}

        kind "StaticLib"
        language "C++"
        targetdir "lib/%{cfg.buildcfg}"

        filter "system:linux"
            pchheader "SDK/stdafx.hh"
        filter "system:windows"
            pchheader "baseHeaders.hh"
        filter {}
            
        pchsource "SDK/stdafx.cc"
        
        files { "SDK/*.hh", "SDK/*.cc", "SDK/sourcesdk/public/bone_setup.cpp", "SDK/sourcesdk/public/collisionutils.cpp",
                "SDK/sourcesdk/public/studio.cpp" }

        includedirs {"SDK", "SDK/steam", "SDK/sourcesdk/public", "SDK/sourcesdk/shared",
                    "SDK/sourcesdk/public/vgui", "SDK/sourcesdk/public/vgui_controls",
                    "SDK/sourcesdk/game", "SDK/sourcesdk/public/tier0", "SDK/sourcesdk/public/tier1"}
                    
        defines {"WIN32", "_WINDOWS"}

	project "tier1"
		filter "system:windows"
			toolset "msc-v141"
		filter {}

		kind "StaticLib"
		language "C++"
		targetdir "lib/%{cfg.buildcfg}"

		files {"SDK/sourcesdk/tier1/*.cpp", "SDK/sourcesdk/public/tier1/*.h"}

		includedirs {"SDK/sourcesdk/common", "SDK/sourcesdk/public", "SDK/sourcesdk/public/tier1", "SDK/sourcesdk/public/tier0",}

		defines{"WIN32", "_WINDOWS", "TIER1_STATIC_LIB"}

		filter "files:**/processor_detect_linux.cpp"
			flags {"ExcludeFromBuild"}
		filter {}
