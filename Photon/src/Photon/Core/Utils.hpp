#pragma once

#include "Photon/Core/Core.hpp"

#include <expected>

namespace Photon::Utils
{

	////////////////////////////////////////////////////////////////////////////////////
	// Utils
	////////////////////////////////////////////////////////////////////////////////////
	bool IsValidIPAddress(std::string_view ipAddress);
	std::expected<std::string, Error> ResolveDomainName(std::string_view name); // Note: Format google.com or google.com:8000

}