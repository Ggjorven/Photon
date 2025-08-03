MacOSVersion = MacOSVersion or "14.5"

project "protobuf"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	-- staticruntime "Off"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	prebuildcommands
	{
		-- Note: This is ugly, but GameNetworkingSockets includes abseil
		'{COPYFILE} "%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/base/internal/prefetch.h" "%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/base"'
	}

	files
	{
        "protobuf/src/google/protobuf/**.h",
		"protobuf/src/google/protobuf/**.cc",

		-- Note: This is ugly, but GameNetworkingSockets includes abseil
		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/**.h",
		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/**.cc",
	}

	removefiles
	{
		"protobuf/src/**/**test**.*",

		-- Note: This is ugly, but GameNetworkingSockets includes abseil
		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/**/**test**.h",
		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/**/**test**.cc",

		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/**/**benchmark**.h",
		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/**/**benchmark**.cc",

		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/base/internal/**",
		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/base/internal/**",

		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil/absl/**/scoped_mock_log.cc",
	}

	includedirs
    {
		"protobuf/src",
		"protobuf/src/google",

		-- Note: This is ugly, but GameNetworkingSockets includes abseil
		"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/external/abseil"
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

project "protoc"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	-- staticruntime "Off"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"GameNetworkingSockets/src/**.h",
		"GameNetworkingSockets/src/**.hpp",
		"GameNetworkingSockets/src/**.cpp",
	}

	includedirs
    {
		"GameNetworkingSockets/src",
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