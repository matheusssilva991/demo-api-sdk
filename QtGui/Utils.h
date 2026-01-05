/**
 * @file Utils.h
 * @brief Funções utilitárias para validação e processamento de dados
 *
 * Este arquivo contém funções auxiliares usadas em todo o projeto,
 * especialmente para validação de entrada do usuário.
 *
 * @author Matheus Silva
 * @date 2024-2026
 */

#pragma once
#ifndef UTILS_H // Guarda de inclusão múltipla
#define UTILS_H

#include <iostream>

/**
 * @brief Valida se uma string representa um endereço IPv4 válido
 *
 * Utiliza expressão regular para verificar se o formato corresponde a
 * um endereço IPv4 padrão (4 octetos separados por pontos).
 *
 * Formato aceito: XXX.XXX.XXX.XXX onde cada XXX é de 0-255
 *
 * Exemplos válidos:
 * - 192.168.0.1
 * - 10.0.0.1
 * - 255.255.255.255
 *
 * Exemplos inválidos:
 * - 256.1.1.1 (octeto > 255)
 * - 192.168.1 (faltam octetos)
 * - abc.def.ghi.jkl (não-numérico)
 *
 * @param ip String contendo o endereço IP a ser validado
 * @return true se o IP é válido, false caso contrário
 *
 * @note Esta função valida apenas o formato, não verifica se o
 *       endereço existe ou está acessível na rede
 */
bool isValidIP(const std::string &ip);

#endif // UTILS_H