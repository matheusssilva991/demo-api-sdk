from scapy.all import sniff, IP, UDP
import winsound
import os


# flake8: noqa

# Defina o IP de destino que deseja monitorar
target_ip = "192.168.1.2"
img_port = 4001
source_ip = "192.168.0.106"


# Função de callback para processar pacotes capturados
def packet_callback(packet):

    # Verificar se o pacote é UDP e tem o IP e porta de destino corretos
    if UDP in packet:

        if packet[IP].dst == source_ip and packet[UDP].dport == img_port:
            data = packet[UDP].payload
            raw_data = bytes(data)
            hex_data = raw_data.hex()

            if hex_data == "bcbce1000000000000002cdc60007a00000011691427fcfc":
                # Emite um beep com frequência de 750 Hz e duração de 500ms
                winsound.Beep(750, 500)
                os.system("echo \a")
            elif hex_data == "bcbce100010000007e002cdc60007a0000008233dbf1fcfc":
                # Emite um beep com frequência de 1000 Hz e duração de 500ms
                winsound.Beep(1000, 500)
                os.system("echo \a")


# Inicia a captura de pacotes na interface de rede (por exemplo, 'eth0' ou
# 'wlan0' no Linux)
# No Windows, pode usar 'Wi-Fi' ou outra interface visível
interface = "Ethernet"  # Mude para a interface de rede que deseja monitorar

# Captura os pacotes, filtrando apenas os pacotes IP enviados para o target_ip
sniff(
    iface=interface,
    filter=f"(ip dst {target_ip}) or (ip src {target_ip})",
    prn=packet_callback,
    store=0,
)
