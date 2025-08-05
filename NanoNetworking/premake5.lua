MacOSVersion = MacOSVersion or "14.5"

project "NanoNetworking"
	kind "StaticLib"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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

	nn_include_dependencies()

	links
	{
		"%{Dependencies.GameNetworkingSockets.LibName}"
	}

	filter "system:windows"
		defines "NN_PLATFORM_DESKTOP"
		defines "NN_PLATFORM_WINDOWS"
		defines "NN_PLATFORM_UNIX"
		systemversion "latest"
		staticruntime "on"

        defines
        {
            "NOMINMAX"
        }

		links
		{
			"ws2_32.lib"
		}

		libdirs("%{Dependencies.OpenSSL.LibDir}")
		links(Dependencies.OpenSSL.LibNames)

	filter "system:linux"
		defines "NN_PLATFORM_DESKTOP"
		defines "NN_PLATFORM_LINUX"
		systemversion "latest"
		staticruntime "on"

    filter "system:macosx"
		defines "NN_PLATFORM_DESKTOP"
		defines "NN_PLATFORM_MACOS"
		defines "NN_PLATFORM_UNIX"
		defines "NN_PLATFORM_APPLE"
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
		defines "NN_CONFIG_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "NN_CONFIG_DIST"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "on"
