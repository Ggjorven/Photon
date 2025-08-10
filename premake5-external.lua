-- Note: This is the premake5 file made to allow for using
-- NanoNetworking as a submodule/dependency without the main workspace.

------------------------------------------------------------------------------
-- Utils
------------------------------------------------------------------------------
function local_require(path)
	return dofile(path)
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

group "Dependencies"
	include "vendor/GameNetworkingSockets"
	include "vendor/protobuf"
	include "vendor/abseil"

	include "NanoNetworking"
group ""
------------------------------------------------------------------------------