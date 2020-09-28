import os
import json
from ftplib import FTP

with open("/nojournal/bin/mmitss-phase3-master-config.json", 'r') as configFile:
    config = json.load(configFile)


ftp = FTP()
ftp.connect(config["DataCollectorIP"],port=config["PortNumber"]["DataCollectorFtpServer"])
ftp.login(user=config["IntersectionName"],passwd="mmitss123")

path = '/nojournal/bin/v2x-data/archive'
archive = list(os.walk(path))[0][1]

while True:
    for directory in archive:
        filenames = os.listdir((path + "/" + directory))
        for filename in filenames:
            if "spat" in filename:
                with open((path + "/" + directory + "/" + filename),'rb') as file:
                    storCommand = "STOR spat/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path + "/" + directory + "/" + filename))

            if "srm" in filename:
                with open((path + "/" + directory + "/" + filename),'rb') as file:
                    storCommand = "STOR srm/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path + "/" + directory + "/" + filename))

            if "ssm" in filename:
                with open((path + "/" + directory + "/" + filename),'rb') as file:
                    storCommand = "STOR ssm/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path + "/" + directory + "/" + filename))

            if "remoteBsm" in filename:
                with open((path + "/" + directory + "/" + filename),'rb') as file:
                    storCommand = "STOR remoteBsm/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path + "/" + directory + "/" + filename))

            if "msgCounts" in filename:
                with open((path + "/" + directory + "/" + filename),'rb') as file:
                    storCommand = "STOR msgCount/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path + "/" + directory + "/" + filename))
ftp.quit()
