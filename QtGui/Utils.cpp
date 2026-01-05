/**
 * @file Utils.cpp
 * @brief Implementação de funções utilitárias
 *
 * @author Matheus Silva
 * @date 2024-2026
 */

#include "Utils.h"
#include <iostream>
#include <regex>

/**
 * @brief Valida formato de endereço IPv4 usando expressão regular
 *
 * A expressão regular verifica:
 * 1. Quatro grupos de números separados por pontos
 * 2. Cada grupo (octeto) deve estar entre 0-255:
 *    - 25[0-5]: 250-255
 *    - 2[0-4][0-9]: 200-249
 *    - [01]?[0-9][0-9]?: 0-199
 *
 * @param ip String com o endereço IP a validar
 * @return true se válido, false se inválido
 *
 * @example
 * isValidIP("192.168.1.1")  // retorna true
 * isValidIP("256.1.1.1")    // retorna false
 * isValidIP("192.168.1")    // retorna false
 */
bool isValidIP(const std::string &ip)
{
	// Expressão regular para validar formato IPv4
	// Padrão: XXX.XXX.XXX.XXX onde cada XXX é 0-255
	const std::regex ip_regex(
		R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");

	// Verifica se a string corresponde ao padrão
	return std::regex_match(ip, ip_regex);
}
