import os
import shutil

path = "/nojournal/bin/v2x-data/archive"

directories = list(os.walk(path))[0][1]

while len (directories) > 0:
    for directory in directories:
        shutil.make_archive((path + "/" + directory), "zip", (path + "/" + directory))
        shutil.rmtree((path + "/" + directory))
        directories = list(os.walk(path))[0][1]