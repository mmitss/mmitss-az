#!/usr/bin/env python3
import socket
import datetime

def Main():
	timestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))
	host = '10.254.56.49'
	port = 5001
	loop = 1
	srmNum = 1
	receiveFlag = 0
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	s.bind((host,port))
	print("SRM Receiver Started")
	outhex = open(("srmLog_" + timestamp), "w")
	while loop:
		binData, addr = s.recvfrom(1024)
		hexData = binData.hex()
		srm = hexData.replace('ffffffffffff','')
		outhex.write(srm + '\n')
		print("Received SRM# " + str(srmNum))
		srmNum = srmNum + 1
		loop = loop + 1
	outhex.close()
	s.close()
if __name__ == '__main__':
	Main()
