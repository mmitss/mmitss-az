import json

class V2XDataTransferConfig:
    def __init__(self, configFilePath:str):
        with open(configFilePath, 'r') as configFile:
            config = json.load(configFile)

        self.localDataDirectory = config["local_data_directory"]
        self.intersections = config["intersections"]
        self.dataTransferStart = config["data_transfer_start_24h"]
