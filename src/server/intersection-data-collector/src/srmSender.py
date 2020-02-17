import socket

hostIp = '10.12.6.108'
port = 20001
addr = (hostIp, port)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(addr)

f = open("ssm.json", 'r')
msg = f.read()

s.sendto(msg.encode(), ("10.12.6.108", 30001))