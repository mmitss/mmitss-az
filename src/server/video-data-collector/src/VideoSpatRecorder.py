import cv2
import time
import datetime
import os
import json

class VideoSpatRecorder:
    def __init__(self, spatFileName:str, leftPhase:int, throughPhase:int):
        self.spatFileName = spatFileName
        self.spatFileUpdateTime = 0

        self.leftPhase = leftPhase
        self.throughPhase = throughPhase

        self.currentState_through = 0
        self.currentState_left = 0

    def drawBlankSignalHeads(self,frame):
        frame = cv2.rectangle(frame, (0,0), (50,230), (0,0,0), -1)
        frame = cv2.circle(frame, (25,25), 22, (30,30,30), 2)
        frame = cv2.circle(frame, (25,70), 22, (30,30,30), 2)
        frame = cv2.circle(frame, (25,115), 22, (30,30,30), 2)
        frame = cv2.circle(frame, (25,160), 22, (30,30,30), 2)
        frame = cv2.circle(frame, (25,205), 22, (30,30,30), 2)


    def fillSignalHeads(self,frame):

        if self.spatFileUpdateTime != time.ctime(os.stat(self.spatFileName)[8]):
            spatFile = open(self.spatFileName, 'r')
            spatJson = json.load(spatFile)
            spatFile.close()
            self.currentState_through = spatJson["SignalStatus"][self.throughPhase-1]
            self.currentState_left = spatJson["SignalStatus"][self.leftPhase-1]
        
        # Draw status of through phase:
        if self.currentState_through == "red": 
            color = (0,0,255)
            frame = cv2.circle(frame, (25,25), 22, color, -1)
        elif self.currentState_through == "yellow": 
            color  = (0,255,255)
            frame = cv2.circle(frame, (25,70), 22, color, -1)
        elif self.currentState_through == "green": 
            color = (0,255,0)
            frame = cv2.circle(frame, (25,115), 22, color, -1)

        # Draw status of left turn phase:
        if self.currentState_left == "red": 
            color = (0,0,255)
            frame = cv2.arrowedLine(frame, (40,160), (10,160), color, 5, tipLength=0.5)
        elif self.currentState_left == "yellow": 
            color  = (0,255,255)
            frame = cv2.arrowedLine(frame, (40,160), (10,160), color, 5, tipLength=0.5)
        elif self.currentState_left == "green": 
            color = (0,255,0)
            frame = cv2.arrowedLine(frame, (40,160), (10,160), color, 5, tipLength=0.5)
        elif self.currentState_left == "permissive_yellow": 
            color = (0,255,255)
            if time.time() % 2 < 1.5 :
                frame = cv2.arrowedLine(frame, (40,160), (10,160), color, 5, tipLength=0.5)
    
    def overlayTimestamp(self, frame):
        font = cv2.FONT_HERSHEY_SIMPLEX
        timestamp = str(datetime.datetime.now())
        frame = cv2.putText(frame, timestamp, (10, 100), font, 1,(0, 255, 255), 2, cv2.LINE_AA)

    def getMetadataJson(self):
        metadataJson = {
            "Timestamp_verbose": str(datetime.datetime.now()),
            "Timestamp_posix": time.time(),
            "PhaseStatus":
            {
                "Through": self.currentState_through,
                "LeftTurn": self.currentState_left
            }
        }
        return metadataJson

if __name__ == "__main__":
    vsr = VideoSpatRecorder("test/DaisyMountain_GavilanPeak.json", 1,6)
    im = cv2.imread("test/sample_picture.jpg")
    vsr.drawBlankSignalHeads(im)
    vsr.fillSignalHeads(im)
    vsr.overlayTimestamp(im)
    cv2.imwrite("sample_spat.jpg", im)