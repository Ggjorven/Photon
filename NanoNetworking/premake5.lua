local Dependencies = local_require("../Dependencies.lua")
local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "NanoNetworking"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

	-- Note: VS2022/Make only need the pchheader filename
	pchheader "nnpch.h"
	pchsource "src/NanoNetworking/nnpch.cpp"

	files
	{
		"src/NanoNetworking/**.h",
		"src/NanoNetworking/**.hpp",
		"src/NanoNetworking/**.inl",
		"src/NanoNetworking/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",
		"_WINSOCK_DEPRECATED_NO_WARNINGS"
	}

	includedirs
	{
		"src",
		"src/NanoNetworking",
	}

	includedirs(Dependencies.ProtoBuf.IncludeDir)
	includedirs(Dependencies.Abseil.IncludeDir)
	includedirs(Dependencies.GameNetworkingSockets.IncludeDir)
	includedirs(Dependencies.Nano.IncludeDir)

	links(Dependencies.GameNetworkingSockets.LibName)

	filter "system:windows"
		systemversion "latest"
		staticruntime "on"

        defines
        {
            "NOMINMAX"
        }
		
		includedirs(Dependencies.OpenSSL.IncludeDir)
		libdirs(Dependencies.OpenSSL.LibDir)
		links(Dependencies.OpenSSL.LibName)

	filter "system:linux"
		systemversion "latest"
		staticruntime "on"

		links(Dependencies.ProtoBuf.LibName)
		links(Dependencies.Abseil.LibName)
		links(Dependencies.OpenSSL.LibName)

    filter "system:macosx"
		systemversion(MacOSVersion)
		staticruntime "on"

	filter "action:vs*"
		buildoptions { "/Zc:preprocessor" }

	filter "action:xcode*"
		-- Note: XCode only needs the full pchheader path
		pchheader "src/NanoNetworking/nnpch.h"

		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs(includedirs())

	filter "configurations:Debug"
		defines "NN_CONFIG_DEBUG"
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines "NDEBUG"
		defines "NN_CONFIG_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "NDEBUG"
		defines "NN_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "on"
