#include "Utils.h"
#include <iostream>
#include <regex>

bool isValidIP(const std::string& ip)
{
	// Expressão regular para um endereço IP válido (IPv4)
	const std::regex ip_regex(
		R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
	);

	// Verificar se a string corresponde ao padrão da expressão regular
	return std::regex_match(ip, ip_regex);
}

