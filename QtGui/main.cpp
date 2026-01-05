/**
 * @file main.cpp
 * @brief Ponto de entrada da aplicação gráfica QtGui
 *
 * Este arquivo inicializa a aplicação Qt e cria a janela principal
 * da interface gráfica para controle do detector X-Panel 1412i.
 *
 * Configurações aplicadas:
 * - Desabilita modo escuro do Windows para consistência visual
 * - Define título e ícone da janela
 * - Inicializa o event loop do Qt
 *
 * @author Matheus Silva
 * @date 2024-2026
 */

#include "QtGui.h"
#include <QtWidgets/QApplication>

/**
 * @brief Função principal da aplicação Qt
 * @param argc Número de argumentos de linha de comando
 * @param argv Array de argumentos de linha de comando
 * @return Código de saída da aplicação (0 = sucesso)
 *
 * Inicializa o framework Qt, cria a janela principal e inicia
 * o loop de eventos da aplicação.
 */
int main(int argc, char *argv[])
{
    // Desabilita modo escuro do Windows para manter aparência consistente
    // Necessário em Windows 10/11 que detectam automaticamente preferência de tema
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");

    // Cria a aplicação Qt
    QApplication a(argc, argv);

    // Cria a janela principal
    QtGui w;

    // Configura título da janela
    w.setWindowTitle("Auto Detector DTi 1412i");

    // Define ícone da janela (arquivo localizado em resources)
    w.setWindowIcon(QIcon(":/Assets/assets/x-ray.png"));

    // Exibe a janela
    w.show();

    // Inicia o loop de eventos e aguarda até a aplicação ser fechada
    return a.exec();
}
