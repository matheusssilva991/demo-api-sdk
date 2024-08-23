# Demo SDK API

Esta API, escrita em C++, foi desenvolvida para interagir com o detector **X-Panel 1412i**. Ela oferece funcionalidades para captura de imagens, controle de parâmetros do detector e integração com sistemas de processamento de imagem em tempo real.

## Tabela de Conteúdos

- [Requisitos](#requisitos)
- [Instalação](#instalação)
  - [MinGW](#mingw-compiler)
  - [Visual Studio](#visual-studio)
- [Como Rodar](#como-rodar)

## Requisitos

- **MinGW Compiler**: Versão mínima 8.1.0
- **Visual Studio**: Recomendado Visual Studio 2019 ou posterior
- **MSYS2**: Para gerenciamento de pacotes no Windows

## Instalação

### MinGW Compiler

1. Baixe o gerenciador de pacotes MSYS2:
   [MSYS2 - Software Distribution and Building Platform for Windows](https://www.msys2.org/)

2. Instale o MSYS2 e, no terminal do MSYS2, use o seguinte comando para instalar o MinGW:

    ```bash
    pacman -S mingw-w64-ucrt-x86_64-gcc
    ```

3. Adicione o caminho do MinGW ao PATH do sistema para garantir que o compilador esteja acessível de qualquer lugar.

### Visual Studio

1. Baixar o Visual Studio: [Visual Studio - IDE para desenvolvedores .NET e C++ no Windows.](https://visualstudio.microsoft.com/pt-br/).

2. Durante a instalação, certifique-se de selecionar as ferramentas de desenvolvimento C++ para desktop.

3. Após a instalação, configure o ambiente de desenvolvimento para trabalhar com o C++.

## Como rodar

Para rodar o projeto, basta abrir o arquivo de solução ``Demo_C++2017.sln`` no Visual Studio e compilar o projeto. O executável será gerado na pasta ``bin``.

Caso tenha problemas ao compilar o código no Visual Studio, pode ser necessário configurar o ambiente de desenvolvimento para que o compilador encontre as bibliotecas necessárias. Para isso, siga os passos abaixo:

1. Abra o arquivo de solução ``Demo_C++2017.sln`` no Visual Studio.

2. Clique com o botão direito no projeto ``Demo_C++2017`` e selecione a opção ``Propriedades``.

3. No menu lateral, em ``Geral``, defina o valor ``bin\$(Platform)\$(Configuration)\`` para ``Diretório de saída`` e ``Diretório intermediário`` .

4. No menu lateral, selecione a opção ``C\C++``, ``Geral`` e depois coloque o valor ``../include/`` em ``Diretórios de inclusão adicionais``.

5. No menu lateral, selecione a opção ``Linker``, ``Geral`` e depois coloque o valor ``../lib/x64`` em ``Diretórios de biblioteca adicionais``.

6. No menu lateral, selecione a opção ``Linker``, ``Entrada`` e depois coloque o valor ``kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)`` em ``Dependências adicionais``.

7. Clique em ``Aplicar`` e depois em ``OK``.

8. Clique com o botão direito no arquivo ``Demo.cpp`` e selecione a opção ``Propriedades``.

9. No menu lateral, selecione a opção ``C\C++``, ``Cabeçalhos Pré-compilados`` e depois coloque o valor ``Usar (/Yu)`` em ``Diretórios de inclusão adicionais``.

10. Clique em ``Aplicar`` e depois em ``OK``.

11. Repita os passos 8 a 10 para o arquivo ``stdafx.h``, contudo, no passo 9 coloque o valor ``Criar (/Yu)``.

Após seguir esses passos, compile o projeto novamente e o executável será gerado na pasta ``bin``.
