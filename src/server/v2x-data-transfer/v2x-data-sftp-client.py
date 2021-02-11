import datetime
import pysftp

def pull_files_from_server(intersections,dataElements,dataElementFiles,localDataDirectory):
    '''
    This function performs the following tasks for each intersection:
    (a) Tries to establish an SFTP connection with the intersection. If the connection is successful:
    (b) Finds the v2x-data/archive directory located on the intersection coprocessor.
    (c) There may be multiple directories in the archive. For each directory:
    (d) Using SFTP, transfers all files to the local machine and places them locally in respective folders based on the data element.
    (e) Deletes the remote directory after transfering to the server
    '''
    # For each intersection (remote machine):
    for intersection in intersections:
        try:
            # Establish an SFTP connection
            with pysftp.Connection(intersection["ip_address"], username=intersection["username"], password=intersection["password"]) as sftp:
                try:
                    # On remote machine, change the working directory to v2x-data/archive
                    with sftp.cd(intersection["v2x-data_location"] + "/archive/"):
                        # List archived directories
                        remoteArchivedDirectories = sftp.listdir()
                        
                        # For each archived directory:
                        for directory in remoteArchivedDirectories:
                            # On remote machine, change the working directory to v2x-data/archive/archivedDirectory
                            with sftp.cd(intersection["v2x-data_location"] + "/archive/" + directory):
                                # List all files available in the directory
                                dataFiles = sftp.listdir()
                                
                                # Identify the data element associated with each available file, and store it in a "dataElementFiles" dictionary
                                for dataElement in dataElements:
                                    filename = [file for file in dataFiles if dataElement in file]
                                    if len(filename) > 0:
                                        dataElementFiles[dataElement] = filename[0]
                                    else: dataElementFiles[dataElement] = None
                                
                                # For each data element:
                                for dataElement in dataElements:
                                    # If the file exists:
                                    if not dataElementFiles[dataElement] == None:
                                        # Set the local path where file needs to be transferred:
                                        localpath=localDataDirectory + "/" + intersection["name"] + "/" + dataElement + "/" + dataElementFiles[dataElement]
                                        # Transfer the file from the remote machine to the local path defined in previous step
                                        sftp.get(dataElementFiles[dataElement], localpath=localpath)
                            
                            # Reset the "dataElementFiles" files dictionary 
                            dataElementFiles = {"spat" : None, "srm": None, "remoteBsm": None, "ssm": None, "msgCount": None}
                            # Remove the data directory from the remote machine
                            sftp.rmdir(intersection["v2x-data_location"] + "/archive/" + directory)
                
                # If the v2x-data/archive directory can not be found on the remote machine, print the error message to the console
                except: print("Failed to locate v2x-data archive on " + intersection["name"] + " at:" + str(datetime.datetime.now()))
            
            # Else print on the console the success message
            print("Data transfer from " + str(intersection["name"]) + " completed at: " + str(datetime.datetime.now()))
        
        # If SFTP connection can not be established with the remote machine, print the error message on the console
        except: print("Failed to establish SFTP connection with " + intersection["name"] + " at: " + str(datetime.datetime.now()))
        

if __name__ == "__main__":
    
    import os
    import json
    import atexit
    import time
    from apscheduler.schedulers.background import BackgroundScheduler
    from apscheduler.triggers import interval

    # Read the config file to store the information in variables
    with open("./v2x-data-transfer-config.json", 'r') as configFile:
        config = json.load(configFile)

    localDataDirectory = config["local_data_directory"]
    intersections = config["intersections"]
    startHour = config["data_transfer_start_24h"]["from_intersections"]["hour"]
    startMinute = config["data_transfer_start_24h"]["from_intersections"]["minute"]

    # If invalid start time is detected, default to the midnight:
    if startHour < 0 or startHour > 23 or startMinute < 0 or startMinute > 59:
        print("Invalid Start Time. Defaulting to midnight!")
        startHour = 0
        startMinute = 0

    # Define data elements
    dataElements = ["spat", "srm", "remoteBsm", "ssm", "msgCount"]
    # For each data element, define the data file name
    dataElementFiles = {"spat" : None, "srm": None, "remoteBsm": None, "ssm": None, "msgCount": None}

    # Check if directory structure exists for listed intersections. If not, create the required folders:
    for intersection in intersections:
        intersectionDirectory = localDataDirectory + "/" + intersection["name"]
        for dataElement in dataElements:
            dataElementDirectory = intersectionDirectory + "/" + dataElement 
            if not os.path.isdir(dataElementDirectory):
                os.makedirs(dataElementDirectory)


    # Create an instance of the background scheduler and start it
    backgroundScheduler = BackgroundScheduler()
    backgroundScheduler.start()
    # Register the shutdown of background scheduler at exit
    atexit.register(lambda: backgroundScheduler.shutdown(wait=False))

    # Get current date and time
    now = datetime.datetime.now()
    
    #If it is in the past (for current day) create the start time (datetime object) for the data transfer from next day
    if ((int(now.hour) > startHour) or  ((int(now.hour) == startHour) and (int(now.minute) >= startMinute))):
        start = (datetime.datetime.now() + datetime.timedelta(days=1)).replace(hour=startHour, minute=startMinute, second=0)

    # Else create the start time (datetime object) for the data transfer starting from current day
    else: start = (datetime.datetime.now()).replace(hour=startHour, minute=startMinute, second=0)

    # Create an object of interval class. Use the datetime object created earlier for the start time.
    intervalTrigger = interval.IntervalTrigger(days=1, start_date=start)                                               

    # Add the recurring job in the background scheduler
    backgroundScheduler.add_job(pull_files_from_server, 
                                            args = [intersections,dataElements,dataElementFiles,localDataDirectory], 
                                                trigger = intervalTrigger, 
                                                max_instances=3)
 
    # Once the job is added in the background scheduler, there is nothing much to do except ensuring that the application is running indefinitely.
    while True: time.sleep(3600) # Note: There is nothing special about number 3600 (seconds) for sleep time. It can be any large number. Larger the better!
                            



    
