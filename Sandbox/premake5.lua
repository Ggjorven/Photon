local Dependencies = local_require("../Dependencies.lua")
local MacOSVersion = MacOSVersion or "14.5"
local OutputDir = OutputDir or "%{cfg.buildcfg}-%{cfg.system}"

project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	debugdir ("%{prj.location}")

	architecture "x86_64"

	warnings "Extra"

	targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

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

	includedirs(Dependencies.NanoNetworking.IncludeDir)

	links
	{
		"NanoNetworking",
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "on"

        defines
        {
            "NOMINMAX"
        }

		postbuildcommands(Dependencies.NanoNetworking.PostBuildCommands)

	filter "system:linux"
		systemversion "latest"
		staticruntime "on"

		-- Linux needs a backwards linking again for some reason
		links(remove_from_table(copy_table(Dependencies.NanoNetworking.LibName), "NanoNetworking"))

    filter "system:macosx"
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
