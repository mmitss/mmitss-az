"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

v2x-data-cyverse-interface.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************
"""

import os
import subprocess
import json
import datetime, time
import sh
import ssl, smtplib

HOURS_IN_DAY = 24
MINUTES_IN_HOUR = 60

SEC_IN_DAY = 24 * 60 * 60
SEC_IN_HOUR = 60 * 60
SEC_IN_MINUTE = 60

DEFAULT_START_HOUR = 1
DEFAULT_START_MINUTE = 0
DEFAULT_END_HOUR = 1
DEFAULT_END_MINUTE = 30

SEC_BEFORE_RETRY = 60

WORKING_DIRECTORY = str(sh.pwd())[:-1]

def main():
    with open("/nojournal/bin/v2x-data-ftp-server-config.json", 'r') as configFile:
        config = json.load(configFile)

    startHour = config["CyVerseDataTransfer"]["StartTime"]["Hour"]
    startMinute = config["CyVerseDataTransfer"]["StartTime"]["Minute"]
    startSecondFromMidnight = ((startHour * SEC_IN_HOUR) + (startMinute * SEC_IN_MINUTE))
    
    endHour = config["CyVerseDataTransfer"]["EndTime"]["Hour"]
    endMinute = config["CyVerseDataTransfer"]["EndTime"]["Minute"]
    endSecondFromMidnight = ((endHour * SEC_IN_HOUR) + (endMinute * SEC_IN_MINUTE))

    if ((endSecondFromMidnight < startSecondFromMidnight) or startHour >= HOURS_IN_DAY or endHour >= HOURS_IN_DAY or startMinute >= MINUTES_IN_HOUR or endMinute >= MINUTES_IN_HOUR):
        startSecondFromMidnight = ((DEFAULT_START_HOUR * SEC_IN_HOUR) + (DEFAULT_START_MINUTE * SEC_IN_MINUTE))
        endSecondFromMidnight = ((DEFAULT_END_HOUR * SEC_IN_HOUR) + (DEFAULT_END_MINUTE * SEC_IN_MINUTE))

    intersections = config["Clients"]

    localNetworkGateway = config["LocalNetworkGateway"]
    dnsServer = config["DnsServer"]

    emailSenderAccount = config["NotificationEmail"]["SenderAccount"]
    emailSenderPassword = config["NotificationEmail"]["Password"]
    emailSslPort = config["NotificationEmail"]["SslPort"]
    notificationEmailRecipients = config["NotificationEmail"]["Recipients"]
    

    while True:
        now = datetime.datetime.now()
        nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
        
        if nowSecondFromMidnight < startSecondFromMidnight:
            sleepTime = startSecondFromMidnight - nowSecondFromMidnight
            time.sleep(sleepTime)

        elif nowSecondFromMidnight >= endSecondFromMidnight:
            sleepTime = SEC_IN_DAY - nowSecondFromMidnight + startSecondFromMidnight
            time.sleep(sleepTime)
        
        else: 
            while (nowSecondFromMidnight >= startSecondFromMidnight and nowSecondFromMidnight < endSecondFromMidnight):
                now = datetime.datetime.now()
                nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
                internetIsReachable = check_network_connection(dnsServer)
                if (internetIsReachable):
                    for intersection in intersections:
                        parentDirectory = intersection["LocalDirectory"]
                        dataDirectories = list(os.walk(parentDirectory))[0][1]
                        for dataDirectory in dataDirectories:
                            localDirectory = parentDirectory + "/" + dataDirectory
                            cyverseDirectory = intersection["CyVerseDirectories"][dataDirectory]
                            transfer_directory_content(localDirectory, cyverseDirectory)
                
                localNetworkIsReachable = check_network_connection(localNetworkGateway)
                if (localNetworkIsReachable==False or internetIsReachable==False):
                    send_notification_email(emailSenderAccount,emailSenderPassword,emailSslPort,notificationEmailRecipients)

                now = datetime.datetime.now()
                nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
                time.sleep(endSecondFromMidnight-nowSecondFromMidnight)
                break

def send_notification_email(emailSenderAccount,emailSenderPassword,emailSslPort,notificationEmailRecipients):
    text = get_email_text()
    context = ssl.create_default_context()
    try:
        with smtplib.SMTP_SSL("smtp.gmail.com", emailSslPort, context=context) as server:
            server.login(emailSenderAccount, emailSenderPassword)
            for receipient in notificationEmailRecipients:
                server.sendmail(emailSenderAccount, receipient, text)
                print("Email sent to {}".format(receipient))
    except:
        print("Network not reachable but notification email could not be sent at: " + str(datetime.datetime.now()))


def get_email_text():
    text = """\
Subject: IAM Data Server

Hello,

IAM Server could not establish communication with local network at {}.

This is an auto-generated email. Please do not reply.

Thanks.""".format(str(datetime.datetime.now()))
    return text


def transfer_directory_content(localDirectory:str, cyverseDirectory:str):
    try:
        sh.cd(localDirectory)
        sh.icd(cyverseDirectory)
        sh.iput("-r","-f",".")
        sh.cd(WORKING_DIRECTORY)
        sh.rm("-r", localDirectory)
        os.makedirs(localDirectory)
    except:
        pass

def check_network_connection(serverIp:str):
    packets = 1
    timeout = 100
    command = ['ping', '-c', str(packets), '-w', str(timeout), serverIp]
    result = subprocess.run(command, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return result.returncode == 0

if __name__ == "__main__":
    main()