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
import time

MIN_DISK_SPACE_GB = 1
BYTES_PER_GB = 1024*1024*1024

path = "/nojournal/bin/v2x-data/archive"

directories = list(os.walk(path))[0][1]

while len (directories) > 0:
    for directory in directories:
        shutil.make_archive((path + "/" + directory), "zip", (path + "/" + directory))
        shutil.rmtree((path + "/" + directory))
        directories = list(os.walk(path))[0][1]

list_of_files = os.listdir(path)
full_path = [path + "/{0}".format(x) for x in list_of_files]

blockSize = os.statvfs(path).f_frsize
remainingDiskSpace_gb = os.statvfs(path)[4]*blockSize/(BYTES_PER_GB) 

while (remainingDiskSpace_gb) < MIN_DISK_SPACE_GB:
    print("Low disk space: removing old logs")
    remainingDiskSpace_gb = os.statvfs(path)[4]*blockSize/(BYTES_PER_GB)
    list_of_files = os.listdir(path)
    full_path = [path + "/{0}".format(x) for x in list_of_files]
    if len(full_path) > 0:
        oldest_file = min(full_path, key=os.path.getctime)
        os.remove(oldest_file)

time.sleep(10)
    

