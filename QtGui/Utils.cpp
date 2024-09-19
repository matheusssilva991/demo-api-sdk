#include "Utils.h"
#include <iostream>
#include <regex>

bool isValidIP(const std::string& ip)
{
	// Express�o regular para um endere�o IP v�lido (IPv4)
	const std::regex ip_regex(
		R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
	);

	// Verificar se a string corresponde ao padr�o da express�o regular
	return std::regex_match(ip, ip_regex);
}

