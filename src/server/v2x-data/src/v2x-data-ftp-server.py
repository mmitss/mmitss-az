import os
import json
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer

DEFAULT_CLIENT_PASSWORD = "mmitss123"

def main():
    # Read the configuration file
    with open("/nojournal/bin/v2x-data-ftp-server-config.json") as configFile:
        config = json.load(configFile)

    clients = config["Clients"]

    # Instantiate a dummy authorizer for managing 'virtual' users
    authorizer = DummyAuthorizer()

    for client in clients:
        add_client(client["Name"], client["LocalDirectory"], authorizer)


    # Instantiate FTP handler class
    handler = FTPHandler
    handler.authorizer = authorizer

    # Instantiate FTP server class to listen at specified address
    address = (config["FtpHost"], config["FtpPort"])
    server = FTPServer(address, handler)

    # set a limit for connections
    server.max_cons = 256
    server.max_cons_per_ip = 5

    # start ftp server
    server.serve_forever()

def add_client(name:str, localDirectory:str, authorizer:DummyAuthorizer):
    create_client_directory(localDirectory, "spat")
    create_client_directory(localDirectory, "remoteBsm")
    create_client_directory(localDirectory, "srm")
    create_client_directory(localDirectory, "ssm")
    create_client_directory(localDirectory, "msgCount")

    # Add clients as authorised users:
    authorizer.add_user(username=name,
                        password=DEFAULT_CLIENT_PASSWORD,
                        homedir=localDirectory, 
                        perm='elradfmwMT')

def create_client_directory(localDirectory:str, dataType:str):
    if not os.path.exists((localDirectory + "/" + dataType)):
        os.makedirs((localDirectory + "/" + dataType))

if __name__ == "__main__":
    main()

