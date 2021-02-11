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

WORKING_DIRECTORY = str(sh.pwd())[:-1]

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

if __name__ == "__main__":
    main()