Dependencies = local_require("../../Dependencies.lua")
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

		"protobuf/src/**/**main**.*",
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
		-- "protobuf/src/google/protobuf/compiler/main.cc",
		-- "protobuf/src/google/protobuf/compiler/code_generator.cc",
		-- "protobuf/src/google/protobuf/compiler/code_generator_lite.cc",
		-- "protobuf/src/google/protobuf/compiler/command_line_interface.cc",
		
		"protobuf/src/google/protobuf/compiler/*.*",
		"protobuf/src/google/protobuf/compiler/cpp/**",

		-- "protobuf/src/google/protobuf/compiler/subprocess.cc",
		-- "protobuf/src/google/protobuf/compiler/zip_writer.cc",
		-- "protobuf/upb_generator/common.cc",
		-- "protobuf/upb_generator/common/names.cc",
		-- "protobuf/upb_generator/file_layout.cc",
		-- "protobuf/upb_generator/minitable/names.cc",
		-- "protobuf/upb_generator/minitable/names_internal.cc",
		-- "protobuf/upb_generator/plugin.cc",

		"protoc-main.cpp",
	}

	removefiles
	{
		"protobuf/src/**/**main**.*",
		"protobuf/src/**/**test**.*",
		"protobuf/src/**/**benchmark**.*",

		"protobuf/src/google/protobuf/compiler/fake_plugin.cc",
		"protobuf/src/google/protobuf/compiler/mock_code_generator.cc",
		"protobuf/src/google/protobuf/compiler/cpp/tools/analyze_profile_proto.cc",
		"protobuf/src/google/protobuf/compiler/cpp/tools/analyze_profile_proto_main.cc",
	}

	includedirs
    {
		"protobuf",
		"protobuf/src",
		"protobuf/src/google",

		"protobuf/upb_generator",
		"protobuf/third_party/utf8_range",

		"%{Dependencies.Abseil.IncludeDir}"
    }

	links
	{
		"%{Dependencies.ProtoBuf.LibName}", -- Includes Abseil
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",

		"UPB_BOOTSTRAP_STAGE=0"
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

		links
		{
			"%{Dependencies.Abseil.LibName}"
		}

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