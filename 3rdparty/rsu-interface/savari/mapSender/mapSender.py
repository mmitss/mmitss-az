import socket
from time import sleep

def Main():
    host = '10.254.56.49'
    port = 5002
    sendingInterval = 0.1

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((host, port))

    server = ('10.254.56.45', 1516)

    fileName = 'MAPActiveList.db'
    
    f = open(fileName, "rb")
    data = f.read(2048)
    while True:
        s.sendto(data, server)
        sleep(sendingInterval)
    s.close()
if __name__ == "__main__":
    Main()
