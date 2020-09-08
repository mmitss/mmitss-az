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

from flask import Flask, render_template, request, flash
from flask_wtf import FlaskForm
from wtforms import StringField, validators
from wtforms.validators import *
from flask_bootstrap import Bootstrap

# Initialize application
app = Flask(__name__)
bootstrap = Bootstrap(app)
app.config['SECRET_KEY'] = '%sq72f#8c$seryfl#2h'

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

    with open('static/json/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)
        pageTitle = data['IntersectionName']
    
    return render_template('local_console.html', pageTitle=pageTitle)

# Configuration Viewer / Editor combined form
class ConfigurationForm(FlaskForm):
    hostIp = StringField(validators=[ip_address()])
    sourceDsrcDeviceIp = StringField(validators=[ip_address()])
    intersectionName = StringField()
    intersectionID = StringField()
    regionalID = StringField([validators.Length(min=2, max=5)])
    dataCollectorIP = StringField()
    hmiControllerIP = StringField()
    messageDistributorIP = StringField()
    priorityRequestGeneratorServerIP = StringField('Priority Request Generator Server IP Address')
    vehicleType = StringField('Vehicle Type')
    logging = StringField('Logging')
    srmTimedOutTime = StringField('SRM Timed Out Time')
    portNumberMTMessageSender = StringField('Port Number: Message Transceiver / Message Sender')
    portNumberMTMessageReceiver = StringField('Port Number: Message Transceiver / Message Receiver')
    portNumberMTMessageEncoder = StringField('Port Number: Message Transceiver / Message Encoder')
    portNumberMTMessageDecoder = StringField('Port Number: Message Transceiver / Message Decoder')
    portNumberMessageDistributor = StringField('Port Number: Message Distributor')
    portNumberRSMDecoder        = StringField('Port Number: RSM Decoder')
    portNumberOBUBSMReceiver = StringField('Port Number: OBU BSM Receiver')
    portNumberHostBsmDecoder = StringField('Port Number: Host BSM Decoder')
    portNumberTrajectoryAware = StringField('Port Number: Trajectory Aware')
    portNumberPriorityRequestServer = StringField('Port Number: Priority Request Server')
    portNumberPrioritySolver = StringField('Port Number: Priority Solver')
    portNumberPriorityRequestGenerator = StringField('Port Number: Priority Request Generator')
    portNumberTrafficControllerInterface = StringField('Port Number: Traffic Controller Interface')
    portNumberTrafficControllerCurrPhaseListener = StringField('Port Number: Traffic Controller Current Phase Listener')
    portNumberTrafficControllerTimingPlanSender = StringField('Port Number: Traffic Controller Timing Plan Sender')
    portNumberPerformanceObserver = StringField('Port Number: Performance Observer')

# System Configuration data object
class SysConfig:
    def __init__(self, data):
        self.hostIp = data['HostIp']
        self.sourceDsrcDeviceIp = data['SourceDsrcDeviceIp']
        self.intersectionName = data['IntersectionName']
        self.intersectionID = data['IntersectionID']
        self.regionalID = data['RegionalID']
        self.dataCollectorIP = data['DataCollectorIP']
        self.hmiControllerIP = data['HMIControllerIP']
        self.messageDistributorIP = data['MessageDistributorIP']
        self.priorityRequestGeneratorServerIP = data['PriorityRequestGeneratorServerIP']
        self.vehicleType = data['VehicleType']
        self.logging = data['Logging']
        self.srmTimedOutTime = data['SRMTimedOutTime']
        self.portNumberMTMessageSender = data['PortNumber']['MessageTransceiver']['MessageSender']
        self.portNumberMTMessageReceiver = data['PortNumber']['MessageTransceiver']['MessageReceiver']
        self.portNumberMTMessageEncoder = data['PortNumber']['MessageTransceiver']['MessageEncoder']
        self.portNumberMTMessageDecoder = data['PortNumber']['MessageTransceiver']['MessageDecoder']
        self.portNumberMessageDistributor = data['PortNumber']['MessageDistributor']
        self.portNumberRSMDecoder = data['PortNumber']['RsmDecoder']
        self.portNumberOBUBSMReceiver = data['PortNumber']['OBUBSMReceiver']
        self.portNumberHostBsmDecoder = data['PortNumber']['HostBsmDecoder']
        self.portNumberTrajectoryAware = data['PortNumber']['TrajectoryAware']
        self.portNumberPriorityRequestServer = data['PortNumber']['PriorityRequestServer']
        self.portNumberPrioritySolver = data['PortNumber']['PrioritySolver']
        self.portNumberPriorityRequestGenerator = data['PortNumber']['PriorityRequestGenerator']
        self.portNumberTrafficControllerInterface = data['PortNumber']['TrafficControllerInterface']
        self.portNumberTrafficControllerCurrPhaseListener = data['PortNumber']['TrafficControllerCurrPhaseListener']
        self.portNumberPerformanceObserver = data['PortNumber']['PerformanceObserver']

def prepareJSONData(data, form):
    data['HostIp']              = form.hostIp.data
    data['SourceDsrcDeviceIp']  = form.sourceDsrcDeviceIp.data
    data['IntersectionName']    = form.intersectionName.data
    data['IntersectionID']      = form.intersectionID.data
    data['RegionalID']          = form.regionalID.data
    data['DataCollectorIP']     = form.dataCollectorIP.data
    data['HMIControllerIP']     = form.hmiControllerIP.data
    data['MessageDistributorIP']= form.messageDistributorIP.data
    data['PriorityRequestGeneratorServerIP']= form.priorityRequestGeneratorServerIP.data
    data['VehicleType']= form.vehicleType.data
    data['Logging']= form.logging.data
    data['SRMTimedOutTime']= form.srmTimedOutTime.data
    data['PortNumber']['MessageTransceiver']['MessageSender']= form.portNumberMTMessageSender.data
    data['PortNumber']['MessageTransceiver']['MessageReceiver']= form.portNumberMTMessageReceiver.data
    data['PortNumber']['MessageTransceiver']['MessageEncoder']= form.portNumberMTMessageEncoder.data
    data['PortNumber']['MessageTransceiver']['MessageDecoder']= form.portNumberMTMessageDecoder.data
    data['PortNumber']['MessageDistributor']    = form.portNumberMessageDistributor.data
    data['PortNumber']['RsmDecoder']    = form.portNumberRSMDecoder.data
    data['PortNumber']['OBUBSMReceiver']    = form.portNumberOBUBSMReceiver.data
    data['PortNumber']['HostBsmDecoder']    = form.portNumberHostBsmDecoder.data
    data['PortNumber']['TrajectoryAware']    = form.portNumberTrajectoryAware.data
    data['PortNumber']['PriorityRequestServer']    = form.portNumberPriorityRequestServer.data
    data['PortNumber']['PrioritySolver']    = form.portNumberPrioritySolver.data
    data['PortNumber']['PriorityRequestGenerator']    = form.portNumberPriorityRequestGenerator.data
    data['PortNumber']['TrafficControllerInterface']    = form.portNumberTrafficControllerInterface.data
    data['PortNumber']['TrafficControllerCurrPhaseListener']    = form.portNumberTrafficControllerCurrPhaseListener.data
    data['PortNumber']['PerformanceObserver']    = form.portNumberPerformanceObserver.data

# configuration viewer / editor
@app.route('/configuration/', methods = ['GET', 'POST'])
def configuration():
    import json

    with open('static/json/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)

    sysConfig = SysConfig(data)    
    pageTitle = data['IntersectionName']
    form = ConfigurationForm(obj=sysConfig)

    #if request.method == 'POST' and form.validate():
    if request.method == 'POST':
        # Serialize the edited data
        with open('static/json/mmitss-phase3-master-config.json', 'w') as json_file:
            prepareJSONData(data, form)
            dataResult = json.dump(data, json_file) 
            flash('Configuration Updated')  
    
    return render_template('configuration.html', pageTitle=pageTitle, form=form)

 # page not found 
@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

# internal error page
@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

if __name__ == "__main__":

    app.run(debug=True)