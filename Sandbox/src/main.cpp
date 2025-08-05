#include <iostream>
#include <string>
#include <vector>

#define STEAMNETWORKINGSOCKETS_STATIC_LINK
#include <steam/isteamnetworkingutils.h>

#if defined(NN_PLATFORM_WINDOWS)
	#include <WinSock2.h>
	#include <ws2tcpip.h>
#else

#endif

namespace Nano::Networking::Utils 
{

	std::vector<std::string> SplitString(const std::string_view string, const std::string_view& delimiters)
	{
		size_t first = 0;

		std::vector<std::string> result;

		while (first <= string.size())
		{
			const auto second = string.find_first_of(delimiters, first);

			if (first != second)
				result.emplace_back(string.substr(first, second - first));

			if (second == std::string_view::npos)
				break;

			first = second + 1;
		}

		return result;
	}

	std::vector<std::string> SplitString(const std::string_view string, const char delimiter)
	{
		return SplitString(string, std::string(1, delimiter));
	}

	bool IsValidIPAddress(std::string_view ipAddress)
	{
		std::string ipAddressStr(ipAddress.data(), ipAddress.size());

		SteamNetworkingIPAddr address;
		return address.ParseString(ipAddressStr.c_str());
	}

	// Platform-specific implementations
	std::string ResolveDomainName(std::string_view name)
	{
#if defined(NN_PLATFORM_WINDOWS)
		// Adapted from example at https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			printf("WSAStartup failed with %u\n", WSAGetLastError());
			return {};
		}

		bool hasPort = name.find(":") != std::string::npos;
		std::string domain, port;
		if (hasPort)
		{
			std::vector<std::string> domainAndPort = SplitString(name, ':');
			if (domainAndPort.size() != 2)
				return {};
			domain = domainAndPort[0];
			port = domainAndPort[1];
			name = domain;
		}

		addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* addressResult = NULL;
		DWORD dwRetval = getaddrinfo(name.data(), nullptr, &hints, &addressResult);
		if (dwRetval != 0)
		{
			printf("getaddrinfo failed with error: %d\n", dwRetval);
			WSACleanup();
			return {};
		}

		std::string ipAddressStr;
		for (addrinfo* ptr = addressResult; ptr != NULL; ptr = ptr->ai_next)
		{
			switch (ptr->ai_family)
			{
			case AF_UNSPEC:
				// Unspecified
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
				INT iRetval = WSAAddressToStringA(sockaddr_ip, (DWORD)ptr->ai_addrlen, nullptr, ipstringbuffer, &actualIPBufferLength);

				if (iRetval)
				{
					printf("WSAAddressToString failed with %u\n", WSAGetLastError());
					WSACleanup();
					return {};
				}

				ipAddressStr = std::string(ipstringbuffer, actualIPBufferLength);
			}
			}

		}

		freeaddrinfo(addressResult);
		WSACleanup();

		return hasPort ? (ipAddressStr + ":" + port) : ipAddressStr;
#else
		bool hasPort = name.find(":") != std::string::npos;
		std::string domain, port;
		if (hasPort)
		{
			std::vector<std::string> domainAndPort = SplitString(name, ':');
			if (domainAndPort.size() != 2)
				return {};
			domain = domainAndPort[0];
			port = domainAndPort[1];
			name = domain;
		}

		addrinfo hints{};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* res = nullptr;
		int err = getaddrinfo(name.data(), nullptr, &hints, &res);
		if (err != 0)
		{
			std::cerr << "getaddrinfo failed: " << gai_strerror(err) << "\n";
			return {};
		}

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
				// Don't break — prefer IPv4 if available, but fallback to IPv6
			}
		}

		freeaddrinfo(res);

		return hasPort ? (ipAddressStr + ":" + port) : ipAddressStr;
#endif
	}

}

int main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	std::cout << "Hello, world!" << std::endl;

	std::cout << Nano::Networking::Utils::ResolveDomainName("google.com") << std::endl;
	std::cout << std::boolalpha << Nano::Networking::Utils::IsValidIPAddress(Nano::Networking::Utils::ResolveDomainName("google.com")) << std::endl;

	return 0;
}