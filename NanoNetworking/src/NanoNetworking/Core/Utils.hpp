#pragma once

#include "NanoNetworking/Core/Core.hpp"

#include <Nano/Nano.hpp>

#include <expected>

namespace Nano::Networking::Utils
{

	////////////////////////////////////////////////////////////////////////////////////
	// Utils
	////////////////////////////////////////////////////////////////////////////////////
	bool IsValidIPAddress(std::string_view ipAddress);
	std::expected<std::string, Error> ResolveDomainName(std::string_view name); // Note: Format 127.0.0.1 or 127.0.0.1:8000

}