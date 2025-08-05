MacOSVersion = MacOSVersion or "14.5"

project "GameNetworkingSockets"
	dependson "protoc"
	kind "StaticLib"
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

	removefiles 
	{
		"GameNetworkingSockets/src/external/**",
	}

	removefiles -- Note: We use OpenSSL
	{
		-- LibSodium
		"GameNetworkingSockets/src/common/crypto_libsodium.cpp",
		"GameNetworkingSockets/src/common/crypto_25519_libsodium.cpp",

		"GameNetworkingSockets/src/common/crypto_sha1_wpa.cpp",
		"GameNetworkingSockets/src/external/sha1-wpa/sha1-internal.c",
		"GameNetworkingSockets/src/external/sha1-wpa/sha1.c",

		-- BCrypt
		"GameNetworkingSockets/src/common/crypto_bcrypt.cpp",

		"GameNetworkingSockets/src/common/crypto_25519_donna.cpp",
		"GameNetworkingSockets/src/external/curve25519-donna/curve25519.c",
		"GameNetworkingSockets/src/external/curve25519-donna/curve25519_VALVE_sse2.c",
		"GameNetworkingSockets/src/external/ed25519-donna/ed25519_VALVE.c",
		"GameNetworkingSockets/src/external/ed25519-donna/ed25519_VALVE_sse2.c"
	}

	includedirs
    {
		"GameNetworkingSockets/src",
		"GameNetworkingSockets/src/public",
		"GameNetworkingSockets/src/common",
		
        "GameNetworkingSockets/include",

		"GameNetworkingSockets/src/external",
		"GameNetworkingSockets/src/external/webrtc",
		"GameNetworkingSockets/src/external/picojson",

		"%{Dependencies.OpenSSL.IncludeDir}",
		"%{Dependencies.ProtoBuf.IncludeDir}",
		"%{Dependencies.ProtoBuf.IncludeDir}/../third_party/utf8_range",
		"%{Dependencies.Abseil.IncludeDir}",
    }

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",

		"VALVE_CRYPTO_ENABLE_25519",
		"STEAMNETWORKINGSOCKETS_STATIC_LINK",
		-- "STEAMNETWORKINGSOCKETS_LIBRARY"
	}

	links
	{
		"%{Dependencies.ProtoBuf.LibName}"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		includedirs("%{wks.location}/vendor/OpenSSL/windows/include")
		defines("WEBRTC_WIN")

		defines 
		{
			"NOMINMAX",
			"_WINDOWS"
		}
		
	filter "system:linux"
		systemversion "latest"
		staticruntime "On"

		defines("WEBRTC_POSIX")

	filter "system:macosx"
		systemversion(MacOSVersion)
		staticruntime "On"

		defines("WEBRTC_MAC")

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