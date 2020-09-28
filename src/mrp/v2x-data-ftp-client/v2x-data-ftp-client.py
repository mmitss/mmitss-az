import os
import json
from ftplib import FTP


def main():
    with open("/nojournal/bin/mmitss-phase3-master-config.json", 'r') as configFile:
        config = json.load(configFile)


    ftp = FTP()
    ftp.connect(config["DataCollectorIP"],port=config["PortNumber"]["DataCollectorFtpServer"])
    ftp.login(user=config["IntersectionName"],passwd="mmitss123")

    path = '/nojournal/bin/v2x-data/archive'

    while True:
        serverIsReachable = ping(config["DataCollectorIP"], 1, 1000)

        if serverIsReachable:
            archive = list(os.walk(path))[0][1]
            if len(archive) != 0:
                for directory in archive:
                    filenames = os.listdir((path + "/" + directory))
                    for filename in filenames:
                        if "spat" in filename:
                            with open((path + "/" + directory + "/" + filename),'rb') as file:
                                storCommand = "STOR spat/" + filename
                                ftp.storbinary(storCommand, file)
                            os.remove((path + "/" + directory + "/" + filename))

                        if "srm" in filename:
                            with open((path + "/" + directory + "/" + filename),'rb') as file:
                                storCommand = "STOR srm/" + filename
                                ftp.storbinary(storCommand, file)
                            os.remove((path + "/" + directory + "/" + filename))

                        if "ssm" in filename:
                            with open((path + "/" + directory + "/" + filename),'rb') as file:
                                storCommand = "STOR ssm/" + filename
                                ftp.storbinary(storCommand, file)
                            os.remove((path + "/" + directory + "/" + filename))

                        if "remoteBsm" in filename:
                            with open((path + "/" + directory + "/" + filename),'rb') as file:
                                storCommand = "STOR remoteBsm/" + filename
                                ftp.storbinary(storCommand, file)
                            os.remove((path + "/" + directory + "/" + filename))

                        if "msgCounts" in filename:
                            with open((path + "/" + directory + "/" + filename),'rb') as file:
                                storCommand = "STOR msgCount/" + filename
                                ftp.storbinary(storCommand, file)
                            os.remove((path + "/" + directory + "/" + filename))
                    
                    files = os.listdir(path + "/" + directory)
                    if len(files) == 0:
                        archive.remove(directory)
                        os.rmdir(path + "/" + directory)
    ftp.quit()

def ping(host, packets, timeout):
    import subprocess, platform

    ''' Calls system "ping" command, returns True if ping succeeds.
    Required parameter: host_or_ip (str, address of host to ping)
    Optional parameters: packets (int, number of retries), timeout (int, ms to wait for response)
    Does not show any output, either as popup window or in command line.
    Python 3.5+, Windows and Linux compatible (Mac not tested, should work)
    '''
    # The ping command is the same for Windows and Linux, except for the "number of packets" flag.
    if platform.system().lower() == 'windows':
        command = ['ping', '-n', str(packets), '-w', str(timeout), host]
        # run parameters: capture output, discard error messages, do not show window
        result = subprocess.run(command, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, creationflags=0x08000000)
        # 0x0800000 is a windows-only Popen flag to specify that a new process will not create a window.
        # On Python 3.7+, you can use a subprocess constant:
        #   result = subprocess.run(command, capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW)
        # On windows 7+, ping returns 0 (ok) when host is not reachable; to be sure host is responding,
        # we search the text "TTL=" on the command output. If it's there, the ping really had a response.
        return result.returncode == 0 and b'TTL=' in result.stdout
    else:
        command = ['ping', '-c', str(packets), '-w', str(timeout), host]
        # run parameters: discard output and error messages
        result = subprocess.run(command, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return result.returncode == 0

if __name__ == "__main__":
    main()