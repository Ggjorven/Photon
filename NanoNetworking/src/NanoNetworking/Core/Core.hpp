#pragma once

#include <cstdint>

#define STEAMNETWORKINGSOCKETS_STATIC_LINK
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace Nano::Networking
{

	////////////////////////////////////////////////////////////////////////////////////
	// Core
	////////////////////////////////////////////////////////////////////////////////////
	using ClientID = uint32_t;

	enum class ConnectionStatus : uint8_t 
	{ 
		Disconnected = 0, 
		Connected, Connecting, 
		FailedToConnect
	};

	struct Error
	{
	public:
		std::string String;
	};

}