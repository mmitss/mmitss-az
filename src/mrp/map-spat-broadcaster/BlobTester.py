import socket

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ip = '10.254.56.49'
port = 6053
testerAddress = (ip, port)
s.bind(testerAddress)

while True:
    tempBlob, addr = s.recvfrom(1024)
    minTimeToChange_1 = int(tempBlob[3]+tempBlob[4])
    minTimeToChange_2 = int(tempBlob[16]+tempBlob[17])
    minTimeToChange_3 = int(tempBlob[29]+tempBlob[30])
    minTimeToChange_4 = int(tempBlob[42]+tempBlob[43])
    minTimeToChange_5 = int(tempBlob[55]+tempBlob[56])
    minTimeToChange_6 = int(tempBlob[68]+tempBlob[69])
    minTimeToChange_7 = int(tempBlob[81]+tempBlob[82])
    minTimeToChange_8 = int(tempBlob[94]+tempBlob[95])
    minTimeToChange = [minTimeToChange_1,minTimeToChange_2,minTimeToChange_3,minTimeToChange_4,minTimeToChange_5,minTimeToChange_6,minTimeToChange_7,minTimeToChange_8]

    maxTimeToChange_1 = int(tempBlob[5]+tempBlob[6])
    maxTimeToChange_2 = int(tempBlob[18]+tempBlob[19])
    maxTimeToChange_3 = int(tempBlob[31]+tempBlob[32])
    maxTimeToChange_4 = int(tempBlob[44]+tempBlob[45])
    maxTimeToChange_5 = int(tempBlob[57]+tempBlob[58])
    maxTimeToChange_6 = int(tempBlob[70]+tempBlob[71])
    maxTimeToChange_7 = int(tempBlob[83]+tempBlob[84])
    maxTimeToChange_8 = int(tempBlob[96]+tempBlob[97])
    maxTimeToChange = [maxTimeToChange_1,maxTimeToChange_2,maxTimeToChange_3,maxTimeToChange_4,maxTimeToChange_5,maxTimeToChange_6,maxTimeToChange_7,maxTimeToChange_8]

    print ("minTimeToChange: " + str(minTimeToChange))
    print ("maxTimeToChange: " + str(maxTimeToChange))
    print ('')
