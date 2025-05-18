import socket

UDP_IP = "192.168.1.128"
UDP_PORT = 12334
MESSAGE = b"Test from PC"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
