------------------------------------------------------------------------------
-- Dependencies
------------------------------------------------------------------------------
Dependencies =
{
	Nano = 
	{
		IncludeDir = "%{wks.location}/vendor/Nano/Nano/Nano/include"
	},
	GameNetworkingSockets =
	{
		LibName = "GameNetworkingSockets",
		IncludeDirs = { 
			"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/include", 
			"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src", 
			"%{wks.location}/vendor/GameNetworkingSockets/GameNetworkingSockets/src/public", 
		},
	},

	OpenSSL = {}, -- Note: On linux/macos, these are systemwide includes
	ProtoBuf = 
	{
		IncludeDir = "%{wks.location}/vendor/protobuf/protobuf/src",
		LibName = "protobuf"
	},
	Abseil = 
	{
		IncludeDir = "%{wks.location}/vendor/abseil/abseil",
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
		IncludeDir = "%{wks.location}/vendor/OpenSSL/windows/include",
        LibDir = "%{wks.location}/vendor/OpenSSL/windows/lib",
		LibNames = {
			"libcrypto",
			"libssl"
		},
		DllName = "libcrypto-3-x64.dll",
	}
end
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Merge
------------------------------------------------------------------------------
Dependencies.Combined = 
{
    IncludeDirs = {},
    LibNames = {},
    LibDirs = {}
}

for name, dep in pairs(Dependencies) do
    if name ~= "Combined" then
        -- IncludeDirs
        if dep.IncludeDir then
            table.insert(Dependencies.Combined.IncludeDirs, dep.IncludeDir)
        end
        if dep.IncludeDirs then
            table.insert(Dependencies.Combined.IncludeDirs, dep.IncludeDirs)
        end
        
        -- LibNames
        if dep.LibName then
            table.insert(Dependencies.Combined.LibNames, dep.LibName)
        end
        if dep.LibNames then
            table.insert(Dependencies.Combined.LibNames, dep.LibNames)
        end

        -- LibDirs
        if dep.LibDir then
            table.insert(Dependencies.Combined.LibDirs, dep.LibDir)
        end
        if dep.LibDirs then
            table.insert(Dependencies.Combined.LibDirs, dep.LibDirs)
        end
    end
end
------------------------------------------------------------------------------

return Dependencies