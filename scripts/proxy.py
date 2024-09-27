from scapy.all import sniff, IP, UDP, send, Raw

# flake8: noqa

# Defina o IP de destino que deseja monitorar
target_ip = '192.168.1.2'
port = 5000
source_ip = '192.168.0.105'


# Função de callback para processar pacotes capturados
def packet_callback(packet):
    # Verificar se o pacote é UDP e tem o IP e porta de destino corretos
    if UDP in packet and packet[IP].dst == target_ip and packet[UDP].dport == port:
        data = packet[UDP].payload
        raw_data = bytes(data)
        hex_data = raw_data.hex()

        print(f"Pacote UDP Capturado de {packet[IP].src} para {packet[IP].dst}")
        print(f"Dados Brutos: {raw_data}")
        print(f"Dados em Hexadecimal: {hex_data}")
        print('-' * 50)

        package = IP(src=target_ip, dst=source_ip) / UDP(dport=port, sport=port) / Raw(load=raw_data)

        send(package, iface='Ethernet')

# Inicia a captura de pacotes na interface de rede (por exemplo, 'eth0' ou
# 'wlan0' no Linux)
# No Windows, pode usar 'Wi-Fi' ou outra interface visível
interface = 'Ethernet'  # Mude para a interface de rede que deseja monitorar

# Captura os pacotes, filtrando apenas os pacotes IP enviados para o target_ip
sniff(iface=interface, filter=f"ip dst {target_ip}",
      prn=packet_callback, store=0)
