"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

data-compressor.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************
"""

import os
import shutil

path = "/nojournal/bin/v2x-data/archive"

directories = list(os.walk(path))[0][1]

while len (directories) > 0:
    for directory in directories:
        shutil.make_archive((path + "/" + directory), "zip", (path + "/" + directory))
        shutil.rmtree((path + "/" + directory))
        directories = list(os.walk(path))[0][1]
