Dependencies = local_require("../Dependencies.lua")
MacOSVersion = MacOSVersion or "14.5"

project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	debugdir ("%{prj.location}")

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.inl",
		"src/**.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",
	}

	includedirs
	{
		"src",

		"%{wks.location}/NanoNetworking/src",
	}
	includedirs(Dependencies.Combined.IncludeDirs)

	links
	{
		"NanoNetworking",
	}

	filter "system:windows"
		defines "NN_PLATFORM_DESKTOP"
		defines "NN_PLATFORM_WINDOWS"
		systemversion "latest"
		staticruntime "on"

        defines
        {
            "NOMINMAX"
        }

		postbuildcommands
		{
			'{COPYFILE} "%{Dependencies.OpenSSL.IncludeDir}/../bin/%{Dependencies.OpenSSL.DllName}" "%{cfg.targetdir}"',
			'{COPYFILE} "%{Dependencies.OpenSSL.IncludeDir}/../bin/%{Dependencies.OpenSSL.DllName}" "%{prj.location}"' -- Note: This is the debugdir
		}

	filter "system:linux"
		defines "NN_PLATFORM_DESKTOP"
		defines "NN_PLATFORM_LINUX"
		defines "NN_PLATFORM_UNIX"
		systemversion "latest"
		staticruntime "on"

		links
		{
			"%{Dependencies.GameNetworkingSockets.LibName}",
			"%{Dependencies.ProtoBuf.LibName}",
			"%{Dependencies.Abseil.LibName}",

			"ssl", "crypto"
		}

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
