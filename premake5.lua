------------------------------------------------------------------------------
-- Utils
------------------------------------------------------------------------------
function local_require(path)
	return dofile(path)
end

function this_directory()
    local str = debug.getinfo(2, "S").source:sub(2)
	local path = str:match("(.*/)")
    return path:gsub("\\", "/") -- Replace \\ with /
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
-- Solution
------------------------------------------------------------------------------
local MacOSVersion = "14.5"
local OutputDir = "%{cfg.buildcfg}-%{cfg.system}"

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