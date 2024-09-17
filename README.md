# Software para manipulação do detector X-Panel 1412i

Software para interação com o detector X-Panel 1412i, desenvolvido em C++. Ele oferece funcionalidades para captura de imagens, controle de parâmetros do detector e integração com sistemas de processamento de imagens em tempo real.

## Tabela de Conteúdos

- [Requisitos](#requisitos)
- [Instalação](#instalação)
  - [MinGW](#mingw-compiler)
  - [Visual Studio](#visual-studio)
- [Como Rodar](#como-rodar)
- [Documentação API](#documentação-api)
  - [Manipular eventos](#manipular-eventos)
    - [Eventos de comando](#eventos-de-comando)
    - [Eventos de imagem](#eventos-de-imagem)
  - [Manipular componentes](#manipular-componentes)
    - [Sistema](#sistema)
      - [Registrar eventos](#registrar-eventos)
      - [Detectar dispositivos](#detectar-dispositivos)
      - [Obter um dispositivo](#obter-um-dispositivo-conectado)
      - [Realizar operações em um dispositivo](#realizar-operações-em-um-dispositivo)
    - [Comando](#comando)
      - [Instanciar e registrar eventos de comando](#instanciar-e-registrar-eventos-de-comandos)
      - [Enviar comandos ASCII](#enviar-comandos-ascii)
      - [Definir parâmetros do dispositivo](#definir-darâmetros-do-dispositivo)
      - [Obter parâmetros do dispositivo](#obter-parâmetros-do-dispositivo)
      - [Parâmetros](#parâmetros)
    - [Aquisição e transferência de imagens](#aquisição-e-transferência)
      - [Instanciar e registrar eventos](#instanciar-e-registrar-eventos-de-aquisição-e-transferência)
      - [Abrir e fechar canal de imagem](#abrir-e-fechar-canal-de-imagem)
      - [Capturar imagens](#capturar-imagens)

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

## Documentação API

### Manipular eventos

#### Eventos de comando

A API oferece a interface `IXCmdSink` para manipulação de eventos de comando do dispositivo. No exemplo abaixo, é apresentada uma implementação dessa interface:

```c++
//A class for handling command events
  class CmdSink :public IXCmdSink
  {
  public:
    // Manipulação de erro
    // Parâmetros err_id: ID do error, err_msg_: Mensagem de erro
    void OnXError(uint32_t err_id, const char* err_msg_) override
    {
      cout << "OnXError: " << err_id << ", " << err_msg_ << endl;
    }
    // Manipulação de eventos
    // Parâmetros event_id: ID do evento, data: Dados do evento
    // Eventos: _cisTemperature; _dasTemperature1; _dasTemperature2; _dasTemperature3; _dasHumidity;
    void OnXEvent(uint32_t event_id, XHealthPara data) override
    {
      // cout << "On Event ID " << event_id << " data " << data._dasTemperature1 << endl;
    }
  };
```

#### Eventos de imagem

A API oferece a interface `IXImgSink` para manipulação de eventos de imagem. No exemplo abaixo, é apresentada uma implementação dessa interface:

```c++
  class ImgSink : public IXImgSink
  {
    // Manipulação de erro
    // Parâmetros err_id: ID do error, err_msg_: Mensagem de erro
    void OnXError(uint32_t err_id, const char* err_msg_) override
    {
      printf("OnXERROR: %u, %s\n", err_id, err_msg_);
    }

    // Manipulação de eventos
    // Parâmetros event_id: ID do evento, data: Dados do evento
    // Eventos: XEVENT_IMG_PARSE_DATA_LOST, XEVENT_IMG_TRANSFER_BUF_FULL, XEVENT_IMG_PARSE_DM_DROP,
    //          XEVENT_IMG_PARSE_PAC_LOST, XEVENT_IMG_PARSE_MONITOR_STATUS_ERR
    void OnXEvent(uint32_t event_id, uint32_t data) override
    {
      if (XEVENT_IMG_PARSE_DATA_LOST == event_id)
      {
        lost_frame_count += data;
      }
    }

    // Manipulação de quadros prontos
    // Parâmetros image_: Ponteiro para o quadro
    void OnFrameReady(XImage* image_) override
    {
      printf("Frame %u ready, width %u, height %d,  lost line %u\n",
        frame_count++, image_->_width, image_->_height, lost_frame_count);

      if (is_save)
      {
        ximg_handle.Write(image_);
      }
    }

    // Manipulação de quadros completos
    void OnFrameComplete() override
    {
      printf("Grab complete.\n");

      if (is_save)
      {
        string txt_name = save_file_name.replace(save_file_name.find(".dat"), 4, ".txt");

        ximg_handle.SaveHeaderFile(txt_name.c_str());
        ximg_handle.CloseFile();

        is_save = 0;
      }

      frame_complete.Set();
    }
  };
```

### Manipular componentes

#### Sistema

A API oferece a classe `XSystem` para facilitar a comunicação entre o computador host e os dispositivos conectados. Para instanciar um objeto `XSystem`, é necessário fornecer o endereço IP do host no construtor. O exemplo abaixo demonstra como utilizar essa classe para estabelecer a conexão:

```c++
  XSystem xsystem("192.168.0.1");

  if (!xsystem.Open()) // Verifica se a conexão com o host foi estabelecida com sucesso
  {
      std::cerr << "Falha ao conectar ao host." << std::endl;
      return 0;  // Termina a execução se não conseguir conectar
  }
```

##### Registrar eventos

Para permitir que o sistema escute eventos gerados por dispositivos conectados, você precisa registrar um "event sink", como mostrado abaixo:

```c++
  CmdSink cmd_sink;
  xsystem.RegisterEventSink(&cmd_sink);
```

##### Detectar dispositivos

Para detectar a quantidade de dispositivos conectados ao sistema, você pode usar o método `FindDevices` da classe `XSystem`:

```c++
  xsystem.FindDevices();

  if (device_count <= 0)
  {
    printf("No device found.\n ");
    return 0;
  }
```

##### Obter um dispositivo conectado

Após detectar os dispositivos, você pode obter uma referência a um dispositivo conectado utilizando o método `GetDevice`:

```c++
  XDevice* xdevice_ptr = NULL;
  uint32_t device_id = 0;

  //Pega o primeiro dispositivo
  xdevice_ptr = xsystem.GetDevice(device_id);
```

##### Realizar operações em um dispositivo

A API oferece a classe `XDevice` para realizar operações em um dispositivo. Um dispositivo pode ser obtido com a função `GetDevice` da classe `XSystem`. A classe XDevice oferece métodos para acessar e configurar parâmetros do dispositivo, conforme mostrado no exemplo abaixo:

```c++
  XSystem xsystem("192.168.0.1");
  XDevice* xdevice_ptr = NULL;
  uint32_t device_id = 0;

  // Obtém o primeiro dispositivo conectado
  xdevice_ptr = xsystem.GetDevice(device_id);

  // Verifica se o dispositivo foi encontrado
  if (xdevice_ptr != NULL)
  {
      // Pega parâmetros do dispositivo
      cout << "Dispositivo encontrado: " << xdevice_ptr->GetIP() << endl;   // IP do dispositivo
      cout << "Porta de comando: " << xdevice_ptr->GetCmdPort() << endl;    // Porta de comando
      cout << "Porta de imagem: " << xdevice_ptr->GetImgPort() << endl;     // Porta de imagem

      // Define novos parâmetros do dispositivo
      xdevice_ptr->SetIP("192.168.0.100"); // Altera o IP do dispositivo
      xdevice_ptr->SetCmdPort(3000);       // Altera a porta de comando
      xdevice_ptr->SetImgPort(5000);       // Altera a porta de imagem

      // Atualiza as configurações do dispositivo
      xsystem.ConfigureDevice(xdevice_ptr);

      // Atualiza o ponteiro do dispositivo após configuração
      xdevice_ptr = xsystem.GetDevice(device_id);
  }
  else
  {
      cout << "Nenhum dispositivo encontrado com o ID fornecido." << endl;
  }
```

#### Comando

A classe XCommand é responsável por enviar comandos e parâmetros ao dispositivo conectado, assim como configurar o seu funcionamento. Abaixo está uma explicação detalhada sobre o uso de XCommand e os parâmetros que podem ser enviados.

##### Instanciar e registrar eventos de comandos

```c++
  XGigFactory xfactory;
  XCommand xcommand(&xfactory);

  // Registrar o gerenciador de eventos de comando
  xcommand.RegisterEventSink(&cmd_sink);

  // Abrir comunicação com o dispositivo
  xcommand.Open(xdevice_ptr);

  // Fechar comunicação com o dispositivo
  xcommand.Close();


```

##### Enviar comandos ASCII

```c++
  xcommand.SendAscCmd(send_str, recv_str);
  cout << "Resposta: " << recv_str << endl;

```

##### Definir darâmetros do dispositivo

```c++
  uint64_t cmd_para = 0;

  if (1 == xcommand.SetPara(XPARA_BINNING_MODE, cmd_para)) {
    cout << "Modo de binning definido com sucesso\n\n";
  }
```

##### Obter parâmetros do dispositivo

```c++
  uint64_t operation_mode;
  uint32_t device_id = 0;
  xcommand.GetPara(XPARA_OPE_MODE, operation_mode, device_id);
```

##### Parâmetros

Abaixo estão os parâmetros que podem ser utilizados com os métodos `SetPara` e `GetPara`:

- XPARA_INIT_PARA
- XPARA_SAVE_PARA
- XPARA_FRAME_PERIOD
- XPARA_NON_INTTIME
- XPARA_OPE_MODE
- XPARA_GAIN_RANGE
- XPARA_EN_SCAN
- XPARA_BINNING_MODE
- XPARA_OUTPUT_RESOLUTION
- XPARA_INPUT_TRIGGER_MODE
- XPARA_EN_INPUT_TRIGGER
- XPARA_OUTPUT_TRIGGER_MODE
- XPARA_EN_OUTPUT_TRIGGER
- XPARA_PIXEL_NUMBER
- XPARA_MAXMIN_FRAME_PERIOD
- XPARA_DAS_FIRM_VER
- XPARA_DAS_TEST_MODE
- XPARA_DAS_SERIAL
- XPARA_INIT1_PARA
- XPARA_EN_LED
- XPARA_DAS_HEALTH
- XPARA_CONFIG_FIRMWARE
- XPARA_EN_ROI
- XPARA_ROI
- XPARA_READ_OUT_PERIOD
- XPARA_READ_OUT_TIME
- XPARA_GCU_WORK_TIME
- XPARA_DEVICE_TYPE

#### Aquisição e transferência

A API fornece duas classes para gerenciar o processo de aquisição de imagens e a transferência de dados do dispositivo para o host, respectivamente. As classes envolvidas nesse processo são XFrameTransfer e XAcquisition.

##### Instanciar e registrar eventos de aquisição e transferência

```c++
  XGigFactory xfactory;
  XFrameTransfer xtransfer;
  XImgSink img_sink;

  // Registrar o manipulador de eventos de imagem (img_sink)
  xtransfer.RegisterEventSink(&img_sink);

  // Criar o objeto para gerenciar o processo de aquisição de imagens
  XAcquisition xacquisition(&xfactory);

  // Registrar o manipulador de eventos
  xacquisition.RegisterEventSink(&img_sink);

  // Registrar o manipulador de transferência
  xacquisition.RegisterFrameTransfer(&xtransfer);
```

##### Abrir e fechar canal de imagem

```c++
  // Abrir canal de imagem
  if (xacquisition.Open(xdevice_ptr, &xcommand))
  {
      cout << "Canal de imagem aberto com sucesso" << endl;
  }
  else
  {
      cout << "Falha ao abrir o canal de imagem" << endl;
  }

  // Fechar canal de imagem
  xacquisition.Close();
```

##### Capturar imagens

```c++
  // Quantidade de frames a serem capturados.
  // Passar um valor 0 faz a captura acontecer de forma contínua.
  int num_frames = 0;

  // Iniciar captura de frames de forma continua
  xacquisition.Grab(num_frames);

  // Parar captura de frames
  xacquisition.Stop();

  // Nova quantidade de frames
  num_frames = 15;

  // Iniciar captura de 15 frames
  xacquisition.Grab(num_frames);
```