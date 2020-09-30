import os
from ftplib import FTP

class V2XDataFtpClient:
    def __init__(self, ftpServerIp: str, ftpServerPort:int, clientUsername:str, clientPassword:str):

        self.ftpServerIp = ftpServerIp

        self.ftp = FTP()
        self.ftp.connect(ftpServerIp, port=ftpServerPort)
        self.ftp.login(user=clientUsername, passwd=clientPassword)

    def transfer_data(self, directoryPath:str, filename:str):

            if "spat" in filename: storCommand = "STOR spat/" + filename
            elif "remoteBsm" in filename: storCommand = "STOR remoteBsm/" + filename
            elif "srm" in filename: storCommand = "STOR srm/" + filename
            elif "ssm" in filename: storCommand = "STOR ssm/" + filename
            elif "msgCount" in filename: storCommand = "STOR msgCount/" + filename
            else: return

            try:
                with open((directoryPath + "/" + filename),'rb') as file:
                    self.ftp.storbinary(storCommand, file)
                os.remove((directoryPath + "/" + filename))
            except FTP.all_errors:
                pass
    
    def close_connection(self):
        self.ftp.quit()