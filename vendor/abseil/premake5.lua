Dependencies = local_require("../../Dependencies.lua")
MacOSVersion = MacOSVersion or "14.5"

project "abseil"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	-- staticruntime "Off"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"abseil/absl/**.h",
		"abseil/absl/**.cc",
	}

	removefiles
	{
		"abseil/absl/**/**test**.h",
		"abseil/absl/**/**test**.cc",

		"abseil/absl/**/**benchmark**.h",
		"abseil/absl/**/**benchmark**.cc",

		-- "abseil/absl/base/internal/**",
		-- "abseil/absl/base/internal/**",

		"abseil/absl/**/scoped_mock_log.cc",
		"abseil/absl/**/status_matchers.cc",
		"abseil/absl/**/gaussian_distribution_gentables.cc",
		"abseil/absl/**/print_hash_of.cc",
	}

	includedirs
    {
		"abseil",
		"abseil/absl",
    }

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		defines 
		{
			"NOMINMAX",
		}
		
	filter "system:linux"
		systemversion "latest"
		staticruntime "On"

	filter "system:macosx"
		systemversion(MacOSVersion)
		staticruntime "On"

	filter "action:xcode*"
		-- Note: If we don't add the header files to the externalincludedirs
		-- we can't use <angled> brackets to include files.
		externalincludedirs(includedirs())

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		runtime "Release"
		optimize "Full"
		linktimeoptimization "On"