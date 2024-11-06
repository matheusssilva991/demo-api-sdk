from scapy.all import sniff, IP, UDP, send, Raw
import json
import time

# flake8: noqa

# Defina o IP de destino que deseja monitorar
target_ip = "192.168.1.2"
cmd_port = 3000
img_port = 4001
source_ip = "192.168.0.106"
channels_open = False

with open("scripts/data/dict_data.json", "r") as fd:
    dict_data = json.load(fd)


def send_udp_data(**kargs):
    data = kargs.get("data")
    target_ip = kargs.get("target_ip")
    source_ip = kargs.get("source_ip")
    port = kargs.get("port")
    verbose = kargs.get("verbose", False)
    iface = kargs.get("iface", "Ethernet")

    package = (
        IP(src=source_ip, dst=target_ip) / UDP(dport=port, sport=port) / Raw(load=data)
    )

    send(package, iface=iface, verbose=verbose)


# Função de callback para processar pacotes capturados
def packet_callback(packet):
    global channels_open

    # Verificar se o pacote é UDP e tem o IP e porta de destino corretos
    if UDP in packet:

        # Verifica se o canal de dados de comando está aberto
        # Se estiver, envia um pacote de heartbeat
        if channels_open:
            data = bytes.fromhex(dict_data["rcv"]["heartbeat"][0])

            # time.sleep(1)
            send_udp_data(
                data=data, target_ip=source_ip, source_ip=target_ip, port=cmd_port
            )

        if packet[IP].dst == target_ip and packet[UDP].dport == cmd_port:
            data = packet[UDP].payload
            raw_data = bytes(data)
            hex_data = raw_data.hex()

            print("1 " + "-" * 50)
            print(
                f"Dados enviados de {packet[IP].src} para {packet[IP].dst} na porta {packet[UDP].dport}"
            )
            print(f"Brutos: {raw_data}")
            print(f"Hexadecimal: {hex_data}")
            print("-" * 50)

            if not channels_open:
                data = dict_data["rcv"]["cmd_data_channel"][0]
                data = bytes.fromhex(data)

                send_udp_data(
                    data=data, target_ip=source_ip, source_ip=target_ip, port=cmd_port
                )
                dict_data["rcv"]["cmd_data_channel"].pop(0)

                if not dict_data["rcv"]["cmd_data_channel"]:
                    channels_open = True

            else:
                keys = list(dict_data["src"].keys())
                keys.remove("cmd_data_channel")

                for key in keys:
                    if hex_data in dict_data["src"][key]:
                        index = dict_data["src"][key].index(hex_data)

                        data = dict_data["rcv"][key][index]
                        data = bytes.fromhex(data)

                        send_udp_data(
                            data=data,
                            target_ip=source_ip,
                            source_ip=target_ip,
                            port=cmd_port,
                        )

        elif packet[IP].dst == source_ip and packet[UDP].dport == cmd_port:
            data = packet[UDP].payload
            raw_data = bytes(data)
            hex_data = raw_data.hex()

            print("2 " + "-" * 49)
            print(
                f"Dados enviados de {packet[IP].src} para {packet[IP].dst} na porta {packet[UDP].dport}"
            )
            print(f"Brutos: {raw_data}")
            print(f"Hexadecimal: {hex_data}")
            print("-" * 50)


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
