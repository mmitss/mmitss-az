import os
from ftplib import FTP
ftp = FTP()
ftp.connect("10.12.6.56",port=9090)
ftp.login(user="daisy-gavilan",passwd="mmitss123")

path = "/nojournal/bin/v2x_data/"
root, directories, files = os.walk(path)

while True:
    for directory in root[1]:
        lst = os.listdir((path + directory))
        for filename in lst:
            if "SpatLog" in filename:
                with open((path+directory+"/"+filename),'rb') as file:
                    storCommand = "STOR spat/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path+directory+"/"+filename))

            if "SrmLog" in filename:
                with open((path+directory+"/"+filename),'rb') as file:
                    storCommand = "STOR srm/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path+directory+"/"+filename))

            if "SsmLog" in filename:
                with open((path+directory+"/"+filename),'rb') as file:
                    storCommand = "STOR ssm/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path+directory+"/"+filename))

            if "RemoteBsmLog" in filename:
                with open((path+directory+"/"+filename),'rb') as file:
                    storCommand = "STOR surrounding_vehicle_data/" + filename
                    ftp.storbinary(storCommand, file)
                os.remove((path+directory+"/"+filename))
ftp.quit()
