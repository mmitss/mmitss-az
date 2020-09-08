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

def prepareJSONData(data, form):
    data['HostIp']              = form.hostIp.data
    data['SourceDsrcDeviceIp']  = form.sourceDsrcDeviceIp.data
    data['IntersectionName']    = form.intersectionName.data
    data['IntersectionID']      = form.intersectionID.data
    data['RegionalID']          = form.regionalID.data
    data['DataCollectorIP']     = form.dataCollectorIP.data
    data['HMIControllerIP']     = form.hmiControllerIP.data
    data['MessageDistributorIP']= form.messageDistributorIP.data

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