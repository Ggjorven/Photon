#include "phpch.h"
#include "Utils.hpp"

#include "Photon/Core/Logging.hpp"

#define NANO_IMPLEMENTATION
#include <Nano/Nano.hpp>

#include <format>

#if defined(NANO_PLATFORM_WINDOWS)
	#include <WinSock2.h>
	#include <ws2tcpip.h>
#elif defined(NANO_PLATFORM_POSIX)
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else
	#error Photon Utils: Utilities include not defined for this platform.
#endif

namespace Photon::Utils
{

	////////////////////////////////////////////////////////////////////////////////////
	// Utils
	////////////////////////////////////////////////////////////////////////////////////
	bool IsValidIPAddress(std::string_view ipAddress)
	{
		std::string ipAddressStr(ipAddress.data(), ipAddress.size());

		SteamNetworkingIPAddr address = {};
		return address.ParseString(ipAddressStr.c_str());
	}

	// Platform-specific implementations
	std::expected<std::string, Error> ResolveDomainName(std::string_view name)
	{
		std::string domain = std::string(name);
		std::string port;

		// Has a port in format: google.com:8000
		if (name.contains(":"))
		{
			std::vector<std::string> domainAndPort = Nano::Text::SplitString(name, ':');

			if (domainAndPort.size() != 2)
				return std::unexpected(Error("Domain contained multiple ports."));

			domain = domainAndPort[0];
			port = domainAndPort[1];
		}

#if defined(NANO_PLATFORM_WINDOWS)
		// Adapted from example at https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo
		WSADATA wsaData;
		int wsaFailed = WSAStartup(MAKEWORD(2, 2), &wsaData);
		// FUTURE: std::scope_exit WSACleanup
		if (wsaFailed)
			return std::unexpected(Error(std::format("WSAStartup failed with error code: {}", WSAGetLastError())));

		addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* addressResult = NULL;
		DWORD getFailed = getaddrinfo(domain.c_str(), nullptr, &hints, &addressResult);
		if (getFailed)
		{
			WSACleanup();
			return std::unexpected(Error(std::format("getaddrinfo failed with error: {}", getFailed)));
		}

		std::string ipAddressStr;
		for (addrinfo* ptr = addressResult; ptr != NULL; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
			case AF_UNSPEC:
				break;

			case AF_INET:
			{
				sockaddr_in* sockaddr_ipv4 = (sockaddr_in*)ptr->ai_addr;
				char* ipAddress = inet_ntoa(sockaddr_ipv4->sin_addr);
				ipAddressStr = ipAddress;
				break;
			}
			case AF_INET6:
			{
				const DWORD ipbufferlength = 46;
				char ipstringbuffer[ipbufferlength];
				DWORD actualIPBufferLength = ipbufferlength;
				LPSOCKADDR sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
				INT strFailed = WSAAddressToStringA(sockaddr_ip, (DWORD)ptr->ai_addrlen, nullptr, ipstringbuffer, &actualIPBufferLength);

				if (strFailed)
				{
					int errorCode = WSAGetLastError();
					WSACleanup();
					return std::unexpected(Error(std::format("WSAAddressToString failed with error: {}", errorCode)));
				}

				ipAddressStr = std::string(ipstringbuffer, actualIPBufferLength);
				break;
			}

			default:
				break;
			}
		}

		freeaddrinfo(addressResult);
		WSACleanup();
#elif defined(NANO_PLATFORM_POSIX)
		addrinfo hints = {};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* res = nullptr;
		int err = getaddrinfo(name.data(), nullptr, &hints, &res);
		if (err != 0)
			return std::unexpected(Error(std::format("getaddrinfo failed with error: {}", gai_strerror(err))));

		std::string ipAddressStr;
		for (addrinfo* ptr = res; ptr != nullptr; ptr = ptr->ai_next)
		{
			char ipstr[INET6_ADDRSTRLEN] = {};

			if (ptr->ai_family == AF_INET)
			{
				sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
				inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
				ipAddressStr = ipstr;
				break; // Prefer IPv4
			}
			else if (ptr->ai_family == AF_INET6)
			{
				sockaddr_in6* ipv6 = reinterpret_cast<sockaddr_in6*>(ptr->ai_addr);
				inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipstr, sizeof(ipstr));
				ipAddressStr = ipstr;
				// Don't break, prefer IPv4 if available, but fallback to IPv6
			}
		}

		freeaddrinfo(res);
#endif

		return (port.empty() ? ipAddressStr : (ipAddressStr + ":" + port));
	}

}