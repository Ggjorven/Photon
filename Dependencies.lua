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

function append_to_table(dest, value)
	if type(value) == "table" then
		for _, v in ipairs(value) do
        	table.insert(dest, v)
    	end
    else
		table.insert(dest, value)
    end

	return dest
end

function remove_from_table(dest, filter)
    for i = #dest, 1, -1 do  -- Iterate backwards
        local value = dest[i]

		-- Note: Allows lua patterns
        if string.find(value, filter) ~= nil then
            table.remove(dest, i)
        end
    end

	return dest
end

function copy_table(tbl)
    if type(tbl) ~= "table" then 
		return tbl 
	end

    local copy = {}
    for k, v in pairs(tbl) do
        copy[k] = copy_table(v)
    end
	
    return copy
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
local Dependencies =
{
	-- Internal Dependencies
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

	OpenSSL = 
	{
		-- Note: On linux/macos, these are systemwide includes
		IncludeDir = {},
		LibName = {},
	}, 
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

	-- Export Dependencies (Note: Makes using as submodule easier.)
	Photon = 
	{
		IncludeDir = {},
		LibName = {},
		PostBuildCommands = {},
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
		PostBuildCommands = {},
	}

	Dependencies.OpenSSL.PostBuildCommands = 
	{
		'{COPYFILE} "' .. Dependencies.OpenSSL.IncludeDir .. '/../bin/libcrypto-3-x64.dll" "%{cfg.targetdir}"',
		'{COPYFILE} "' .. Dependencies.OpenSSL.IncludeDir .. '/../bin/libcrypto-3-x64.dll" "%{prj.location}"' -- Note: This is the debugdir most of the time
	}
elseif os.target() == "linux" or os.target() == "macosx" then
	Dependencies.OpenSSL = 
	{
		LibName = 
		{
			"ssl",
			"crypro"
		},
	}
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Export Dependencies
------------------------------------------------------------------------------
-- IncludeDirs
append_to_table(Dependencies.Photon.IncludeDir, this_directory() .. "Photon/src/")
append_to_table(Dependencies.Photon.IncludeDir, Dependencies.ProtoBuf.IncludeDir)
append_to_table(Dependencies.Photon.IncludeDir, Dependencies.Abseil.IncludeDir)
append_to_table(Dependencies.Photon.IncludeDir, Dependencies.GameNetworkingSockets.IncludeDir)
append_to_table(Dependencies.Photon.IncludeDir, Dependencies.Nano.IncludeDir)
append_to_table(Dependencies.Photon.IncludeDir, Dependencies.OpenSSL.IncludeDir)

-- LibNames
append_to_table(Dependencies.Photon.LibName, "Photon")

if os.target() == "linux" then
	append_to_table(Dependencies.Photon.LibName, Dependencies.GameNetworkingSockets.LibName)
	append_to_table(Dependencies.Photon.LibName, Dependencies.ProtoBuf.LibName)
	append_to_table(Dependencies.Photon.LibName, Dependencies.Abseil.LibName)
	append_to_table(Dependencies.Photon.LibName, Dependencies.OpenSSL.LibName)
end

-- PostBuildCommands
append_to_table(Dependencies.Photon.PostBuildCommands, Dependencies.OpenSSL.PostBuildCommands)
------------------------------------------------------------------------------

return Dependencies