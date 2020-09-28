import os
import json
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer

# Read the configuration file
with open("../config/v2x-data-ftp-server-config.json") as configFile:
    config = json.load(configFile)

clients = config["clients"]

# Instantiate a dummy authorizer for managing 'virtual' users
authorizer = DummyAuthorizer()

for client in clients:

    # Create root directory for each client, if it does not exist::
    if not os.path.exists(client["directory"]):
        os.makedirs(client["directory"])

    # Add clients as authorised users:
    authorizer.add_user(username=client["name"],
                        password='mmitss123',
                        homedir=client["directory"], 
                        perm='elradfmwMT')



# Instantiate FTP handler class
    handler = FTPHandler
    handler.authorizer = authorizer


# Instantiate FTP server class to listen at specified address
address = (config["ftpHost"], config["ftpPort"])
server = FTPServer(address, handler)

# set a limit for connections
server.max_cons = 256
server.max_cons_per_ip = 5

# start ftp server
server.serve_forever()

