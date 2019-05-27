#!/usr/bin/env python3
import socket
import datetime

def Main():
	timestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))
	host = '10.254.56.49'
	port = 5000
	loop = 1
	bsmNum = 1
	receiveFlag = 0
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	s.bind((host,port))
	print("BSM Receiver Started")
	outhex = open(("bsmLog_" + timestamp), "w")
	while loop:
		binData, addr = s.recvfrom(1024)
		hexData = binData.hex()
		bsm = hexData.replace('ffffffffffff70b3d50401dd88dc0b030f01ac10010c04019400202e03802b','')
		outhex.write(bsm + '\n')
		print("Received BSM# " + str(bsmNum))
		bsmNum = bsmNum + 1
		loop = loop + 1
	outhex.close()
	s.close()
if __name__ == '__main__':
	Main()
