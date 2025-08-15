------------------------------------------------------------------------------
-- Utils
------------------------------------------------------------------------------
local function local_require(path)
	return dofile(path)
end

local function this_directory()
    local str = debug.getinfo(2, "S").source:sub(2)
	local path = str:match("(.*/)")
    return path:gsub("\\", "/") -- Replace \\ with /
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
local Dependencies =
{
	Nano = 
	{
		IncludeDir = this_directory() .. "/vendor/Nano/Nano/Nano/include"
	},
	GameNetworkingSockets =
	{
		LibName = "GameNetworkingSockets",
		IncludeDir = 
		{ 
			this_directory() .. "/vendor/GameNetworkingSockets/GameNetworkingSockets/include", 
			this_directory() .. "/vendor/GameNetworkingSockets/GameNetworkingSockets/src", 
			this_directory() .. "/vendor/GameNetworkingSockets/GameNetworkingSockets/src/public", 
		},
	},

	OpenSSL = {}, -- Note: On linux/macos, these are systemwide includes
	ProtoBuf = 
	{
		IncludeDir = this_directory() .. "/vendor/protobuf/protobuf/src",
		LibName = "protobuf"
	},
	Abseil = 
	{
		IncludeDir = this_directory() .. "/vendor/abseil/abseil",
		LibName = "abseil"
	},
}
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Platform specific
------------------------------------------------------------------------------
if os.target() == "windows" then
	Dependencies.OpenSSL = 
	{
		IncludeDir = this_directory() .. "/vendor/OpenSSL/windows/include",
        LibDir = this_directory() .. "/vendor/OpenSSL/windows/lib",
		LibName = 
		{
			"libcrypto",
			"libssl",

			"ws2_32.lib"
		},
		DllName = "libcrypto-3-x64.dll",
		PostBuildCommands = {},
	}

	Dependencies.OpenSSL.PostBuildCommands = 
	{
		'{COPYFILE} "' .. Dependencies.OpenSSL.IncludeDir .. '/../bin/' .. Dependencies.OpenSSL.DllName .. '" "%{cfg.targetdir}"',
		'{COPYFILE} "' .. Dependencies.OpenSSL.IncludeDir .. '/../bin/' .. Dependencies.OpenSSL.DllName .. '" "%{prj.location}"' -- Note: This is the debugdir most of the time
	}
elseif os.target() == "linux" then
	Dependencies.OpenSSL = 
	{
		LibName = 
		{
			"ssl",
			"crypro"
		},
	}
else
	error("TODO: Other platforms")
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Merge
------------------------------------------------------------------------------
Dependencies.Combined =
{
    IncludeDir = {},
    LibName = {},
    LibDir = {}
}

for name, dep in pairs(Dependencies) do
    if name ~= "Combined" then
        -- IncludeDirs
        if dep.IncludeDir then
            table.insert(Dependencies.Combined.IncludeDir, dep.IncludeDir)
        end
        
        -- LibNames
        if dep.LibName then
            table.insert(Dependencies.Combined.LibName, dep.LibName)
        end

        -- LibDirs
        if dep.LibDir then
            table.insert(Dependencies.Combined.LibDir, dep.LibDir)
        end
    end
end
------------------------------------------------------------------------------

return Dependencies