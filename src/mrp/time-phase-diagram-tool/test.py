import os
import shutil
import time
path = "/nojournal/bin/performance-measurement-diagrams/time-phase-diagram/archive"

list_of_files = os.listdir(path)
full_path = [path + "/{0}".format(x) for x in list_of_files]