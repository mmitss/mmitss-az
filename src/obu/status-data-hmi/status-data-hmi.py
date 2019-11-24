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
    secMark = int(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["secMark_Second"])
    latitude_DecimalDegree= float(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["position"]["latitude_DecimalDegree"])
    longitude_DecimalDegree= float(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["position"]["longitude_DecimalDegree"])
    elevation_Meter= float(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["position"]["elevation_Meter"])
    heading_Degree= float(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["heading_Degree"])
    speed_mph= str(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["speed_mph"])
    temporaryID= str(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["temporaryID"])
    vehicleType= str(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["vehicleType"])
    vehicleLane= (interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["lane"])
    onMap= bool(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["priority"]["OnMAP"])
    requestSent = bool(interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["priority"]["requestSent"])
    #signalGroup= (interfaceJson["mmitss_hmi_interface"]["hostVehicle"]["lane"])

    # get maps
    availableMaps = []
    availableMaps = interfaceJson["mmitss_hmi_interface"]["infrastructure"]["availableMaps"]
    print(len(availableMaps))
    print(availableMaps)

    '''
    for i in range(0, lenL(availableMaps)): # assuming up to 5 maps have been received 
        gui.map_intersectionID = int(data_array[index_maps + 1 + receivedMap*4])
        gui.map_DescriptiveName = data_array[index_maps + 2 + receivedMap*4]
        gui.map_active = bool_map[data_array[index_maps + 3 + receivedMap*4]]
    
    if vehicleLane == 0: # vehicle is not in work zone
        # reallocate memory for lane display
        reallocatelaneDisplay() 
        workers_present_value.set('                ')
        set_speed_limit(0)
        set_speed_warning_level(0, int(speed_mph))
        gui.reallocation_counter = 0
    else:
         # vehicle is in work zone
        statusOfLanes = []

        statusOfLanes = (interfaceJson["mmitss_hmi_interface"]["lanes"])

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
    activeMap = interfaceJson["mmitss_hmi_interface"]["mapCache"]["activeMap"]
    availableMaps = interfaceJson["mmitss_hmi_interface"]["mapCache"]["availableMaps"]
    '''
    # vehicle current speed and position
    gui.latString = "{:.7f}".format(latitude_DecimalDegree) # want 7 digits showing to right of decimal
    gui.lat_value.set("Latitude: " + gui.latString)
    gui.longString = "{:.7f}".format(longitude_DecimalDegree) # want 7 digits showing to right of decimal
    gui.long_value.set("Longitude: " + gui.longString)
    gui.elevation_value.set("Elevation: " + str(elevation_Meter))
    gui.heading_value.set("Heading: " + str(heading_Degree))
    gui.lane_value.set("Lane: " + str(vehicleLane))
    gui.speed_value.set(str("Speed: " + speed_mph + " mph"))
    gui.vehicle_type_value.set(str("Vehicle Type: " + vehicleType))
    gui.temporaryID_value.set(str("Temporary ID: " + temporaryID))
    
    # on map?
    if onMap == True:
        gui.on_map_value.set("    On Map    ")
        gui.map_label.config(bg = 'pale green', fg='black', padx=5, pady=5)	
    else:
        gui.on_map_value.set("  Not On Map  ")
        gui.map_label.config(bg = 'black', fg='pale green', padx=5, pady=5)	

    # priority request sent?
    if requestSent == True:
        gui.priority_request_value.set("Priority Request is ACTIVE")
        gui.priority_label.config(bg='yellow', fg='black', padx=2, pady=5)
    else:
        gui.priority_request_value.set("   No Request Sent   ")
        gui.priority_label.config(bg='black', fg='yellow', padx=2, pady=5)

    # gui.signal_group_value.set(str("Signal Group: " + signalGroup))
        
    # MAP messages
#    active_message_value.set(str(activeMap["descriptiveName"]))
    # get list of available maps
    '''
    message1_descriptive_name = availableMaps["availableMaps"][0]["DescriptiveName"]
    message2_descriptive_name = availableMaps["availableMaps"][1]["DescriptiveName"]
    message3_descriptive_name = availableMaps["availableMaps"][2]["DescriptiveName"]
    message4_descriptive_name = availableMaps["availableMaps"][3]["DescriptiveName"]
    message5_descriptive_name = availableMaps["availableMaps"][4]["DescriptiveName"]
	
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
    if (message1_descriptive_name == ''):
        received_message5_value.set('Unavailable')
    else:
        received_message5_value.set(message4_descriptive_name)
	'''

    # adjust manual updates (graphics)
 
    # performance test time that HMI receives message
    if args.perf:
        perfTest.time_received = time.time()

##############################################
#   APPLICATION STATUS DISPLAY
##############################################

def set_display_fonts_and_colors():
    # background colors
    gui.appBackground = 'black'
    gui.statusDisplayBackground = 'black'
    gui.statusPanelBackground = 'gray5'
    gui.statusDisplayBackground = 'gray10'
    gui.textForeground = 'gray90'
    gui.onMapBackground = gui.statusDisplayBackground
    gui.requestSentBackground = gui.statusDisplayBackground
    gui.onMapForeground = 'pale green'
    gui.requestSentForeground = 'light yellow'

    # fonts
    gui.smallFont=("Helvetica", 10)
    gui.mediumFont=("Helvetica", 15)
    gui.largeFont=("Helvetica", 20)
    gui.hugeFont=("Helvetica", 25)


##############################################
#   APPLICATION DYNAMIC LABEL VARIABLES
##############################################
def set_dynamic_variables():
    # initialize textvariables for dynamic updates
    gui.on_map_value = StringVar()
    gui.on_map_value.set('Not On Map')
    gui.priority_request_value = StringVar()
    gui.priority_request_value.set('Priority Request Not Active')
    gui.speed_value = StringVar()
    gui.speed_value.set('Speed:')
    gui.lat_value = StringVar()
    gui.lat_value.set('Latitude:')
    gui.long_value = StringVar()
    gui.long_value.set('Longitude:')
    gui.elevation_value = StringVar()
    gui.elevation_value.set('Elevation:')
    gui.heading_value = StringVar()
    gui.heading_value.set('Heading:')
    gui.lane_value = StringVar()
    gui.lane_value.set('Lane:')
    gui.temporaryID_value = StringVar()
    gui.temporaryID_value.set('Temporary ID:')
    gui.vehicle_type_value = StringVar()
    gui.vehicle_type_value.set('Vehicle Type:')
    gui.active_message_value = StringVar()
    gui.active_message_value.set('Unavailable')
    gui.received_message1_value = StringVar()
    gui.received_message1_value.set('Unavailable')
    gui.received_message2_value = StringVar()
    gui.received_message2_value.set('Unavailable')
    gui.received_message3_value = StringVar()
    gui.received_message3_value.set('Unavailable')
    gui.received_message4_value = StringVar()
    gui.received_message4_value.set('Unavailable')
    gui.received_message5_value = StringVar()
    gui.received_message5_value.set('Unavailable')

##############################################
#   APPLICATION STATIC GRAPHICS
##############################################

def load_static_graphics():
    # load all static graphics at start up
 
    # signal status icon (red / yellow / green)
    gui.signal_red = PhotoImage(file = directory_path + "/images/SignalHeadRed.png") 
    gui.signal_red = gui.signal_red.subsample(4,4) 
 
    gui.signal_yellow = PhotoImage(file = directory_path + "/images/SignalHeadYellow.png") 
    gui.signal_yellow = gui.signal_red.subsample(5,5) 

    gui.signal_green = PhotoImage(file = directory_path + "/images/SignalHeadGreen.png") 
    gui.signal_greem = gui.signal_red.subsample(5,5) 

    # ev icon dark means ev not present
    gui.ev_dark = PhotoImage(file = directory_path + "/images/ev_dark.png") 
    gui.ev_dark = gui.ev_dark.subsample(2,2) 

   # ev icon means ev present
    gui.ev = PhotoImage(file = directory_path + "/images/EV.png") 
    gui.ev = gui.ev.subsample(2,2) 
 
    # school zone icon dark means school zone not present
    gui.school_zone_dark = PhotoImage(file = directory_path + "/images/school_zone_dark.png") 
    gui.school_zone_dark = gui.school_zone_dark.subsample(3,3) 

   # school zone icon means school zone present
    gui.school_zone = PhotoImage(file = directory_path + "/images/school_zone.png") 
    gui.school_zone = gui.school_zone.subsample(3,3) 
 
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
#   8-PHASE STATUS DISPLAY
##############################################

def build_phase_tree():
    gui.phase_tree = ttk.Treeview(gui.Phase, selectmode='none')
    gui.phase_tree["columns"]=("1", "2", "3", "4", "5", "6", "7", "8")
    gui.phase_tree.column("#0", width=100)
    gui.phase_tree.column("1", width=30)
    gui.phase_tree.column("2", width=30)
    gui.phase_tree.column("3", width=30) 
    gui.phase_tree.column("4", width=30)
    gui.phase_tree.column("5", width=30)
    gui.phase_tree.column("6", width=30) 
    gui.phase_tree.column("7", width=30)
    gui.phase_tree.column("8", width=30)
    gui.phase_tree.heading('#0', text='Phases') 
    gui.phase_tree.heading('1', text='1') 
    gui.phase_tree.heading("2", text="2") 
    gui.phase_tree.heading("3", text="3") 
    gui.phase_tree.heading("4", text="4") 
    gui.phase_tree.heading("5", text="5") 
    gui.phase_tree.heading("6", text="6") 
    gui.phase_tree.heading("7", text="7") 
    gui.phase_tree.heading("8", text="8") 

    gui.phase_tree.grid(row=1, column=0, rowspan=3, sticky=E+W)

    # set style
    style = ttk.Style()
    style.configure("mystyle.Treeview", highlightthickness=0, bd=0, font=gui.smallFont) # Modify the font of the body
    style.configure("mystyle.Treeview.Heading", font=gui.smallFont) # Modify the font of the headings
    #style.layout("mystyle.Treeview", [('mystyle.Treeview.treearea', {'sticky': 'nswe'})]) # Remove the borders
    
    gui.phase_tree.insert('', 'end', iid='Signal', text='Signal', values=('R', 'R', 'R', 'G', 'R', 'R', '', 'R'), tags=('odd'))
    gui.phase_tree.insert('', 'end', iid='Ped', text='Ped', values=('-', 'DW', '-', 'W', '-', 'DW', '-', 'DW'), tags=('even'))
    
    # tag styles
    #gui.phase_tree.tag_configure('odd', background='#e8e8e8')
    #gui.phase_tree.tag_configure('odd', background='#dfdfdf')


##############################################
#   ACTIVE REQUEST DISPLAY
##############################################

def build_ART_tree():
    gui.ART_tree = ttk.Treeview(gui.ART, selectmode='none')
    gui.ART_tree["columns"]=("RequestID", "VehicleID", "BasicVehicleRole", "PriorityRequestStatus", "MessageCount", "InBoundLane", "VehicleETA", "VehicleDuration")
    gui.ART_tree.column("#0", width=1)
    gui.ART_tree.column("RequestID", width=100)
    gui.ART_tree.column("VehicleID", width=100)
    gui.ART_tree.column("BasicVehicleRole", width=100) 
    gui.ART_tree.column("PriorityRequestStatus", width=100)
    gui.ART_tree.column("MessageCount", width=100)
    gui.ART_tree.column("InBoundLane", width=100) 
    gui.ART_tree.column("VehicleETA", width=100)
    gui.ART_tree.column("VehicleDuration", width=100)
    gui.ART_tree.heading('RequestID', text='Request ID') 
    gui.ART_tree.heading("VehicleID", text="Vehicle ID") 
    gui.ART_tree.heading("BasicVehicleRole", text="Basic Vehicle Role") 
    gui.ART_tree.heading("PriorityRequestStatus", text="PriorityRequestStatus") 
    gui.ART_tree.heading("MessageCount", text="MessageCount") 
    gui.ART_tree.heading("InBoundLane", text="In Bound Lane") 
    gui.ART_tree.heading("VehicleETA", text="Vehicle ETA") 
    gui.ART_tree.heading("VehicleDuration", text="Vehicle Duration") 
    #gui.ART_tree.heading("Vehicle Type", text="Vehicle Type") 

    # set style
    style = ttk.Style()
    style.configure("mystyle.Treeview", highlightthickness=0, bd=0, font=gui.smallFont) # Modify the font of the body
    style.configure("mystyle.Treeview.Heading", font=gui.smallFont) # Modify the font of the headings
    style.layout("mystyle.Treeview", [('mystyle.Treeview.treearea', {'sticky': 'nswe'})]) # Remove the borders
    
    
    gui.ART_tree.insert('', 1, '36500', text='2', values=('transit'), tags=('odd'))
    #gui.ART_tree.TreeAdd('', 1, '36500', text='3', values=('truck'), tags=('even'))   
    #gui.ART_tree.insert('', 1, '36500', text='4', values=('passenger'), tags=('odd'))
    
    gui.ART_tree.grid(row=1, column=0, sticky=E+W)
    
    gui.ART_tree.tag_configure('odd', background='#e8e8e8')

    # tag styles
    gui.ART_tree.tag_configure('odd', background='#e8e8e8')
    gui.ART_tree.tag_configure('odd', background='#dfdfdf')

##############################################
#   REMOTE BSM DISPLAY
##############################################

def build_BSM_tree():
    gui.bsm_tree = ttk.Treeview(gui.BSM, selectmode='none')
    gui.bsm_tree["columns"]=("Time", "Temp ID", "Vehicle Type", "Latitude", "Longitude", "Elevation", "Heading", "Speed")
    gui.bsm_tree.column("#0", width=1)
    gui.bsm_tree.column("Time", width=100)
    gui.bsm_tree.column("Temp ID", width=100)
    gui.bsm_tree.column("Vehicle Type", width=100) 
    gui.bsm_tree.column("Latitude", width=100)
    gui.bsm_tree.column("Longitude", width=100)
    gui.bsm_tree.column("Elevation", width=100) 
    gui.bsm_tree.column("Heading", width=100)
    gui.bsm_tree.column("Speed", width=100)
    gui.bsm_tree.heading('Time', text='Time') 
    gui.bsm_tree.heading("Temp ID", text="Temp ID") 
    gui.bsm_tree.heading("Vehicle Type", text="Vehicle Type") 
    gui.bsm_tree.heading("Latitude", text="Latitude") 
    gui.bsm_tree.heading("Longitude", text="Longitude") 
    gui.bsm_tree.heading("Elevation", text="Elevation") 
    gui.bsm_tree.heading("Heading", text="Heading") 
    gui.bsm_tree.heading("Speed", text="Speed") 
    #gui.bsm_tree.heading("Vehicle Type", text="Vehicle Type") 

    # set style
    style = ttk.Style()
    style.configure("mystyle.Treeview", highlightthickness=0, bd=0, font=gui.smallFont, foreground="green", background="black") # Modify the font of the body
    style.configure("mystyle.Treeview.Heading", font=gui.smallFont) # Modify the font of the headings
    style.layout("mystyle.Treeview", [('mystyle.Treeview.treearea', {'sticky': 'nswe'})]) # Remove the borders
    
    gui.bsm_tree.insert('', 1, '36500', text='2', values=('transit'), tags=('odd'))
    #gui.bsm_tree.TreeAdd('', 1, '36500', text='3', values=('truck'), tags=('even'))   
    #gui.bsm_tree.insert('', 1, '36500', text='4', values=('passenger'), tags=('odd'))
    
    gui.bsm_tree.grid(row=0, column=0, sticky=E+W)
    
    gui.bsm_tree.tag_configure('odd', background='#e8e8e8')

    # tag styles
    gui.bsm_tree.tag_configure('odd', background='#e8e8e8')
    gui.bsm_tree.tag_configure('odd', background='#dfdfdf')

##############################################
#  STATUS WIDGET INITIAL DISPLAY
##############################################
def create_status_widgets():
    # initialize textvariables for dynamic updates
    set_dynamic_variables()
    
    # load the static graphics
    load_static_graphics()

    # SPaT Data
    gui.SPaT = LabelFrame(gui, relief=RIDGE, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, text="SPaT Data", fg=gui.textForeground)
    gui.SPaT.grid(row=0, column=0, columnspan=1, rowspan=1, padx=10, pady=10)

    # Signal Data
    gui.Signal = Label(gui.SPaT, image=gui.signal_red, relief=FLAT, bd=1, bg=gui.statusPanelBackground)
    gui.Signal.grid(row=0, column=0, columnspan=1, rowspan=1, padx=10, pady=10)

    # Phase Data
    gui.Phase = Frame(gui.SPaT, relief=FLAT, bd=1, bg=gui.statusPanelBackground)
    gui.Phase.grid(row=1, column=0, columnspan=1, rowspan=1, padx=10, pady=10)

    # Vehicle Position Data
    gui.BasicVehicle = LabelFrame(gui, relief=RIDGE, bd=1, bg=gui.statusPanelBackground, text='Host Vehicle', font=gui.mediumFont, fg=gui.textForeground)
    gui.BasicVehicle.grid(row=4, column=0, columnspan=1, rowspan=2, padx=10, pady=10)

    Label(gui.BasicVehicle, textvariable=gui.temporaryID_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2)
    Label(gui.BasicVehicle, textvariable=gui.vehicle_type_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=1, column=0, padx=5, pady=2)
    Label(gui.BasicVehicle, textvariable=gui.speed_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=2, column=0, padx=5, pady=2)
    Label(gui.BasicVehicle, textvariable=gui.lat_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=3, column=0, padx=5, pady=2)
    Label(gui.BasicVehicle, textvariable=gui.long_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=4, column=0, padx=5, pady=2)
    Label(gui.BasicVehicle, textvariable=gui.elevation_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=5, column=0, padx=5, pady=2)
    Label(gui.BasicVehicle, textvariable=gui.heading_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=6, column=0, padx=5, pady=2)
    Label(gui.BasicVehicle, textvariable=gui.lane_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=7, column=0, padx=5, pady=2)

    # Map Status
    gui.Map = LabelFrame(gui, relief=RIDGE, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, text="MAP Status", fg=gui.textForeground)
    gui.Map.grid(row=0, column=1, columnspan=1, rowspan=1, padx=10, pady=10, sticky=W)

    # Map Status
    gui.map_label = Label(gui.Map, textvariable=gui.on_map_value, relief=RIDGE, bd=1, bg=gui.onMapBackground, font=gui.mediumFont, text="Map Status", fg=gui.onMapForeground)
    gui.map_label.grid(row=0, column=0, columnspan=1, padx=10, pady=10, sticky=E+W)
    # priority request status
    gui.priority_label = Label(gui.Map, textvariable=gui.priority_request_value, relief=RIDGE, font=gui.mediumFont, fg=gui.requestSentForeground, bg=gui.requestSentBackground, justify=LEFT)
    gui.priority_label.grid(row=1, column=0, rowspan=1, padx=5, pady=2, sticky=E+W)

    # ART
    gui.ART = LabelFrame(gui, relief=RIDGE, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, text="Active Request Table", fg=gui.textForeground)
    gui.ART.grid(row=2, column=1, columnspan=1, rowspan=2, padx=10, pady=10, sticky=W)
    #Label(gui.ART, text="PLACEHOLDER", font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=E+W)

    # Remote Basic Vehicle Messages
    gui.BSM = LabelFrame(gui, relief=RIDGE, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, text="Remote Vehicles: Basic Vehicle Data", fg=gui.textForeground)
    gui.BSM.grid(row=4, column=1, columnspan=1, rowspan=2, padx=10, pady=10, sticky=W)
    Label(gui.BSM, text="No Vehicle Data", font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=E+W)

    # Bottom Row (EV, School Zone, and Available Maps)
    gui.Multi = Frame(gui, relief=RAISED, bd=1, bg=gui.statusPanelBackground)
    gui.Multi.grid(row=6, column=1, columnspan=1, rowspan=2, padx=10, pady=10, sticky=W)
 
    # EV
    gui.EV = Frame(gui.Multi, relief=FLAT, bd=1, bg=gui.statusPanelBackground)
    gui.EV.grid(row=0, column=0, padx=10, pady=10, sticky=W)
    Label(gui.EV, image=gui.ev_dark, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)

    # School Zone
    gui.SchoolZone = Frame(gui.Multi, relief=FLAT, bd=1, bg=gui.statusPanelBackground)
    gui.SchoolZone.grid(row=0, column=1, padx=10, pady=10, sticky=W)
    Label(gui.SchoolZone, image=gui.school_zone_dark, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)

    # Available Maps
    gui.AvailableMaps = LabelFrame(gui.Multi, relief=FLAT, bd=1, bg=gui.statusDisplayBackground, font=gui.mediumFont, text="Available Maps", fg=gui.textForeground)
    gui.AvailableMaps.grid(row=0, column=2, padx=10, pady=10, sticky=W)
    Label(gui.AvailableMaps, text="placeholder", font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)
    Label(gui.AvailableMaps, textvariable=gui.received_message1_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=0, column=0, padx=50, pady=2)
    Label(gui.AvailableMaps, textvariable=gui.received_message2_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=1, column=0, padx=50, pady=2)
    Label(gui.AvailableMaps, textvariable=gui.received_message3_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=2, column=0, padx=50, pady=2)
    Label(gui.AvailableMaps, textvariable=gui.received_message4_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=3, column=0, padx=100, pady=2)
    Label(gui.AvailableMaps, textvariable=gui.received_message5_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=4, column=0, padx=100, pady=2)

##############################################
#   APPLICATION FOOTER
##############################################

def create_app_footer():

    # app logo
    gui.app_logo = PhotoImage(file = directory_path + "/images/vehicle-status-display-logo.png") 
    gui.app_logo = gui.app_logo.subsample(1,1) 

    # create the footer, add , and place it at the bottom of the application
    logo_label = Label(gui, image = gui.app_logo, relief=RIDGE, bd=6, bg='white')   
    logo_label.grid(row=15, column=0, columnspan=4, padx = 20, pady = 20, sticky=S+E+W) 

##############################################
#   UPDATE (REFRESH) DISPLAY
##############################################

def update_display():
    # refresh data

    # read JSON
    get_data()
	
    # build the treeview containing Phases
    build_phase_tree()

    # build the treeview containing Active Requests
    build_ART_tree()

    # build the treeview containing BSMs
    build_BSM_tree()

    # refresh dynamic data labels that were set to type class StringVar
    # class StringVar updates dynamically with root.update_idletasks()
    gui.update_idletasks()
    
    # Performance Testing
    if args.perf:
        perfTest.time_refreshed = time.time()
        # write the row for perf test
        perfTest.perf_writer.writerow([perfTest.time_sent, perfTest.time_received, perfTest.time_refreshed])	
    
    # reset refresh timer
    gui.after(100, update_display)

##############################################
#   MONITOR GUI (APPLICATION) ON CLOSING
##############################################

def on_closing():
    # save and close the performance test measure file
    if args.perf:
        perfTest.hmi_perf_file.close()
    
    # kill the socket
    s.close()
    gui.destroy()
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
    hmiPort = 5002
    hmi = (hmiIP, hmiPort)

    # Create a socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the created socket to the server information.
    s.bind((hmi))
        # create Monitor GUI
    #    root = Tk()
    #gui = MonitorGUI(root)
    gui = Tk()
    set_display_fonts_and_colors()
    gui.title("Vehicle Status Data Prototype")  
    gui['bg'] = gui.appBackground

    # create the application footer with s
    create_app_footer()
    
    # create the frame that holds all dynamic data (everything except the static footer)
    create_status_widgets()

    # set timer for updating data and call the update function
    gui.after(100, update_display)


    # handle event for closing the application
    gui.protocol("WM_DELETE_WINDOW", on_closing)
	
    # fire up the Monitor GUI
    gui.mainloop()

