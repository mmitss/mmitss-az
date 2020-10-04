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

    # Read the configuration file
    with open("/nojournal/bin/v2x-data-ftp-server-config.json", 'r') as configFile:
        config = json.load(configFile)

    # Read start and end of the allocated interval for data transfer, and convert both to seconds from midnight
    startHour = config["CyVerseDataTransfer"]["StartTime"]["Hour"]
    startMinute = config["CyVerseDataTransfer"]["StartTime"]["Minute"]
    startSecondFromMidnight = ((startHour * SEC_IN_HOUR) + (startMinute * SEC_IN_MINUTE))
    
    endHour = config["CyVerseDataTransfer"]["EndTime"]["Hour"]
    endMinute = config["CyVerseDataTransfer"]["EndTime"]["Minute"]
    endSecondFromMidnight = ((endHour * SEC_IN_HOUR) + (endMinute * SEC_IN_MINUTE))

    # If invalid start and end time of allocated interval are detecteded, assign the default values
    if ((endSecondFromMidnight < startSecondFromMidnight) or startHour >= HOURS_IN_DAY or endHour >= HOURS_IN_DAY or startMinute >= MINUTES_IN_HOUR or endMinute >= MINUTES_IN_HOUR):
        startSecondFromMidnight = ((DEFAULT_START_HOUR * SEC_IN_HOUR) + (DEFAULT_START_MINUTE * SEC_IN_MINUTE))
        endSecondFromMidnight = ((DEFAULT_END_HOUR * SEC_IN_HOUR) + (DEFAULT_END_MINUTE * SEC_IN_MINUTE))

    # Read client information
    intersections = config["Clients"]

    # Read local gateway information
    localNetworkGateway = config["LocalNetworkGateway"]
    dnsServer = config["DnsServer"]

    # Read email-related information
    emailSenderAccount = config["NotificationEmail"]["SenderAccount"]
    emailSenderPassword = config["NotificationEmail"]["Password"]
    emailSslPort = config["NotificationEmail"]["SslPort"]
    notificationEmailRecipients = config["NotificationEmail"]["Recipients"]
    
    while True:
        # Get current time and convert it to seconds from midnight
        now = datetime.datetime.now()
        nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
        
        # Check if the allocated time interval has already began. If not, the sleep until it begins
        if nowSecondFromMidnight < startSecondFromMidnight:
            sleepTime = startSecondFromMidnight - nowSecondFromMidnight
            time.sleep(sleepTime)

        # Check if the allocated time interval has already over. If yes, sleep until the next day's allocated interval
        elif nowSecondFromMidnight >= endSecondFromMidnight:
            sleepTime = SEC_IN_DAY - nowSecondFromMidnight + startSecondFromMidnight
            time.sleep(sleepTime)
        
        # Finally, if the allocated time interval is in progress:
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
                # If the internet or local network is not reachable, notify through email to concerned personnels
                if (localNetworkIsReachable==False or internetIsReachable==False):
                    send_notification_email(emailSenderAccount,emailSenderPassword,emailSslPort,notificationEmailRecipients)

                # Sleep until the end of the current allocated time interval
                now = datetime.datetime.now()
                nowSecondFromMidnight = ((now.hour * SEC_IN_HOUR) + (now.minute * SEC_IN_MINUTE) + (now.second))
                time.sleep(endSecondFromMidnight-nowSecondFromMidnight)
                break

def send_notification_email(emailSenderAccount,emailSenderPassword,emailSslPort,notificationEmailRecipients):
    """
    Establishes the SSL connection and sends email to the reciepients listed in the argument.
    If email can not be sent for some reason, inform in the console output.
    """
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
    """
    Transfers the content from the localDirectory to the cyverseDirectory - Needs internet connection.
    """
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
    """
    Pings the serverIp for one packet and timeout=100ms. If successful, returns True, else returns False.
    """
    packets = 1
    timeout = 1000
    command = ['ping', '-c', str(packets), '-w', str(timeout), serverIp]
    result = subprocess.run(command, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return result.returncode == 0

if __name__ == "__main__":
    main()