------------------------------------------------------------------------------
-- Utilities
------------------------------------------------------------------------------
local function GetIOResult(cmd)
	local handle = io.popen(cmd) -- Open a console and execute the command.
	local output = handle:read("*a") -- Read the output.
	handle:close() -- Close the handle.

	return output:match("^%s*(.-)%s*$") -- Trim any trailing whitespace (such as newlines)
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Bug fixes
------------------------------------------------------------------------------
-- Visual Studio: Bugfix for C++ Modules (same module file name per project)
-- https://github.com/premake/premake-core/issues/2177
require("vstudio")
premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, prj)
    local m = premake.vstudio.vc2010
    local calls = base(prj)

    if premake.project.iscpp(prj) then
		table.insertafter(calls, premake.xmlDeclaration,  function()
			premake.w('<ModuleDependenciesFile>$(IntDir)\\%%(RelativeDir)</ModuleDependenciesFile>')
			premake.w('<ModuleOutputFile>$(IntDir)\\%%(RelativeDir)</ModuleOutputFile>')
			premake.w('<ObjectFileName>$(IntDir)\\%%(RelativeDir)</ObjectFileName>')
		end)
    end

    return calls
end)
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
MacOSVersion = "14.5"

Dependencies =
{
	GameNetworkingSockets =
	{
		LibName = "GameNetworkingSockets",
		IncludeDirs = { 
			"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/include", 
			"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src", 
			"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/public", 
		},
	},

	OpenSSL = {},
	ProtoBuf = 
	{
		IncludeDir = "%{wks.location}/vendor/protobuf/protobuf/src/google",
		LibName = "protobuf"
	},
	Abseil = 
	{
		IncludeDir = "%{wks.location}/vendor/abseil/abseil",
		LibName = "abseil"
	},
}

function nn_include_dependencies()
	for name, dep in pairs(Dependencies) do
		if dep.IncludeDir then
			includedirs { dep.IncludeDir }
		end
		if dep.IncludeDirs then
			includedirs { dep.IncludeDirs }
		end
	end
end

function nn_link_dependencies()
	for name, dep in pairs(Dependencies) do
		if dep.LibDir then
			libdirs { dep.LibDir }
		end
		if dep.LibDirs then
			libdirs { dep.LibDirs }
		end
		if dep.LibName then
			links { dep.LibName }
		end
		if dep.LibNames then
			links { dep.LibNames }
		end
	end
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Platform specific
------------------------------------------------------------------------------
if os.target() == "windows" then
	Dependencies.OpenSSL = 
	{
		IncludeDir = "%{wks.location}/vendor/OpenSSL/windows/include",
		-- TODO: Linking
	}
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Solution
------------------------------------------------------------------------------
outputdir = "%{cfg.buildcfg}-%{cfg.system}"

workspace "NanoNetworking"
	architecture "x86_64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

group "Dependencies"
	include "vendor/GameNetworkingSockets"
	include "vendor/protobuf"
	include "vendor/abseil"
group ""

group "NanoNetworking"
	include "NanoNetworking"
group ""

include "Sandbox"
------------------------------------------------------------------------------