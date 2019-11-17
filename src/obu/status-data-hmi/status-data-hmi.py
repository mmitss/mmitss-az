'''
**********************************************************************************
  2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  status-data-hmi.py
  Created by: Sherilyn Keaton
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department
  
  Operational Description:

  This application does the following tasks:
  1. Receives and parses a JSON object from the status-data-controller
  2. Displays the current speed and location of the vehicle
  3. Displays the current available maps
  4. When the vehicle is inside the work zone, displays the current work zone lane status and speed limit
'''

# command line argument processing
import argparse

# Performance Testing
import csv

# for performance time tracking
import time;

# capture time for performance testing
class perfTest():
    
    time_sent = time.time()         # message sent from controller
    time_received = time.time()     # message received by application
    time_refreshed = time.time()    # updates displayed by application

# UI
from tkinter import *
import tkinter.ttk as ttk

# directory manipulation
import os 

# for the data comm
import socket
import json

##############################################
#   RECEIVE DATA
##############################################
def get_data():
    # receive the JSON data from the controller
    # set local data to be refreshed
    # set dyanamic text variables for labels
    receivedData, addr = s.recvfrom(2048)
    interfaceJson = json.loads(receivedData.decode())
    secMark = int(interfaceJson["controller_hmi_interface"]["BasicVehicle"]["secMark_Second"])
    latitude_DecimalDegree= float(interfaceJson["controller_hmi_interface"]["BasicVehicle"]["position"]["latitude_DecimalDegree"])
    longitude_DecimalDegree= float(interfaceJson["controller_hmi_interface"]["BasicVehicle"]["position"]["longitude_DecimalDegree"])
    elevation_Meter= float(interfaceJson["controller_hmi_interface"]["BasicVehicle"]["position"]["elevation_Meter"])
    heading_Degree= float(interfaceJson["controller_hmi_interface"]["BasicVehicle"]["heading_Degree"])
    speed_MeterPerSecond= float(interfaceJson["controller_hmi_interface"]["BasicVehicle"]["speed_MeterPerSecond"])
    speed_mph= int(interfaceJson["controller_hmi_interface"]["BasicVehicle"]["speed_mph"])
    vehicleType= int(interfaceJson["controller_hmi_interface"]["BasicVehicle"]["vehicleType"])
    vehicleLane= (interfaceJson["controller_hmi_interface"]["BasicVehicle"]["lane"])

    if vehicleLane == 0: # vehicle is not in work zone
        # reallocate memory for lane display
        reallocatelaneDisplay() 
        workers_present_value.set('                ')
        set_speed_limit(0)
        set_speed_warning_level(0, int(speed_mph))
        monitor_gui.reallocation_counter = 0
    else:
         # vehicle is in work zone
        statusOfLanes = []

        statusOfLanes = (interfaceJson["controller_hmi_interface"]["lanes"])

        laneList = []

        numLanes = len(statusOfLanes)

        for i in range(0, numLanes):
            # get info
            ThisLaneKey = 'Lane ' + str(i)
            laneList.append(LaneStatus(ThisLaneKey))
            laneList[i].laneKey = ThisLaneKey
            laneList[i].laneSpeedLimit_mph = statusOfLanes[i][ThisLaneKey]['laneSpeedLimit_mph']
            laneList[i].laneSpeedChangeDistance = float(statusOfLanes[i][ThisLaneKey]['speedLimitDistance'])
            laneList[i].laneClosed = statusOfLanes[i][ThisLaneKey]['laneClosed']
            laneList[i].laneClosedDistance = float(statusOfLanes[i][ThisLaneKey]['laneClosedDistance'])
            laneList[i].taperRight = bool(statusOfLanes[i][ThisLaneKey]['taperRight'])
            laneList[i].taperLeft = bool(statusOfLanes[i][ThisLaneKey]['taperLeft'])
 
             # set values that are common for lanes
            if i == 0:
                workers_present = bool(statusOfLanes[i][ThisLaneKey]['workersPresent'])
                if workers_present == True:
                    workers_present_value.set('Workers Present')
                else:
                    workers_present_value.set('                ')

                set_speed_limit(int(laneList[i].laneSpeedLimit_mph))
                set_speed_warning_level(int(laneList[i].laneSpeedLimit_mph), int(speed_mph))
                distance = round(laneList[i].laneSpeedChangeDistance)
                if distance > 5:
                    speedDist_value.set(str(distance) + ' ft')
                else:
                    speedDist_value.set('       ')

            # display status
            update_lane_display(laneList[i], i+1, vehicleLane, numLanes)

    # get map info
    activeMap = interfaceJson["controller_hmi_interface"]["mapCache"]["activeMap"]
    availableMaps = interfaceJson["controller_hmi_interface"]["mapCache"]["availableMaps"]
    
    # vehicle current speed and position
    latString = "{:.7f}".format(latitude_DecimalDegree) # want 7 digits showing to right of decimal
    lat_value.set("Latitude: " + latString)
    longString = "{:.7f}".format(longitude_DecimalDegree) # want 7 digits showing to right of decimal
    long_value.set("Longitude: " + longString)
    elevation_value.set("Elevation: " + str(elevation_Meter))
    heading_value.set("Heading: " + str(heading_Degree))
    lane_value.set("Lane: " + str(vehicleLane))
    # set speed limit display variables
    speed_value.set(str(speed_mph))
    mph_value.set('mph')       
        

    # work zone (WZ) messages
    active_message_value.set(str(activeMap["descriptiveName"]))
    # get list of available maps
    message1_descriptive_name = availableMaps["availableMaps"][0]["descriptiveName"]
    message2_descriptive_name = availableMaps["availableMaps"][1]["descriptiveName"]
    message3_descriptive_name = availableMaps["availableMaps"][2]["descriptiveName"]
    message4_descriptive_name = availableMaps["availableMaps"][3]["descriptiveName"]
	
    # set the correct display values for each available map
    if (message1_descriptive_name == ''):
        received_message1_value.set('Unavailable')
    else:
        received_message1_value.set(message1_descriptive_name)
    if (message2_descriptive_name == ''):
        received_message2_value.set('Unavailable')
    else:
        received_message2_value.set(message2_descriptive_name)
    if (message1_descriptive_name == ''):
        received_message3_value.set('Unavailable')
    else:
        received_message3_value.set(message3_descriptive_name)
    if (message1_descriptive_name == ''):
        received_message4_value.set('Unavailable')
    else:
        received_message4_value.set(message4_descriptive_name)
	

    # adjust manual updates (graphics)
 
    # performance test time that HMI receives message
    if args.perf:
        perfTest.time_received = time.time()

##############################################
#   APPLICATION STATUS DISPLAY
##############################################

def set_display_fonts_and_colors():
    # background colors
    monitor_gui.appBackground = 'black'
    monitor_gui.statusDisplayBackground = 'black'
    monitor_gui.statusPanelBackground = 'gray5'
    monitor_gui.statusDisplayBackground = 'gray10'
    monitor_gui.textForeground = 'gray90'

    # fonts
    monitor_gui.smallFont=("Helvetica", 10)
    monitor_gui.mediumFont=("Helvetica", 15)
    monitor_gui.largeFont=("Helvetica", 20)
    monitor_gui.hugeFont=("Helvetica", 25)


##############################################
#   APPLICATION DYNAMIC LABEL VARIABLES
##############################################
def set_dynamic_variables():
    # initialize textvariables for dynamic updates
    monitor_gui.on_map_value = StringVar()
    monitor_gui.on_map_value.set('Not On Map')
    monitor_gui.priority_request_value = StringVar()
    monitor_gui.priority_request_value.set('Priority Request Not Active')
    monitor_gui.speed_value = StringVar()
    monitor_gui.speed_value.set('Speed:')
    monitor_gui.lat_value = StringVar()
    monitor_gui.lat_value.set('Latitude:')
    monitor_gui.long_value = StringVar()
    monitor_gui.long_value.set('Longitude:')
    monitor_gui.elevation_value = StringVar()
    monitor_gui.elevation_value.set('Elevation:')
    monitor_gui.heading_value = StringVar()
    monitor_gui.heading_value.set('Heading:')
    monitor_gui.active_message_value = StringVar()
    monitor_gui.active_message_value.set('Unavailable')
    monitor_gui.received_message1_value = StringVar()
    monitor_gui.received_message1_value.set('Unavailable')
    monitor_gui.received_message2_value = StringVar()
    monitor_gui.received_message2_value.set('Unavailable')
    monitor_gui.received_message3_value = StringVar()
    monitor_gui.received_message3_value.set('Unavailable')
    monitor_gui.received_message4_value = StringVar()
    monitor_gui.received_message4_value.set('Unavailable')

##############################################
#   APPLICATION STATIC GRAPHICS
##############################################

def load_static_graphics():
    # load all static graphics at start up
 
    # signal status icon (red / yellow / green)
    monitor_gui.signal_red = PhotoImage(file = directory_path + "/images/SignalHead.png") 
    monitor_gui.signal_red = monitor_gui.signal_red.subsample(5,5) 
 
    # ev icon dark means ev not present
    monitor_gui.ev_dark = PhotoImage(file = directory_path + "/images/ev_dark.png") 
    monitor_gui.ev_dark = monitor_gui.ev_dark.subsample(2,2) 

   # ev icon means ev present
    monitor_gui.ev = PhotoImage(file = directory_path + "/images/EV.png") 
    monitor_gui.ev = monitor_gui.ev.subsample(2,2) 
 
    # school zone icon dark means school zone not present
    monitor_gui.school_zone_dark = PhotoImage(file = directory_path + "/images/school_zone_dark.png") 
    monitor_gui.school_zone_dark = monitor_gui.school_zone_dark.subsample(3,3) 

   # school zone icon means school zone present
    monitor_gui.school_zone = PhotoImage(file = directory_path + "/images/school_zone.png") 
    monitor_gui.school_zone = monitor_gui.school_zone.subsample(3,3) 
 
##############################################
#   SET UP PERFORMANCE TEST OUTPUT FILE
##############################################

def initialize_perf_output_file():
    # Performance Testing
    # set up a new Excel Workbook and write the send (from data source) and update (after the HMI display is refreshed) times

    perfTest()
    perfTest.hmi_perf_file = open('hmi_perf_file.csv', mode='w', newline='')
    perfTest.perf_writer = csv.writer(perfTest.hmi_perf_file)
    perfTest.perf_writer.writerow(['Message Sent', 'Message Received', 'Display Updated'])



##############################################
#   VEHICLE SPEED / POSITION DISPLAY
##############################################

def set_speed_warning_level(speedLimit_mph, speed_mph):
    current_speed_label.config(bg=statusDisplayBackground)
    if speedLimit_mph > 0:
        warning_level = speed_mph - speedLimit_mph
        if warning_level > 5:
            # red level warning
            current_speed_label.config(fg='red', padx=5, pady=5)	
        elif warning_level > 0:
            # yellow level warning
            current_speed_label.config(fg='yellow', padx=5, pady=5)
        else:
            # zero level warning
            current_speed_label.config(bg=statusDisplayBackground, fg=textForeground)
    else: # not in work zone
        # zero level warning
        current_speed_label.config(bg=statusDisplayBackground, fg=textForeground)

'''
##############################################
#   MAP MESSAGES DISPLAY
##############################################

# message status frame for work zone message status
message_status_frame = LabelFrame(monitor_gui.status_update_frame, font=mediumFont, text = "Work Zone Messages", fg=textForeground, bg=statusPanelBackground)
message_status_frame.grid(row=2, column=1, padx=10, pady=10, sticky=N+E+W)

# WZ Active Message Frame (Top Right) and its contents
active_message_frame = LabelFrame(message_status_frame, font=mediumFont, relief=FLAT, fg=textForeground, bg=statusPanelBackground)
active_message_frame.grid(row=0, column=0, padx=10, pady=10, sticky=N)

# active message label
Label(active_message_frame, text="     Active     ", font=mediumFont, foreground="green", bg=statusPanelBackground).grid(row=0, column=0, padx=100, pady=2)
# active message content
Label(active_message_frame, textvariable=active_message_value, font=mediumFont, foreground="green", bg=statusPanelBackground).grid(row=1, column=0, padx=50, pady=2)

# WZ Received Message Frame (Top Right) and its contents
received_message_frame = LabelFrame(message_status_frame, font=mediumFont, relief=FLAT, fg=textForeground, bg=statusPanelBackground)
received_message_frame.grid(row=0, column=1, padx=10, pady=10, sticky=N)

# received message(s) label
Label(received_message_frame, text="    Received    ", font=mediumFont, foreground="RoyalBlue1", bg=statusPanelBackground).grid(row=2, column=0, padx=100, pady=2)
# received message content
Label(received_message_frame, textvariable=received_message1_value, font=mediumFont, foreground="RoyalBlue1", bg=statusPanelBackground).grid(row=3, column=0, padx=50, pady=2)
Label(received_message_frame, textvariable=received_message2_value, font=mediumFont, foreground="RoyalBlue1", bg=statusPanelBackground).grid(row=4, column=0, padx=50, pady=2)
Label(received_message_frame, textvariable=received_message3_value, font=mediumFont, foreground="RoyalBlue1", bg=statusPanelBackground).grid(row=5, column=0, padx=50, pady=2)
Label(received_message_frame, textvariable=received_message4_value, font=mediumFont, foreground="RoyalBlue1", bg=statusPanelBackground).grid(row=6, column=0, padx=100, pady=2)





'''

##############################################
#   VEHICLE SPEED / POSITION DISPLAY
##############################################

def build_BSM_tree():
    monitor_gui.bsm_tree = ttk.Treeview(monitor_gui.BSM)
    monitor_gui.bsm_tree["columns"]=("Vehicle ID", "Vehicle Type")
    monitor_gui.bsm_tree.column("#0", width=100)
    monitor_gui.bsm_tree.column("Vehicle ID", width=100)
    monitor_gui.bsm_tree.column("Vehicle Type", width=100) 
    monitor_gui.bsm_tree.heading("Vehicle ID", text="Vehicle ID") 
    monitor_gui.bsm_tree.heading("Vehicle Type", text="Vehicle Type") 
    '''
    monitor_gui.bsm_tree.insert('',1,'', text='2', values=('Transit'), tags=('odd'))
    monitor_gui.bsm_tree.insert('',1,'', text='3', values=('Fleet'), tags=('even'))   
    monitor_gui.bsm_tree.insert('',1,'', text='4', values=('Transit'), tags=('odd'))
    '''
    monitor_gui.bsm_tree.grid(row=0, column=0, sticky=E+W)
 
    # tag styles
    monitor_gui.bsm_tree.tag_configure('odd', background='#e8e8e8')
    monitor_gui.bsm_tree.tag_configure('odd', background='#dfdfdf')

##############################################
#  STATUS WIDGET INITIAL DISPLAY
##############################################
def create_status_widgets():
    # initialize textvariables for dynamic updates
    set_dynamic_variables()
    
    # load the static graphics
    load_static_graphics()

    # create the frame that holds all dynamic data (everything except the static footer)
    
    ### west (left side) ###
    monitor_gui.status_west = Frame(monitor_gui, relief=FLAT, bd=1, bg=monitor_gui.statusPanelBackground)
    monitor_gui.status_west.grid(row=0, column=0, columnspan=3, rowspan=3, padx=5, pady=5)

    # SPaT Data
    monitor_gui.SPaT = LabelFrame(monitor_gui, relief=RIDGE, bd=1, bg=monitor_gui.statusPanelBackground, font=monitor_gui.mediumFont, text="SPaT Data", fg=monitor_gui.textForeground)
    monitor_gui.SPaT.grid(row=0, column=0, columnspan=1, rowspan=2, padx=10, pady=10)

    # Signal Data
    monitor_gui.Signal = Label(monitor_gui.SPaT, image=monitor_gui.signal_red, relief=FLAT, bd=1, bg=monitor_gui.statusPanelBackground)
    monitor_gui.Signal.grid(row=0, column=0, columnspan=1, rowspan=2, padx=10, pady=10)

    # Phase Data
    monitor_gui.Phase = Frame(monitor_gui.SPaT, relief=FLAT, bd=1, bg=monitor_gui.statusPanelBackground)
    monitor_gui.Phase.grid(row=2, column=0, columnspan=1, rowspan=2, padx=10, pady=10)

    # Vehicle Position Data
    monitor_gui.BasicVehicle = LabelFrame(monitor_gui, relief=RIDGE, bd=1, bg=monitor_gui.statusPanelBackground, text="Host Vehicle Data", font=monitor_gui.mediumFont, fg=monitor_gui.textForeground)
    monitor_gui.BasicVehicle.grid(row=4, column=0, columnspan=1, rowspan=2, padx=10, pady=10)

    Label(monitor_gui.BasicVehicle, textvariable=monitor_gui.speed_value, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusPanelBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2)
    Label(monitor_gui.BasicVehicle, textvariable=monitor_gui.lat_value, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusPanelBackground, justify=LEFT).grid(row=1, column=0, padx=5, pady=2)
    Label(monitor_gui.BasicVehicle, textvariable=monitor_gui.long_value, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusPanelBackground, justify=LEFT).grid(row=2, column=0, padx=5, pady=2)
    Label(monitor_gui.BasicVehicle, textvariable=monitor_gui.elevation_value, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusPanelBackground, justify=LEFT).grid(row=3, column=0, padx=5, pady=2)
    Label(monitor_gui.BasicVehicle, textvariable=monitor_gui.heading_value, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusPanelBackground, justify=LEFT).grid(row=4, column=0, padx=5, pady=2)
    #Label(monitor_gui.BasicVehicle, textvariable=lane_value, font=mediumFont, fg=textForeground, bg=statusPanelBackground, justify=LEFT).grid(row=5, column=0, padx=5, pady=2)

    ### east (right side) ###
    monitor_gui.status_east = Frame(monitor_gui, relief=FLAT, bd=1, bg=monitor_gui.statusPanelBackground)
    monitor_gui.status_east.grid(row=0, column=1, columnspan=3, rowspan=4, padx=5, pady=5)

    # Map Status
    monitor_gui.Map = LabelFrame(monitor_gui, relief=RIDGE, bd=1, bg=monitor_gui.statusPanelBackground, font=monitor_gui.mediumFont, text="MAP Status", fg=monitor_gui.textForeground)
    monitor_gui.Map.grid(row=0, column=1, columnspan=1, rowspan=2, padx=10, pady=10, sticky=W)

    # Map Status text
    monitor_gui.map_label = Label(monitor_gui.Map, textvariable=monitor_gui.on_map_value, relief=FLAT, bd=1, bg=monitor_gui.statusPanelBackground, font=monitor_gui.mediumFont, text="Map Status", fg=monitor_gui.textForeground)
    monitor_gui.map_label.grid(row=0, column=0, columnspan=1, rowspan=2, padx=10, pady=10, sticky=E+W)
    #Label(monitor_gui.Map, textvariable=monitor_gui.speed_value, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2)
    # priority request status
    Label(monitor_gui.Map, textvariable=monitor_gui.priority_request_value, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusDisplayBackground, justify=LEFT).grid(row=2, column=0, padx=5, pady=2)

    # ART
    monitor_gui.ART = LabelFrame(monitor_gui, relief=RIDGE, bd=1, bg=monitor_gui.statusPanelBackground, font=monitor_gui.mediumFont, text="ART", fg=monitor_gui.textForeground)
    monitor_gui.ART.grid(row=2, column=1, columnspan=1, rowspan=2, padx=10, pady=10, sticky=W)
    Label(monitor_gui.ART, text="PLACEHOLDER", font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=E+W)

    # Basic Vehicle Messages
    monitor_gui.BSM = LabelFrame(monitor_gui, relief=RIDGE, bd=1, bg=monitor_gui.statusPanelBackground, font=monitor_gui.mediumFont, text="Remote Vehicles: Basic Vehicle Data", fg=monitor_gui.textForeground)
    monitor_gui.BSM.grid(row=4, column=1, columnspan=1, rowspan=2, padx=10, pady=10, sticky=W)
    Label(monitor_gui.BSM, text="No Vehicle Data", font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=E+W)

    # Bottom Row (EV, School Zone, and Available Maps)
    monitor_gui.Multi = Frame(monitor_gui, relief=RAISED, bd=1, bg=monitor_gui.statusPanelBackground)
    monitor_gui.Multi.grid(row=6, column=1, columnspan=1, rowspan=2, padx=10, pady=10, sticky=W)
 
    # EV
    monitor_gui.EV = Frame(monitor_gui.Multi, relief=FLAT, bd=1, bg=monitor_gui.statusPanelBackground)
    monitor_gui.EV.grid(row=0, column=0, padx=10, pady=10, sticky=W)
    Label(monitor_gui.EV, image=monitor_gui.ev_dark, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)

    # School Zone
    monitor_gui.SchoolZone = Frame(monitor_gui.Multi, relief=FLAT, bd=1, bg=monitor_gui.statusPanelBackground)
    monitor_gui.SchoolZone.grid(row=0, column=1, padx=10, pady=10, sticky=W)
    Label(monitor_gui.SchoolZone, image=monitor_gui.school_zone_dark, font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusPanelBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)

    # Available Maps
    monitor_gui.AvailableMaps = LabelFrame(monitor_gui.Multi, relief=FLAT, bd=1, bg=monitor_gui.statusDisplayBackground, font=monitor_gui.mediumFont, text="Available Maps", fg=monitor_gui.textForeground)
    monitor_gui.AvailableMaps.grid(row=0, column=2, padx=10, pady=10, sticky=W)
    Label(monitor_gui.AvailableMaps, text="placeholder", font=monitor_gui.mediumFont, fg=monitor_gui.textForeground, bg=monitor_gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)

##############################################
#   APPLICATION FOOTER
##############################################

def create_app_footer():

    # app logo
    monitor_gui.app_logo = PhotoImage(file = directory_path + "/images/vehicle-status-display-logo.png") 
    monitor_gui.app_logo = monitor_gui.app_logo.subsample(1,1) 

    # create the footer, add , and place it at the bottom of the application
    logo_label = Label(monitor_gui, image = monitor_gui.app_logo, relief=RIDGE, bd=6, bg='white')   
    logo_label.grid(row=15, column=0, columnspan=4, padx = 20, pady = 20, sticky=S+E+W) 

##############################################
#   UPDATE (REFRESH) DISPLAY
##############################################

def update_display():
    # refresh data

    # read JSON
    #get_data()
	
    # build the treeview containing BSMs
    build_BSM_tree()

    # refresh dynamic data labels that were set to type class StringVar
    # class StringVar updates dynamically with root.update_idletasks()
    monitor_gui.update_idletasks()
    
    # Performance Testing
    if args.perf:
        perfTest.time_refreshed = time.time()
        # write the row for perf test
        perfTest.perf_writer.writerow([perfTest.time_sent, perfTest.time_received, perfTest.time_refreshed])	
    
    # reset refresh timer
    monitor_gui.after(800, update_display)

##############################################
#   MONITOR GUI (APPLICATION) ON CLOSING
##############################################

def on_closing():
    # save and close the performance test measure file
    if args.perf:
        perfTest.hmi_perf_file.close()
    
    # kill the socket
    s.close()
    monitor_gui.destroy()
'''
class MonitorGUI:
    def __init__(self, master):
        self.master = master
        master.title('Vehicle Status Data Prototype')
'''

##############################################
#   MAIN
##############################################

if __name__ == "__main__":

    # parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-p','--perf', help="Run internal performance test on the HMI.", action="store_true")
    args = parser.parse_args()

    if args.perf:
        initialize_perf_output_file()

    # get current working directory
    directory_path = os.getcwd()

    # set up communications
    hmiIP = '127.0.0.1'
    hmiPort = 20010
    hmi = (hmiIP, hmiPort)

    # Create a socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the created socket to the server information.
    s.bind((hmi))
        # create Monitor GUI
    #    root = Tk()
    #monitor_gui = MonitorGUI(root)
    monitor_gui = Tk()
    set_display_fonts_and_colors()
    monitor_gui.title("Vehicle Status Data Prototype")  
    monitor_gui['bg'] = monitor_gui.appBackground

    # create the application footer with s
    create_app_footer()
    
    # create the frame that holds all dynamic data (everything except the static footer)
    create_status_widgets()

    # set timer for updating data and call the update function
    monitor_gui.after(800, update_display)


    # handle event for closing the application
    monitor_gui.protocol("WM_DELETE_WINDOW", on_closing)
	
    # fire up the Monitor GUI
    monitor_gui.mainloop()

