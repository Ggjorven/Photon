#pragma once

#define STEAMNETWORKINGSOCKETS_STATIC_LINK
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <Nano/Nano.hpp>

#include <cstdint>
#include <string>

namespace Nano::Networking
{

	////////////////////////////////////////////////////////////////////////////////////
	// Core
	////////////////////////////////////////////////////////////////////////////////////
	using ClientID = uint32_t;
	using Buffer = Nano::Memory::Buffer;

	enum class ConnectionStatus : uint8_t 
	{ 
		Disconnected = 0,
		Connected, Connecting, 
		FailedToConnect, FailedToInitialize,
		InvalidIP
	};

	struct Error
	{
	public:
		std::string String;
	};

}