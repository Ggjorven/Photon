MacOSVersion = MacOSVersion or "14.5"

project "protobuf"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	-- staticruntime "Off"
	warnings "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
        "protobuf/src/google/protobuf/**.h",
		"protobuf/src/google/protobuf/**.cc",

		-- "protobuf/third_party/utf8_range/**.h",
		-- "protobuf/third_party/utf8_range/**.c",
		-- "protobuf/third_party/utf8_range/**.cc",
		-- "protobuf/third_party/utf8_range/**.cpp",

		"protobuf/third_party/utf8_range/utf8_range.h",
		"protobuf/third_party/utf8_range/utf8_range.c",
	}

	removefiles
	{
		"protobuf/src/google/protobuf/compiler/**",
		"protobuf/src/google/protobuf/testing/**",
		"protobuf/src/google/protobuf/util/python/**",

		"protobuf/src/**/**test**.*",
		"protobuf/src/**/**benchmark**.*",
		"protobuf/src/**/mock_code_generator.cc",

		-- "protobuf/third_party/utf8_range/**/**main**.*",
		-- "protobuf/third_party/utf8_range/**/**test**.*",
	}

	includedirs
    {
		"protobuf/src",
		"protobuf/src/google",

		"protobuf/third_party/utf8_range",

		"%{Dependencies.Abseil.IncludeDir}"
    }

	links
	{
		"%{Dependencies.Abseil.LibName}"
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