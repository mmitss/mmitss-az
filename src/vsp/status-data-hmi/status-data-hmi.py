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
import time

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
    receivedData, addr = s.recvfrom(20480)
    interfaceJson = json.loads(receivedData.decode())
    # print(interfaceJson)
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

    # Host Vehicle BSM vehicle current speed and position
    gui.latString = "{:.7f}".format(latitude_DecimalDegree) # want 7 digits showing to right of decimal
    gui.lat_value.set(gui.latString)
    gui.longString = "{:.7f}".format(longitude_DecimalDegree) # want 7 digits showing to right of decimal
    gui.long_value.set(gui.longString)
    gui.elevation_value.set(str(elevation_Meter))
    gui.heading_value.set(str(heading_Degree))
    gui.lane_value.set(str(vehicleLane))
    gui.speed_value.set(str(speed_mph + " mph"))
    gui.vehicle_type_value.set(str(vehicleType))
    gui.temporaryID_value.set(str(temporaryID))
    
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

    # SPaT status
    # get the status
    currentPhase = interfaceJson["mmitss_hmi_interface"]["infrastructure"]["currentPhase"]
    #print(currentPhase)
    # build the treeview containing the Phase Table
    redStatus = bool(currentPhase['red'])
    yellowStatus = bool(currentPhase['yellow'])
    greenStatus = bool(currentPhase['green'])
    darkStatus = bool(currentPhase['dark'])
    #print("Signal ", redStatus, yellowStatus, greenStatus, darkStatus)

    # set signal head icon
    if redStatus == True:    
        gui.Signal.config(image=gui.signal_red)
    else:
        if yellowStatus == True:    
            gui.Signal.config(image=gui.signal_yellow)
        else:
            if greenStatus == True:    
                gui.Signal.config(image=gui.signal_green)
            else:
                if darkStatus == True:    
                    gui.Signal.config(image=gui.signal_dark)

    # set min and max end times
    gui.min_value.set("Min End Time: " + str(currentPhase['minEndTime'])) 
    gui.max_value.set("Max End Time: " + str(currentPhase['maxEndTime'])) 

    # get the phase status
    phaseTable = []
    phaseTable = interfaceJson["mmitss_hmi_interface"]["infrastructure"]["phaseStates"]
    #for phase in phaseTable:
    #    print(phase['ped_status'],phase['phase'], phase['phase_status'])
    # populate the treeview containing the Phase Table
    populate_phase_tree(phaseTable)

    # tables only to be updated once every second
    if gui.tableUpdate >= 10:
        # ART Active Requests
        # get list of Priority Requests
        activeRequestTable = []
        activeRequestTable = interfaceJson["mmitss_hmi_interface"]["infrastructure"]["activeRequestTable"]
        #print("activeRequestTable", len(activeRequestTable), activeRequestTable)

        # populate the treeview containing ART
        populate_ART_tree(activeRequestTable)

        # Remote BSMs
        # get list of Remote BSMs
        remoteVehicles = []
        remoteVehicles = interfaceJson["mmitss_hmi_interface"]["remoteVehicles"]
        #print("Remote Vehicles", len(remoteVehicles), remoteVehicles)

        # populate the treeview containing ART
        populate_BSM_tree(remoteVehicles)

    
        # MAP messages
        # get list of available maps
        availableMaps = []
        availableMaps = interfaceJson["mmitss_hmi_interface"]["infrastructure"]["availableMaps"]
        #print("Available Maps", len(availableMaps), availableMaps)

        # populate the treeview containing MAPs
        populate_MAP_tree(availableMaps)

        #reset timer
        gui.tableUpdate = 0

    # performance test time that HMI receives message
    if args.perf:
        perfTest.time_received = time.time()

##############################################
#   APPLICATION STATUS DISPLAY
##############################################

def set_display_fonts_and_colors():
    # update timers
    gui.tableUpdate = 0
    
    # background colors
    gui.appBackground = 'black'
    gui.statusDisplayBackground = 'black'
    gui.statusPanelBackground = 'gray5'
    gui.statusDisplayBackground = 'gray10'
    gui.textForeground = 'gray90'
    gui.tableTitleForeground = 'alice blue'
    gui.onMapBackground = gui.statusDisplayBackground
    gui.requestSentBackground = gui.statusDisplayBackground
    gui.onMapForeground = 'pale green'
    gui.requestSentForeground = 'light yellow'

    # fonts
    gui.smallFont=("Helvetica", 15)
    gui.mediumFont=("Helvetica", 19)
    gui.largeFont=("Helvetica", 20)
    gui.hugeFont=("Helvetica", 25)
    gui.treeviewFont=("Helvetica", 19)


##############################################
#   APPLICATION DYNAMIC LABEL VARIABLES
##############################################
def set_dynamic_variables():
    # initialize textvariables for dynamic updates
    gui.min_value = StringVar()
    gui.min_value.set('Min End Time: ')
    gui.max_value = StringVar()
    gui.max_value.set('Max End Time: ')
    gui.on_map_value = StringVar()
    gui.on_map_value.set('Not On Map')
    gui.priority_request_value = StringVar()
    gui.priority_request_value.set('Priority Request Not Active')
    gui.speed_value = StringVar()
    gui.lat_value = StringVar()
    gui.long_value = StringVar()
    gui.elevation_value = StringVar()
    gui.heading_value = StringVar()
    gui.lane_value = StringVar()
    gui.temporaryID_value = StringVar()
    gui.vehicle_type_value = StringVar()
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
 
    # signal status icon (red / yellow / green / dark)
    gui.signal_red = PhotoImage(file = directory_path + "/images/Red.png") 
    gui.signal_red = gui.signal_red.subsample(1,1) 
 
    gui.signal_yellow = PhotoImage(file = directory_path + "/images/Yellow.png") 
    gui.signal_yellow = gui.signal_yellow.subsample(1,1) 

    gui.signal_green = PhotoImage(file = directory_path + "/images/Green.png") 
    gui.signal_green = gui.signal_green.subsample(1,1) 

    gui.signal_dark = PhotoImage(file = directory_path + "/images/Dark.png") 
    gui.signal_dark = gui.signal_dark.subsample(1,1) 

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
    gui.phase_tree = ttk.Treeview(gui.Phase, selectmode='none', height=2)
    gui.phase_tree["columns"]=("1", "2", "3", "4", "5", "6", "7", "8")
    gui.phase_tree.column("#0", width=100, anchor='center')
    gui.phase_tree.column("1", width=60, anchor='center')
    gui.phase_tree.column("2", width=60, anchor='center')
    gui.phase_tree.column("3", width=60, anchor='center') 
    gui.phase_tree.column("4", width=60, anchor='center')
    gui.phase_tree.column("5", width=60, anchor='center')
    gui.phase_tree.column("6", width=60, anchor='center') 
    gui.phase_tree.column("7", width=60, anchor='center')
    gui.phase_tree.column("8", width=60, anchor='center')
    gui.phase_tree.heading('#0', text='Phases', anchor='center') 
    gui.phase_tree.heading('1', text='1', anchor='center') 
    gui.phase_tree.heading("2", text="2", anchor='center') 
    gui.phase_tree.heading("3", text="3", anchor='center') 
    gui.phase_tree.heading("4", text="4", anchor='center') 
    gui.phase_tree.heading("5", text="5", anchor='center') 
    gui.phase_tree.heading("6", text="6", anchor='center') 
    gui.phase_tree.heading("7", text="7", anchor='center') 
    gui.phase_tree.heading("8", text="8", anchor='center') 

    # set style
    gui.phase_tree.style = ttk.Style()
    gui.phase_tree.style.configure("gui.phase_tree", background=gui.statusPanelBackground, foreground=gui.textForeground) # Modify the font of the body
    gui.phase_tree.style.configure("gui.phase_tree.Heading", background=gui.statusPanelBackground, foreground=gui.textForeground) # Modify the font of the headings

    # placement
    gui.phase_tree.grid(row=1, column=0, rowspan=1, sticky=E+W)

def populate_phase_tree(phaseTable):

    phaseList = []
    pedList = []

    gui.phase_tree.delete(*gui.phase_tree.get_children())

    for phase in phaseTable:
        #print(phase['ped_status'],phase['phase'], phase['phase_status'])
        phaseList.append(phase['phase_status'])
        pedList.append(phase['ped_status'])
    
    gui.phase_tree.insert('', 'end', iid='Signal', text='Signal', values=(phaseList), tags=('data'))
    gui.phase_tree.insert('', 'end', iid='Ped', text='Ped', values=(pedList), tags=('data'))
    
    # tag styles
    gui.phase_tree.tag_configure('data', background=gui.statusPanelBackground, foreground=gui.textForeground)


##############################################
#   ACTIVE REQUEST DISPLAY
##############################################

def build_ART_tree():
    gui.ART_tree = ttk.Treeview(gui.ART, selectmode='none', height=6)
    gui.ART_tree["columns"]=("VehicleID", "BasicVehicleRole", "PriorityRequestStatus", "MessageCount", "InBoundLane", "VehicleETA", "VehicleDuration")
    gui.ART_tree.column("#0", width=1)
    #gui.ART_tree.column("RequestID", width=150, anchor='center', stretch=True)
    gui.ART_tree.column("VehicleID", width=150, anchor='center', stretch=True)
    gui.ART_tree.column("BasicVehicleRole", width=100, anchor='center', stretch=True) 
    gui.ART_tree.column("PriorityRequestStatus", width=150, anchor='center', stretch=True)
    gui.ART_tree.column("MessageCount", width=150, anchor='center', stretch=True)
    gui.ART_tree.column("InBoundLane", width=150, anchor='center', stretch=True) 
    gui.ART_tree.column("VehicleETA", width=100, anchor='center', stretch=True)
    gui.ART_tree.column("VehicleDuration", width=150, anchor='center', stretch=True)
    #gui.ART_tree.heading('RequestID', text='Request', anchor='center') 
    gui.ART_tree.heading("VehicleID", text="Vehicle", anchor='center') 
    gui.ART_tree.heading("BasicVehicleRole", text="Role", anchor='center') 
    gui.ART_tree.heading("PriorityRequestStatus", text="PR Status", anchor='center') 
    gui.ART_tree.heading("MessageCount", text="Messages", anchor='center') 
    gui.ART_tree.heading("InBoundLane", text="In Bound", anchor='center') 
    gui.ART_tree.heading("VehicleETA", text="ETA", anchor='center') 
    gui.ART_tree.heading("VehicleDuration", text="Duration", anchor='center') 
    #gui.ART_tree.heading("Vehicle Type", text="Vehicle Type") 

def populate_ART_tree(activeRequestTable):

    gui.ART_tree.delete(*gui.ART_tree.get_children())

    gui.ART_tree.configure(height=6)

    for request in activeRequestTable:
        #print(request)
        gui.ART_tree.insert('', 'end', iid=None, text="", values=(request['vehicleID'], request['basicVehicleRole'], request['priorityRequestStatus'], request['msgCount'], request['inBoundLane'], request['vehicleETA'], request['duration'] ), tag = 'data')
        
    gui.ART_tree.grid(row=0, column=0, sticky=E+W)
    
    # tag styles
    gui.ART_tree.tag_configure('data', background=gui.statusPanelBackground, foreground=gui.textForeground)

##############################################
#   REMOTE BSM DISPLAY
##############################################

def build_BSM_tree():
    gui.bsm_tree = ttk.Treeview(gui.BSM, selectmode='none', height=6)
    gui.bsm_tree["columns"]=("Temp ID", "Time","Vehicle Type", "Latitude", "Longitude", "Elevation", "Heading", "Speed")
    gui.bsm_tree.column("#0", width=0)
    gui.bsm_tree.column("Temp ID", width=200, anchor='center')
    gui.bsm_tree.column("Time", width=150, anchor='center')
    gui.bsm_tree.column("Vehicle Type", width=150, anchor='center') 
    gui.bsm_tree.column("Latitude", width=175, anchor='center')
    gui.bsm_tree.column("Longitude", width=175, anchor='center')
    gui.bsm_tree.column("Elevation", width=150, anchor='center') 
    gui.bsm_tree.column("Heading", width=150, anchor='center')
    gui.bsm_tree.column("Speed", width=120, anchor='center')
    gui.bsm_tree.heading('Time', text='Time') 
    gui.bsm_tree.heading("Temp ID", text="Vehicle") 
    gui.bsm_tree.heading("Vehicle Type", text="Type") 
    gui.bsm_tree.heading("Latitude", text="Latitude") 
    gui.bsm_tree.heading("Longitude", text="Longitude") 
    gui.bsm_tree.heading("Elevation", text="Elevation") 
    gui.bsm_tree.heading("Heading", text="Heading") 
    gui.bsm_tree.heading("Speed", text="Speed") 
    #gui.bsm_tree.heading("Vehicle Type", text="Vehicle Type") 

def populate_BSM_tree(remoteVehicles):

    gui.bsm_tree.delete(*gui.bsm_tree.get_children())

    gui.bsm_tree.config(height=6)
    #gui.update_idletasks()

    for vehicle in remoteVehicles:
        gui.bsm_tree.insert('', 'end', iid=None, text=" ", values=(vehicle['BasicVehicle']['temporaryID'], vehicle['BasicVehicle']['secMark_Second'], vehicle['BasicVehicle']['vehicleType'], vehicle['BasicVehicle']['position']['latitude_DecimalDegree'], vehicle['BasicVehicle']['position']['longitude_DecimalDegree'], vehicle['BasicVehicle']['position']['elevation_Meter'], vehicle['BasicVehicle']['heading_Degree'], vehicle['BasicVehicle']['speed_MeterPerSecond']), tag='data')
        
    
    gui.bsm_tree.grid(row=0, column=0, sticky=E+W)
    
    # tag styles
    gui.bsm_tree.tag_configure('data', background=gui.statusPanelBackground, foreground=gui.textForeground)

##############################################
#   AVAILABLE MAP DISPLAY
##############################################

def build_MAP_tree():
    gui.MAP_tree = ttk.Treeview(gui.AvailableMaps, selectmode='none', height=6)
    gui.MAP_tree["columns"]=("IntersectionID", "DescriptiveName", "Active", "Age")
    gui.MAP_tree.column("#0", width=1, anchor='center')
    gui.MAP_tree.column("IntersectionID", width=150, anchor='center')
    gui.MAP_tree.column("DescriptiveName", width=600, anchor='center')
    gui.MAP_tree.column("Active", width=120, anchor='center') 
    gui.MAP_tree.column("Age", width=100, anchor='center')
    gui.MAP_tree.heading('#0', text='', anchor='center') 
    gui.MAP_tree.heading('IntersectionID', text='Intersection', anchor='center') 
    gui.MAP_tree.heading("DescriptiveName", text="Descriptive Name", anchor='center') 
    gui.MAP_tree.heading("Active", text="Active", anchor='center') 
    gui.MAP_tree.heading("Age", text="Age", anchor='center') 

    # placement
    gui.MAP_tree.grid(row=1, column=0, rowspan=1, sticky=E+W)

    # set style
    style = ttk.Style()
    style.configure("Treeview", rowheight=30, highlightthickness=0, bd=0, font=gui.treeviewFont, background=gui.statusPanelBackground, fieldbackground=gui.statusPanelBackground,foreground=gui.textForeground)
    style.configure("Treeview.Heading", font=gui.treeviewFont) # Modify the font of the headings
    style.layout("Treeview", [('Treeview.treearea', {'sticky': 'nswe'})]) # Remove the borders

def populate_MAP_tree(availableMaps):

    gui.MAP_tree.delete(*gui.MAP_tree.get_children())

    #gui.MAP_tree.config(height=len(availableMaps))
    gui.MAP_tree.config(height=6)

    if availableMaps != None:
        for map in availableMaps:
            gui.MAP_tree.insert('', 'end', iid=None, text=" ", values=(map['IntersectionID'], map['DescriptiveName'], map['active'], map['age'] ), tag='data')

    # tag styles
    #gui.MAP_tree.tag_configure('data', background=gui.statusPanelBackground, foreground=gui.textForeground)

##############################################
#  STATUS WIDGET INITIAL DISPLAY
##############################################
def create_status_widgets():
    # initialize textvariables for dynamic updates
    set_dynamic_variables()
    
    # load the static graphics
    load_static_graphics()

    # east display (left half of screen)
    gui.east = LabelFrame(gui, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, fg=gui.textForeground)
    gui.east.grid(row=0, column=0, columnspan=1, rowspan=4, padx=10, pady=10, sticky=N)

    # west display (right half of screen)
    gui.west = LabelFrame(gui, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, fg=gui.textForeground)
    gui.west.grid(row=0, column=1, columnspan=1, rowspan=6, padx=10, pady=10, sticky=N)

    # SPaT Data
    gui.SPaT = LabelFrame(gui.east, relief=RIDGE, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, text=" SPaT Data ", fg=gui.tableTitleForeground)
    gui.SPaT.grid(row=0, column=0, columnspan=1, rowspan=2, padx=10, pady=10)

    # Signal Data
    gui.SignalFrame = LabelFrame(gui.SPaT, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, fg=gui.tableTitleForeground)
    gui.SignalFrame.grid(row=0, column=0, columnspan=2, rowspan=2, padx=10, pady=10)
    gui.Signal = Label(gui.SignalFrame, image=gui.signal_dark, relief=FLAT, bd=1, bg=gui.statusPanelBackground)
    gui.Signal.grid(row=0, column=0, columnspan=1, rowspan=2, padx=10, pady=10)

    # Min / Max Data
    gui.min = Label(gui.SignalFrame, relief=FLAT, bd=1, bg=gui.statusPanelBackground, textvariable=gui.min_value, font=gui.mediumFont, fg=gui.textForeground)
    gui.min.grid(row=0, column=1, columnspan=1, rowspan=1, padx=10, pady=10, sticky=E+W)
    gui.max = Label(gui.SignalFrame, relief=FLAT, bd=1, bg=gui.statusPanelBackground, textvariable=gui.max_value, font=gui.mediumFont, fg=gui.textForeground)
    gui.max.grid(row=1, column=1, columnspan=1, rowspan=1, padx=10, pady=10, sticky=E+W)

    # Phase Data
    gui.Phase = Frame(gui.SPaT, relief=FLAT, bd=1, bg=gui.statusPanelBackground)
    gui.Phase.grid(row=3, column=0, columnspan=1, rowspan=1, padx=10, pady=10)

    # Vehicle Position Data
    gui.BasicVehicle = LabelFrame(gui.east, relief=RIDGE, bd=1, bg=gui.statusPanelBackground, text=' Host Vehicle ', font=gui.mediumFont, fg=gui.tableTitleForeground)
    gui.BasicVehicle.grid(row=2, column=0, columnspan=1, rowspan=1, padx=10, pady=10, sticky=W)

    # Labels for Host Vehicle Info
    gui.BasicVehicleLabels = LabelFrame(gui.BasicVehicle, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, fg=gui.tableTitleForeground)
    gui.BasicVehicleLabels.grid(row=0, column=0, columnspan=1, rowspan=1, padx=10, pady=10, sticky=W)

    Label(gui.BasicVehicleLabels, text='Temp ID:', font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=0, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleLabels, text='Vehicle Type:', font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=1, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleLabels, text='Speed:', font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=2, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleLabels, text='Latitude:', font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=3, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleLabels, text='Longitude:', font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=4, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleLabels, text='Elevation:', font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=5, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleLabels, text='Heading:', font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=6, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleLabels, text='Lane:', font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=7, column=0, padx=10, pady=3, sticky=W)

    # Values for Host Vehicle Info
    gui.BasicVehicleValues = LabelFrame(gui.BasicVehicle, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, fg=gui.tableTitleForeground)
    gui.BasicVehicleValues.grid(row=0, column=1, columnspan=1, rowspan=1, padx=10, pady=10, sticky=W)

    Label(gui.BasicVehicleValues, textvariable=gui.temporaryID_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=0, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleValues, textvariable=gui.vehicle_type_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=1, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleValues, textvariable=gui.speed_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=2, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleValues, textvariable=gui.lat_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=3, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleValues, textvariable=gui.long_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=4, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleValues, textvariable=gui.elevation_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=5, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleValues, textvariable=gui.heading_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=6, column=0, padx=10, pady=3, sticky=W)
    Label(gui.BasicVehicleValues, textvariable=gui.lane_value, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=7, column=0, padx=10, pady=3, sticky=W)

     # Bottom Row (EV, School Zone, and Available Maps)
    gui.Multi = Frame(gui.west, relief=RAISED, bd=1, bg=gui.statusPanelBackground)
    gui.Multi.grid(row=0, column=0, columnspan=1, rowspan=1, padx=10, pady=10, sticky=S)
 
   # Top Multi-Column Status
    gui.Map = LabelFrame(gui.Multi, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, fg=gui.textForeground)
    gui.Map.grid(row=0, column=0, columnspan=1, rowspan=1, padx=10, pady=10, sticky=N+E+W)

    # Map Status
    gui.map_label = Label(gui.Map, textvariable=gui.on_map_value, relief=RIDGE, bd=1, bg=gui.onMapBackground, font=gui.mediumFont, text="Map Status", fg=gui.onMapForeground)
    gui.map_label.grid(row=0, column=0, columnspan=1, padx=10, pady=10, sticky=E+W)
    # priority request status
    gui.priority_label = Label(gui.Map, textvariable=gui.priority_request_value, relief=RIDGE, font=gui.mediumFont, fg=gui.requestSentForeground, bg=gui.requestSentBackground, justify=LEFT)
    gui.priority_label.grid(row=1, column=0, rowspan=1, padx=5, pady=2, sticky=E+W)

    # EV
    gui.EV = Frame(gui.Multi, relief=FLAT, bd=1, bg=gui.statusPanelBackground)
    gui.EV.grid(row=0, column=1, padx=10, pady=10, sticky=W)
    Label(gui.EV, image=gui.ev_dark, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)

    # School Zone
    gui.SchoolZone = Frame(gui.Multi, relief=FLAT, bd=1, bg=gui.statusPanelBackground)
    gui.SchoolZone.grid(row=0, column=2, padx=10, pady=10, sticky=W)
    Label(gui.SchoolZone, image=gui.school_zone_dark, font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusPanelBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)

    # ART
    gui.ART = LabelFrame(gui.west, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, text="Active Request Table", fg=gui.tableTitleForeground)
    gui.ART.grid(row=1, column=0, columnspan=1, rowspan=1, padx=10, pady=10, sticky=W)
    #Label(gui.ART, text="PLACEHOLDER", font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=E+W)

    # Remote Basic Vehicle Messages
    gui.BSM = LabelFrame(gui.west, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, text="Remote Vehicles", fg=gui.tableTitleForeground)
    gui.BSM.grid(row=2, column=0, columnspan=1, rowspan=1, padx=10, pady=10, sticky=W)
    Label(gui.BSM, text="No Vehicle Data", font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=E+W)

    # Available Maps
    gui.AvailableMaps = LabelFrame(gui.west, relief=FLAT, bd=1, bg=gui.statusPanelBackground, font=gui.mediumFont, text="Available Maps", fg=gui.tableTitleForeground)
    gui.AvailableMaps.grid(row=3, column=0, padx=10, pady=10, sticky=E+W)
    
    '''
    Label(gui.AvailableMaps, text="placeholder", font=gui.mediumFont, fg=gui.textForeground, bg=gui.statusDisplayBackground, justify=LEFT).grid(row=0, column=0, padx=5, pady=2, sticky=S+E+W)
    Label(gui.AvailableMaps, textvariable=gui.received_message1_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=0, column=0, padx=50, pady=2)
    Label(gui.AvailableMaps, textvariable=gui.received_message2_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=1, column=0, padx=50, pady=2)
    Label(gui.AvailableMaps, textvariable=gui.received_message3_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=2, column=0, padx=50, pady=2)
    Label(gui.AvailableMaps, textvariable=gui.received_message4_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=3, column=0, padx=100, pady=2)
    Label(gui.AvailableMaps, textvariable=gui.received_message5_value, font=gui.mediumFont, foreground="RoyalBlue1", bg=gui.statusPanelBackground).grid(row=4, column=0, padx=100, pady=2)
    '''

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

    # refresh dynamic data labels that were set to type class StringVar
    # class StringVar updates dynamically with root.update_idletasks()
    gui.update_idletasks()

    # update timer
    gui.tableUpdate = gui.tableUpdate + 1

    # Performance Testing
    if args.perf:
        perfTest.time_refreshed = time.time()
        # write the row for perf test
        perfTest.perf_writer.writerow([perfTest.time_sent, perfTest.time_received, perfTest.time_refreshed])	
    
    # reset refresh timer
    gui.after(50, update_display)

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
    hmiPort = 20010
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
    gui.title("MMITSS Vehicle Status")  
    gui['bg'] = gui.appBackground

    # create the application footer with s
    create_app_footer()

    # create the frame that holds all dynamic data (everything except the static footer)
    create_status_widgets()

    # build the lists for phase, ART, BSM, and MAP    
    build_phase_tree()
    build_ART_tree()
    build_BSM_tree()
    build_MAP_tree()
    #populate_MAP_tree('')

    # set timer for updating data and call the update function
    gui.after(50, update_display)


    # handle event for closing the application
    gui.protocol("WM_DELETE_WINDOW", on_closing)
	
    # fire up the Monitor GUI
    gui.mainloop()
