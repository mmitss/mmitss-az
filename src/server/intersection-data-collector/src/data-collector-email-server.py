'''
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  data-collector-email-server.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

***************************************************************************************
'''
import socket
import json
import DataCollectorMethods as DCM
import datetime
import smtplib, ssl

def main():
    
    dataCollectorConfigFile = open('./../config/data-collection-module-config.json', 'r')
    dataCollectorConfig = json.load(dataCollectorConfigFile)
    
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = dataCollectorConfig["DataCollectorIp"]
    emailServerPort = dataCollectorConfig["DataCollectorEmailServer"]["Port"]
    emailServerAddress = (hostIp, emailServerPort)
    s.bind(emailServerAddress)
    s.settimeout(5)
    totalTransferSize = 0

#################### Email Config ####################
    port = 465
    sender_email = dataCollectorConfig["DataCollectorEmailServer"]["EmailAccount"]
    password = dataCollectorConfig["DataCollectorEmailServer"]["Password"]
    email_receipients = dataCollectorConfig["EmailRecipients"]
    context = ssl.create_default_context()
######################################################

    while True:
        try:
            data, address = s.recvfrom(64)
            transferSizeBytes = int(data)
            totalTransferSize = totalTransferSize + transferSizeBytes
        except socket.timeout:
            if totalTransferSize !=0:
                transferSize, unit = DCM.convertSizeBytesToAppropriateUnits(totalTransferSize)
                message = DCM.generateEmail(transferSize, unit)
                with smtplib.SMTP_SSL("smtp.gmail.com", port, context=context) as server:
                    server.login(sender_email, password)
                    for receipient in email_receipients:
                        server.sendmail(sender_email, receipient, message)
                totalTransferSize = 0
                

if __name__ == "__main__":
    main()
