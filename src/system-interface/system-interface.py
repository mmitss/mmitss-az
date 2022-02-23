"""
***************************************************************************************
 © 2020 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
***************************************************************************************
system-interface.py
Created by: Sherilyn Keaton
University of Arizona   
College of Engineering
This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.
***************************************************************************************
Description:
------------
This is a web-based Python Flask application that has the following functionality:
    * MMITSS Component Process Control and Status Reporting Console
    * MMITSS Configuration Viewer and Editor
***************************************************************************************
"""

from flask import Flask, render_template, request, flash, send_from_directory
from flask_wtf import FlaskForm
from wtforms import StringField, IntegerField, BooleanField, DecimalField, validators, SelectField
from wtforms.validators import *
from flask_bootstrap import Bootstrap
import os
import sys
import pandas as pd
import csv
import json
import glob
from PIL import Image
import base64
import io

# Initialize application for either PyInstaller or Development
if getattr(sys, 'frozen', False):
    template_folder = os.path.join(sys._MEIPASS, 'templates')
    static_folder = os.path.join(sys._MEIPASS, 'static')
    app = Flask(__name__, template_folder=template_folder, static_folder=static_folder)
else:
    app = Flask(__name__)

# Apply Bootstrap 
bootstrap = Bootstrap(app)
app.config['SECRET_KEY'] = '%sq72f#8c$seryfl#2h'
app.config['BOOTSTRAP_SERVE_LOCAL'] = True

# Index (Homepage)
@app.route('/')
def index():
    return render_template('index.html')

# Process Control Console (remote)
@app.route('/console')
def console():
    return render_template('console.html')

# Process Control Console (local)
@app.route('/local_console')
def local_console():
    import json
    

    with open('/nojournal/bin/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)
        pageTitle = data['IntersectionName']
        hostIp = data['HostIp']
        hostIpAndPort = "http://" + hostIp + ":9001"

    return render_template('local_console.html', pageTitle=pageTitle, hostIpAndPort=hostIpAndPort)

# Configuration Viewer / Editor combined form
class ConfigurationForm(FlaskForm):
    hostIp = StringField('Host IP', validators=[ip_address()])
    sourceDsrcDeviceIp      = StringField('Wireless Device IP (RSU or OBU)', validators=[ip_address()])
    intersectionName        = StringField('Intersection Name')
    intersectionID          = IntegerField('Intersection ID')
    regionalID              = IntegerField('Regional ID')
    mapPayload              = StringField('Map Payload')
    dataCollectorIP         = StringField('Remote Data Collector Server IP')
    hmiControllerIP         = StringField('HMI Controller IP')
    messageDistributorIP    = StringField('Message Distributor IP')
    priorityRequestGeneratorServerIP = StringField('Priority Request Generator Server IP Address (optional)')
    vehicleType                     = SelectField('Vehicle Type', choices = ["Transit", "EmergencyVehicle", "Truck"])
    logging                         = BooleanField('Logging')
    consoleOutput                   = BooleanField('Console Output')
    timePhaseDiagram   = BooleanField('Time Phase Diagram')
    srmTimedOutTime                 = StringField('SRM Timed Out Time (seconds)')
    scheduleExecutionBuffer         = StringField('Schedule Execution Buffer')
    systemPerformanceTimeInterval   = StringField('System Performance Time Interval (seconds)')
    applicationPlatform             = SelectField('Application Platform', choices = ["roadside", "vehicle"])
    peerDataDecoding                = BooleanField('Peer Data Decoding')
    offmapBsmFiltering              = BooleanField("Off Map BSM Filtering")
    portNumberMTMessageSender       = IntegerField('Port Number: Message Transceiver / Message Sender')
    portNumberMTMessageReceiver     = IntegerField('Port Number: Message Transceiver / Message Receiver')
    portNumberMTMessageEncoder      = IntegerField('Port Number: Message Transceiver / Message Encoder')
    portNumberMTMessageDecoder      = IntegerField('Port Number: Message Transceiver / Message Decoder')
    portNumberMessageDistributor    = IntegerField('Port Number: Message Distributor')
    portNumberRSMDecoder            = IntegerField('Port Number: RSM Decoder')
    portNumberOBUBSMReceiver        = IntegerField('Port Number: OBU BSM Receiver')
    portNumberHostBsmDecoder        = IntegerField('Port Number: Host BSM Decoder')
    portNumberTrajectoryAware       = IntegerField('Port Number: TrajectoryAware')
    portNumberTrajectoryAware       = IntegerField('Port Number: Trajectory Aware')
    portNumberPriorityRequestServer = IntegerField('Port Number: Priority Request Server')
    portNumberPrioritySolver        = IntegerField('Port Number: Priority Solver')
    portNumberPriorityRequestGenerator              = IntegerField('Port Number: Priority Request Generator')
    portNumberTrafficControllerInterface            = IntegerField('Port Number: Traffic Controller Interface')
    portNumberTrafficControllerCurrPhaseListener    = IntegerField('Port Number: Traffic Controller Current Phase Listener')
    portNumberTrafficControllerTimingPlanSender     = IntegerField('Port Number: Traffic Controller Timing Plan Sender')
    portNumberPerformanceObserver                   = IntegerField('Port Number: Performance Observer')
    portNumberHMIController                         = IntegerField('Port Number: HMI Controller')
    portNumberPrioritySolverToTCIInterface          = IntegerField('Port Number: Priority Solver To TCI Interface')
    portNumberSignalCoordination                    = IntegerField('Port Number: Signal Coordination')
    portNumberMapSPaTBroadcaster                    = IntegerField('Port Number: Map SPaT Broadcaster')
    portNumberDsrcImmediateForwarder                = IntegerField('Port Number: DSRC Immediate Forwarder')
    portNumberPriorityRequestServer_SendSSM         = IntegerField('Port Number: PriorityRequestServer_SendSSM')
    portNumberDataCollector                         = IntegerField('Port Number: Data Collector')
    portNumberSnmpEngine                            = IntegerField('Port Number: SNMP Engine')
    portNumberSnmpEngineInterface                   = IntegerField('Port Number: SNMP Engine Interface')
    portNumberPriorityRequestGeneratorServer        = IntegerField('Port Number: Priority Request Generator Server')
    portNumberTrajectoryAware_MapEngineInterface    = IntegerField('Port Number: Trajectory Aware Map Engine Interface')
    portNumberMapEngine                             = IntegerField('Port Number: Map Engine')
    portNumberLightSirenStatusManager               = IntegerField('Port Number: Light Siren Status Manager')
    portNumberPeerToPeerPriority                    = IntegerField('Port Number: Peer To Peer Priority')
    portNumberTimePhaseDiagramTool                  = IntegerField('Port Number: Time Phase Diagram Tool')
    psidMap = StringField('PSID: Map')
    psidSPaT = StringField('PSID: SPaT')    
    psidRSM = StringField('PSID: RSM')    
    psidSRM = StringField('PSID: SRM')    
    psidSSM = StringField('PSID: SSM')    
    psidBSM = StringField('PSID: BSM')
    msgIdMap = StringField('Msg ID: map')
    msgIdSPaT = StringField('Msg ID: SPaT')
    msgIdRSM = StringField('Msg ID: RSM')
    msgIdSRMLower = StringField('Msg ID: SRM Lower')
    msgIdSRMUpper = StringField('Msg ID: SRM Upper')
    msgIdSSMLower = StringField('Msg ID: SSM Lower')
    msgIdSSMUpper = StringField('Msg ID: SSM Upper')
    msgIdBSM = StringField('Msg ID: BSM')
    txChannelMap    = StringField('Tx Channel: Map')
    txChannelSPaT   = StringField('Tx Channel: SPaT')    
    txChannelRSM    = StringField('Tx Channel: RSM')    
    txChannelSRM    = StringField('Tx Channel: SRM')    
    txChannelSSM    = StringField('Tx Channel: SSM')    
    txChannelBSM    = StringField('Tx Channel: BSM')
    txModeMap       = StringField('Tx Mode: Map')
    txModeSPaT      = StringField('Tx Mode: SPaT')    
    txModeRSM       = StringField('Tx Mode: RSM')    
    txModeSRM       = StringField('Tx Mode: SRM')    
    txModeSSM       = StringField('Tx Mode: SSM')    
    txModeBSM       = StringField('Tx Mode: BSM')
    signalControllerIP                  = StringField('Signal Controller IP Address')
    signalControllerNTCIPPort           = IntegerField('Signal Controller NTCIP Port')
    signalControllerUpdateInterval      = IntegerField('Signal Controller Timing Plan Update Interval (seconds)')
    signalControllerNtcipBackupTime_sec = IntegerField('Signal Controller NTCIP Backup Time (seconds)')
    signalControllerVendor              = StringField('Signal Controller Vendor')
    signalControllerTimingPlanMib       = StringField('Signal Controller Timing Plan MIB')
    signalControllerInactiveVehPhases   = StringField('Signal Controller Inactive Vehicle Phases  (e.g., [1,3,5])')
    signalControllerInactivePedPhases   = StringField('Signal Controller Inactive Pedestrian Phases  (e.g., [1,3,5])')
    signalControllerSplitPhases1        = IntegerField('Signal Controller Split Phases 1')
    signalControllerSplitPhases3        = IntegerField('Signal Controller Split Phases 3')
    signalControllerSplitPhases5        = IntegerField('Signal Controller Split Phases 5')
    signalControllerSplitPhases7        = IntegerField('Signal Controller Split Phases 7')
    signalControllerPermissiveEnabled1  = BooleanField('Signal Controller Permissive Enabled 1')
    signalControllerPermissiveEnabled3  = BooleanField('Signal Controller Permissive Enabled 3')
    signalControllerPermissiveEnabled5  = BooleanField('Signal Controller Permissive Enabled 5')
    signalControllerPermissiveEnabled7  = BooleanField('Signal Controller Permissive Enabled 7')
    intersectionReferencePointLatitudeDecimalDegree     = StringField('Intersection Reference Point Latitude Decimal Degree')
    intersectionReferencePointLongitudeDecimalDegree    = StringField('Intersection Reference Point Longitude Decimal Degree')
    intersectionReferencePointElevationMeter            = IntegerField('Intersection Reference Point Elevation Meter')
    dataTransferServerDataDirectory             = StringField('Data Transfer Server Data Directory')
    dataTransferServerIPAddress                 = StringField('Data Transfer Server IP Address')
    dataTransferServerUsername                  = StringField('Data Transfer Server Username')
    dataTransferServerPassword                  = StringField('Data Transfer Server Password')
    dataTransferIntersectionName = StringField('Data Transfer Intersection Name')
    dataTransferIntersectionV2xDataLocation = StringField('Data Transfer Interesction v2x Data Location')
    dataTransferStartTimePushToServerHour       = IntegerField('Data Transfer Start Time Push To Server Hour')
    dataTransferStartTimePushToServerMinute     = IntegerField('Data Transfer Start Time Push To Server Minute')  
    priorityEVWeight                    = StringField('Emergency Vehicle Weight')    
    priorityEVSplitPhaseWeight          = StringField('Emergency Vehicle Split Phase Weight')    
    priorityTransitWeight                = StringField('Transit Weight')    
    priorityTruckWeight                 = StringField('Truck Weight')    
    priorityDilemmaZoneRequestWeight      = StringField('Dilemma Zone Request Weight (Truck Only)')    
    priorityCoordinationWeight            = StringField('Coordination Weight')
    priorityFlexibilityWeight             = StringField('Flexibility Weight')    
    coordinationPlanCheckingTimeInterval  = IntegerField('Coordination Plan Update Interval (seconds)')    

# System Configuration data object
class SysConfig:
    def __init__(self, data):
        self.hostIp = data['HostIp']
        self.sourceDsrcDeviceIp = data['SourceDsrcDeviceIp']
        self.intersectionName = data['IntersectionName']
        self.intersectionID = data['IntersectionID']
        self.mapPayload = data['MapPayload']
        self.regionalID = data['RegionalID']
        self.dataCollectorIP = data['DataCollectorIP']
        self.hmiControllerIP = data['HMIControllerIP']
        self.messageDistributorIP = data['MessageDistributorIP']
        self.priorityRequestGeneratorServerIP = data['PriorityRequestGeneratorServerIP']
        self.vehicleType = data['VehicleType']
        self.logging = data['Logging']
        self.consoleOutput = data['ConsoleOutput']
        # check for past versions that may not have element
        try:
            self.timePhaseDiagram = data['TimePhaseDiagram']
        except (KeyError):
            flash("Time Phase Diagram field has not been saved.")
            self.timePhaseDiagram = False
        self.srmTimedOutTime = data['SRMTimedOutTime']
        self.scheduleExecutionBuffer = data['ScheduleExecutionBuffer']
        self.systemPerformanceTimeInterval = data['SystemPerformanceTimeInterval']
        self.applicationPlatform = data['ApplicationPlatform']
        self.peerDataDecoding = data['PeerDataDecoding']
        self.offmapBsmFiltering = data['OffmapBsmFiltering']
        self.portNumberMTMessageSender = data['PortNumber']['MessageTransceiver']['MessageSender']
        self.portNumberMTMessageReceiver = data['PortNumber']['MessageTransceiver']['MessageReceiver']
        self.portNumberMTMessageEncoder = data['PortNumber']['MessageTransceiver']['MessageEncoder']
        self.portNumberMTMessageDecoder = data['PortNumber']['MessageTransceiver']['MessageDecoder']
        self.portNumberMessageDistributor = data['PortNumber']['MessageDistributor']
        self.portNumberRSMDecoder = data['PortNumber']['RsmDecoder']
        self.portNumberOBUBSMReceiver = data['PortNumber']['OBUBSMReceiver']
        self.portNumberHostBsmDecoder = data['PortNumber']['HostBsmDecoder']
        self.portNumberTrajectoryAware = data['PortNumber']['TrajectoryAware']
        self.portNumberTrajectoryAware = data['PortNumber']['TrajectoryAware']
        self.portNumberPriorityRequestServer = data['PortNumber']['PriorityRequestServer']
        self.portNumberPrioritySolver = data['PortNumber']['PrioritySolver']
        self.portNumberPriorityRequestGenerator = data['PortNumber']['PriorityRequestGenerator']
        self.portNumberTrafficControllerInterface = data['PortNumber']['TrafficControllerInterface']
        self.portNumberTrafficControllerCurrPhaseListener = data['PortNumber']['TrafficControllerCurrPhaseListener']
        self.portNumberTrafficControllerTimingPlanSender = data['PortNumber']['TrafficControllerTimingPlanSender']
        self.portNumberPerformanceObserver = data['PortNumber']['PerformanceObserver']
        self.portNumberHMIController = data['PortNumber']['HMIController']
        self.portNumberPrioritySolverToTCIInterface = data['PortNumber']['PrioritySolverToTCIInterface']
        self.portNumberSignalCoordination = data['PortNumber']['SignalCoordination']
        self.portNumberMapSPaTBroadcaster = data['PortNumber']['MapSPaTBroadcaster']
        self.portNumberDsrcImmediateForwarder = data['PortNumber']['DsrcImmediateForwarder']
        self.portNumberPriorityRequestServer_SendSSM = data['PortNumber']['PriorityRequestServer_SendSSM']
        self.portNumberDataCollector = data['PortNumber']['DataCollector']
        self.portNumberSnmpEngine = data['PortNumber']['SnmpEngine']
        self.portNumberSnmpEngineInterface = data['PortNumber']['SnmpEngineInterface']
        self.portNumberPriorityRequestGeneratorServer = data['PortNumber']['PriorityRequestGeneratorServer']
        self.portNumberTrajectoryAware_MapEngineInterface = data['PortNumber']['TrajectoryAware_MapEngineInterface']
        self.portNumberMapEngine = data['PortNumber']['MapEngine']
        self.portNumberLightSirenStatusManager = data['PortNumber']['LightSirenStatusManager']
        self.portNumberPeerToPeerPriority = data['PortNumber']['PeerToPeerPriority']
        try:
            self.portNumberTimePhaseDiagramTool = data['PortNumber']['TimePhaseDiagramTool']
        except (KeyError):
            flash("Time Phase Diagram Tool field has not been saved.")
            self.portNumberTimePhaseDiagramTool = "0"
        self.psidMap = data['psid']['map']
        self.psidSPaT = data['psid']['spat']
        self.psidRSM = data['psid']['rsm']
        self.psidSRM = data['psid']['srm']
        self.psidSSM = data['psid']['ssm']
        self.psidBSM = data['psid']['bsm']
        self.msgIdMap = data['msgId']['map']
        self.msgIdSPaT = data['msgId']['spat']
        self.msgIdRSM = data['msgId']['rsm']
        self.msgIdSRMLower = data['msgId']['srm_lower']
        self.msgIdSRMUpper = data['msgId']['srm_upper']
        self.msgIdSSMLower = data['msgId']['ssm_lower']
        self.msgIdSSMUpper = data['msgId']['ssm_upper']
        self.msgIdBSM = data['msgId']['bsm']
        self.txChannelMap = data['TxChannel']['map']
        self.txChannelSPaT = data['TxChannel']['spat']
        self.txChannelRSM = data['TxChannel']['rsm']
        self.txChannelSRM = data['TxChannel']['srm']
        self.txChannelSSM = data['TxChannel']['ssm']
        self.txChannelBSM = data['TxChannel']['bsm']
        self.txModeMap = data['TxMode']['map']
        self.txModeSPaT = data['TxMode']['spat']
        self.txModeRSM = data['TxMode']['rsm']
        self.txModeSRM = data['TxMode']['srm']
        self.txModeSSM = data['TxMode']['ssm']
        self.txModeBSM = data['TxMode']['bsm']
        self.signalControllerIP = data['SignalController']['IpAddress']
        self.signalControllerNTCIPPort = data['SignalController']['NtcipPort']
        self.signalControllerUpdateInterval = data['SignalController']['TimingPlanUpdateInterval_sec']
        self.signalControllerNtcipBackupTime_sec = data['SignalController']['NtcipBackupTime_sec']
        self.signalControllerVendor = data['SignalController']['Vendor']
        self.signalControllerTimingPlanMib = data['SignalController']['TimingPlanMib']
        self.signalControllerInactiveVehPhases = data['SignalController']['InactiveVehPhases']
        self.signalControllerInactivePedPhases = data['SignalController']['InactivePedPhases']
        self.signalControllerSplitPhases1 = data['SignalController']['SplitPhases']['1']
        self.signalControllerSplitPhases3 = data['SignalController']['SplitPhases']['3']
        self.signalControllerSplitPhases5 = data['SignalController']['SplitPhases']['5']
        self.signalControllerSplitPhases7 = data['SignalController']['SplitPhases']['7']
        self.signalControllerPermissiveEnabled1 = data['SignalController']['PermissiveEnabled']['1']
        self.signalControllerPermissiveEnabled3 = data['SignalController']['PermissiveEnabled']['3']
        self.signalControllerPermissiveEnabled5 = data['SignalController']['PermissiveEnabled']['5']
        self.signalControllerPermissiveEnabled7 = data['SignalController']['PermissiveEnabled']['7']
        self.intersectionReferencePointLatitudeDecimalDegree = data['IntersectionReferencePoint']['Latitude_DecimalDegree']
        self.intersectionReferencePointLongitudeDecimalDegree = data['IntersectionReferencePoint']['Longitude_DecimalDegree']
        self.intersectionReferencePointElevationMeter = data['IntersectionReferencePoint']['Elevation_Meter']
        self.dataTransferServerDataDirectory = data['DataTransfer']['server']['data_directory']
        self.dataTransferServerIPAddress = data['DataTransfer']['server']['ip_address']
        self.dataTransferServerUsername = data['DataTransfer']['server']['username']
        self.dataTransferServerPassword = data['DataTransfer']['server']['password']
        self.dataTransferIntersectionName = data['DataTransfer']['intersection'][0]['name']
        self.dataTransferIntersectionV2xDataLocation = data['DataTransfer']['intersection'][0]['v2x-data_location']
        self.dataTransferStartTimePushToServerHour = data['DataTransfer']['StartTime_PushToServer']['hour']
        self.dataTransferStartTimePushToServerMinute = data['DataTransfer']['StartTime_PushToServer']['minute']
        self.priorityEVWeight                   = data['PriorityParameter']['EmergencyVehicleWeight']
        self.priorityEVSplitPhaseWeight         = data['PriorityParameter']['EmergencyVehicleSplitPhaseWeight']
        self.priorityTransitWeight              = data['PriorityParameter']['TransitWeight']
        self.priorityTruckWeight                = data['PriorityParameter']['TruckWeight']
        self.priorityDilemmaZoneRequestWeight   = data['PriorityParameter']['DilemmaZoneRequestWeight']
        self.priorityCoordinationWeight         = data['PriorityParameter']['CoordinationWeight']
        try:
            self.priorityFlexibilityWeight          = data['PriorityParameter']['FlexibilityWeight']
        except (KeyError):
            flash("Flexibility Weight field has not been saved.")
            self.priorityFlexibilityWeight          = "0.00"
        self.coordinationPlanCheckingTimeInterval   = data['CoordinationPlanCheckingTimeInterval']

'''
def buildMissingFieldsList(missingFields):
    if missing_fields_message.Empty()
        # add preamble
    else
        # append missing fields
    return intList
'''

def convertToList(formString):
    # remove any brackets
    formString = formString.replace("[", "")
    formString = formString.replace("]", "")
    
    if len(formString) > 0:
        # split the string
        string_list = formString.split(",")
        intList = [int(x) for x in string_list]
    else:
        # empty list
        intList = []
    return intList

def prepareJSONData(data, form):
    data['HostIp']              = form.hostIp.data
    data['SourceDsrcDeviceIp']  = form.sourceDsrcDeviceIp.data
    data['IntersectionName']    = form.intersectionName.data
    data['IntersectionID']      = form.intersectionID.data
    data['MapPayload']          = form.mapPayload.data
    data['RegionalID']          = form.regionalID.data
    data['DataCollectorIP']     = 'Deprecated'
    data['HMIControllerIP']     = form.hmiControllerIP.data
    data['MessageDistributorIP']= form.messageDistributorIP.data
    data['PriorityRequestGeneratorServerIP']= form.priorityRequestGeneratorServerIP.data
    data['VehicleType']= form.vehicleType.data
    data['Logging']= form.logging.data
    data['ConsoleOutput']=form.consoleOutput.data
    data['TimePhaseDiagram']=form.timePhaseDiagram.data
    data['SRMTimedOutTime']= float(form.srmTimedOutTime.data)
    data['ScheduleExecutionBuffer']= 'Deprecated'
    data['SystemPerformanceTimeInterval']= float(form.systemPerformanceTimeInterval.data)
    data['ApplicationPlatform']= form.applicationPlatform.data
    data['PeerDataDecoding']= form.peerDataDecoding.data
    data['OffmapBsmFiltering']  =  form.offmapBsmFiltering.data
    data['PortNumber']['MessageTransceiver']['MessageSender']= 'Deprecated'
    data['PortNumber']['MessageTransceiver']['MessageReceiver']= 'Deprecated'
    data['PortNumber']['MessageTransceiver']['MessageEncoder']= form.portNumberMTMessageEncoder.data
    data['PortNumber']['MessageTransceiver']['MessageDecoder']= form.portNumberMTMessageDecoder.data
    data['PortNumber']['MessageDistributor']    = form.portNumberMessageDistributor.data
    data['PortNumber']['RsmDecoder']    = form.portNumberRSMDecoder.data
    data['PortNumber']['OBUBSMReceiver']    = form.portNumberOBUBSMReceiver.data
    data['PortNumber']['HostBsmDecoder']    = form.portNumberHostBsmDecoder.data
    data['PortNumber']['TrajectoryAware']   = form.portNumberTrajectoryAware.data
    data['PortNumber']['PriorityRequestServer']    = form.portNumberPriorityRequestServer.data
    data['PortNumber']['PrioritySolver']    = form.portNumberPrioritySolver.data
    data['PortNumber']['PriorityRequestGenerator']    = form.portNumberPriorityRequestGenerator.data
    data['PortNumber']['TrafficControllerInterface']    = form.portNumberTrafficControllerInterface.data
    data['PortNumber']['TrafficControllerCurrPhaseListener']    = form.portNumberTrafficControllerCurrPhaseListener.data
    data['PortNumber']['TrafficControllerTimingPlanSender']    = form.portNumberTrafficControllerTimingPlanSender.data
    data['PortNumber']['PerformanceObserver']    = form.portNumberPerformanceObserver.data
    data['PortNumber']['HMIController']    = form.portNumberHMIController.data
    data['PortNumber']['PrioritySolverToTCIInterface']    = form.portNumberPrioritySolverToTCIInterface.data
    data['PortNumber']['SignalCoordination']    = form.portNumberSignalCoordination.data
    data['PortNumber']['MapSPaTBroadcaster']    = form.portNumberMapSPaTBroadcaster.data
    data['PortNumber']['DsrcImmediateForwarder']    = form.portNumberDsrcImmediateForwarder.data
    data['PortNumber']['PriorityRequestServer_SendSSM']    = form.portNumberPriorityRequestServer_SendSSM.data
    data['PortNumber']['DataCollector']    = form.portNumberDataCollector.data
    data['PortNumber']['SnmpEngine']    = form.portNumberSnmpEngine.data
    data['PortNumber']['SnmpEngineInterface']    = form.portNumberSnmpEngineInterface.data
    data['PortNumber']['PriorityRequestGeneratorServer']    = form.portNumberPriorityRequestGeneratorServer.data
    data['PortNumber']['TrajectoryAware_MapEngineInterface']    = form.portNumberTrajectoryAware_MapEngineInterface.data
    data['PortNumber']['MapEngine']    = 'Component is Yet to Come'
    data['PortNumber']['LightSirenStatusManager']    = form.portNumberLightSirenStatusManager.data
    data['PortNumber']['PeerToPeerPriority']    = form.portNumberPeerToPeerPriority.data
    data['PortNumber']['TimePhaseDiagramTool']   = form.portNumberTimePhaseDiagramTool.data
    data['psid']['map']    = form.psidMap.data    
    data['psid']['spat']    = form.psidSPaT.data
    data['psid']['rsm']    = form.psidRSM.data
    data['psid']['srm']    = form.psidSRM.data
    data['psid']['ssm']    = form.psidSSM.data
    data['psid']['bsm']    = form.psidBSM.data
    data['msgId']['map']    = form.msgIdMap.data
    data['msgId']['spat']    = form.msgIdSPaT.data
    data['msgId']['rsm']    = form.msgIdRSM.data
    data['msgId']['srm_lower']    = form.msgIdSRMLower.data
    data['msgId']['srm_upper']    = form.msgIdSRMUpper.data
    data['msgId']['ssm_lower']    = form.msgIdSSMLower.data
    data['msgId']['ssm_upper']    = form.msgIdSSMUpper.data
    data['msgId']['bsm']    = form.msgIdBSM.data
    data['TxChannel']['map']    = form.txChannelMap.data    
    data['TxChannel']['spat']    = form.txChannelSPaT.data
    data['TxChannel']['rsm']    = form.txChannelRSM.data
    data['TxChannel']['srm']    = form.txChannelSRM.data
    data['TxChannel']['ssm']    = form.txChannelSSM.data
    data['TxChannel']['bsm']    = form.txChannelBSM.data
    data['TxMode']['map']    = form.txModeMap.data    
    data['TxMode']['spat']    = form.txModeSPaT.data
    data['TxMode']['rsm']    = form.txModeRSM.data
    data['TxMode']['srm']    = form.txModeSRM.data
    data['TxMode']['ssm']    = form.txModeSSM.data
    data['TxMode']['bsm']    = form.txModeBSM.data
    data['SignalController']['IpAddress']    = form.signalControllerIP.data
    data['SignalController']['NtcipPort']    = form.signalControllerNTCIPPort.data
    data['SignalController']['TimingPlanUpdateInterval_sec']    = form.signalControllerUpdateInterval.data
    data['SignalController']['NtcipBackupTime_sec']    = 'Deprecated'
    data['SignalController']['Vendor']    = form.signalControllerVendor.data
    data['SignalController']['TimingPlanMib']    = form.signalControllerTimingPlanMib.data
    data['SignalController']['InactiveVehPhases']    = convertToList(form.signalControllerInactiveVehPhases.data)
    data['SignalController']['InactivePedPhases']    = convertToList(form.signalControllerInactivePedPhases.data)
    data['SignalController']['SplitPhases'] ['1']   = form.signalControllerSplitPhases1.data
    data['SignalController']['SplitPhases'] ['3']   = form.signalControllerSplitPhases3.data
    data['SignalController']['SplitPhases'] ['5']   = form.signalControllerSplitPhases5.data
    data['SignalController']['SplitPhases'] ['7']   = form.signalControllerSplitPhases7.data
    data['SignalController']['PermissiveEnabled'] ['1']   = form.signalControllerPermissiveEnabled1.data
    data['SignalController']['PermissiveEnabled'] ['3']   = form.signalControllerPermissiveEnabled3.data
    data['SignalController']['PermissiveEnabled'] ['5']   = form.signalControllerPermissiveEnabled5.data
    data['SignalController']['PermissiveEnabled'] ['7']   = form.signalControllerPermissiveEnabled7.data
    data['IntersectionReferencePoint']['Latitude_DecimalDegree']    = float(form.intersectionReferencePointLatitudeDecimalDegree.data)
    data['IntersectionReferencePoint']['Longitude_DecimalDegree']   = float(form.intersectionReferencePointLongitudeDecimalDegree.data)
    data['IntersectionReferencePoint']['Elevation_Meter']           = float(form.intersectionReferencePointElevationMeter.data)
    data['DataTransfer']['server']['data_directory']            = form.dataTransferServerDataDirectory.data
    data['DataTransfer']['server']['ip_address']                = form.dataTransferServerIPAddress.data
    data['DataTransfer']['server']['username']                  = form.dataTransferServerUsername.data
    data['DataTransfer']['server']['password']                  = form.dataTransferServerPassword.data
    data['DataTransfer']['intersection'][0]['name']                 = form.dataTransferIntersectionName.data
    data['DataTransfer']['intersection'][0]['v2x-data_location']    = form.dataTransferIntersectionV2xDataLocation.data
    data['DataTransfer']['StartTime_PushToServer']['hour']     = form.dataTransferStartTimePushToServerHour.data
    data['DataTransfer']['StartTime_PushToServer']['minute']   = form.dataTransferStartTimePushToServerMinute.data
    data['PriorityParameter']['EmergencyVehicleWeight']             = float(form.priorityEVWeight.data)
    data['PriorityParameter']['EmergencyVehicleSplitPhaseWeight']   = float(form.priorityEVSplitPhaseWeight.data)
    data['PriorityParameter']['TransitWeight']                      = float(form.priorityTransitWeight.data)
    data['PriorityParameter']['TruckWeight']                        = float(form.priorityTruckWeight.data)
    data['PriorityParameter']['DilemmaZoneRequestWeight']           = float(form.priorityDilemmaZoneRequestWeight.data)
    data['PriorityParameter']['CoordinationWeight']                 = float(form.priorityCoordinationWeight.data)
    data['PriorityParameter']['FlexibilityWeight']                  = float(form.priorityFlexibilityWeight.data)
    data['CoordinationPlanCheckingTimeInterval']                    = form.coordinationPlanCheckingTimeInterval.data

# configuration viewer / editor
@app.route('/configuration/', methods = ['GET', 'POST'])
def configuration():
    import json
    
    #field location
    with open('/nojournal/bin/mmitss-phase3-master-config.json') as json_file:
    #test location
    #with open('static/json/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)
        sysConfig = SysConfig(data)    
        pageTitle = data['IntersectionName']
        form = ConfigurationForm(obj=sysConfig)

    #if request.method == 'POST' and form.validate():
    if request.method == 'POST':
        # Serialize the edited data
        #field location
        with open('/nojournal/bin/mmitss-phase3-master-config.json', 'w') as json_file:
        #test location
        #with open('static/json/mmitss-phase3-master-config.json', 'w') as json_file:
            prepareJSONData(data, form)
            dataResult = json.dump(data, json_file, indent="\t") 
            flash('Configuration Updated')  
    
    return render_template('configuration.html', pageTitle=pageTitle, form=form)


@app.route('/performance_data/',methods=['GET','POST'])
def performance_data():
    
    #read the cofig file to extract the intersection name and platform
    with open('/nojournal/bin/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)
        
        thisPlatform = data['ApplicationPlatform']
        intName = data['IntersectionName']
        
    #ran a loop to search for the file matching the below mentioned pattern
    for file in glob.glob('/nojournal/bin/v2x-data/' + intName + '*/' + intName + '_msgCountsLog_*.csv'):
        i = 0
    #file = "static/images/daisy-anthem_msgCountsLog_01012021_000000.csv"
        
    #read the csv file and extracted the required columns and stored in the dataframe "df"   
    col_list = ["log_timestamp_verbose","msg_type","msg_count"]
    df = pd.read_csv(file ,usecols=col_list)
    if(df.empty):
        t2 = 0
        if thisPlatform == "roadside":
            platform = "Infrastructure Side (MRP)"
        elif thisPlatform == "vehicle":
            platform = "Vehicle Side (VSP)"
        df.columns = ["Time","Message","Count"]
        df1 = df2 = df

        
    else:
        #read the csv file again but just the timestamp column and determined the latest refreshed rate
        #stored the dataframe in "rt" and assigned the Time column to "t1" as a list
        rt = pd.read_csv(file , usecols= ["log_timestamp_verbose"])
        t1= rt["log_timestamp_verbose"].tolist()
        #t2 will be the most latest added time
        t2 = max(t1)
        #assigned names to the respective columns
        df.columns = ["Time","Message","Count"]
        #generated additional column to contain the cumulative count grouped by each message type
        df['Cumulative'] = df.groupby(['Message'])['Count'].cumsum(axis=0)
        #sorted the dataframe in descending order to obtain the latest entries on top and assigned to new dataframe "new_df"
        new_df = df.sort_values(['Time'], ascending=False)
        #dropped the rows with duplicate values and kept the latest entry i.e. the first entry of each mssg type
        new_df = new_df.drop_duplicates(subset='Message', keep="first")

        #checking whether to display the MRP or VSP 
        if thisPlatform == "roadside":
            #df1 is the dataframe folding the transmitted table
            df1 = new_df
            #df2 is the dataframe holding the received table
            df2 = new_df
            platform = "Infrastructure Side (MRP)"
            #dropping unrequired rows from both dataframes to separate transmitted and received table
            df1 = df1[df1.Message != 'SRM']
            df1 = df1[df1.Message != 'RemoteBSM']
            df1 = df1[df1.Message != 'HostBSM']
            df2 = df2[df2.Message != 'HostBSM']
            df2 = df2[df2.Message != 'MAP']
            df2 = df2[df2.Message != 'SPaT']
            df2 = df2[df2.Message != 'SSM']
        
        
                

        elif thisPlatform == "vehicle":
            df1 = new_df
            df2 = new_df
            platform = "Vehicle Side (VSP)"
            df2 = df2[df2.Message != 'SRM']
            df2 = df2[df2.Message != 'HostBSM']
            df1 = df1[df1.Message != 'RemoteBSM']
            df1 = df1[df1.Message != 'MAP']
            df1 = df1[df1.Message != 'SPaT']
            df1 = df1[df1.Message != 'SSM']

    #time phase diagrams
    #checking if directory exists
    try:
        #extracting all the filenames from the directory
        diagrams = os.listdir("/nojournal/bin/performance-measurement-diagrams/time-phase-diagram")
        diagrams.sort()
        t_diagrams = []
        diagrams_path = ["/nojournal/bin/performance-measurement-diagrams/time-phase-diagram/"+ diagram for diagram in diagrams]
        for diag_path in diagrams_path:
            im = Image.open(diag_path)
            data = io.BytesIO()
            im.save(data, "JPEG")
            encoded_img_data = base64.b64encode(data.getvalue()).decode('utf-8')
            t_diagrams.append(encoded_img_data) 

       
       
    except:
        diagrams = []
        t_diagrams = []


    #sending the dataframes to HTML template
    return render_template('performance_data.html', platform=platform, time=t2 , tables1=df1.to_html(index=False), tables2=df2.to_html(index=False), diagrams = diagrams, t_diagrams = t_diagrams)



 # page not found 
@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

# internal error page
@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

if __name__ == "__main__":

    app.run(debug=True,host='0.0.0.0')
