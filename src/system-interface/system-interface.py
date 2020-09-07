from flask import Flask, render_template, request
from flask_wtf import FlaskForm
from wtforms import StringField
from wtforms.validators import *
from flask_bootstrap import Bootstrap

app = Flask(__name__)
bootstrap = Bootstrap(app)
app.config['SECRET_KEY'] = '%sq72f#8c$seryfl#2h'

# index
@app.route('/')
def index():
    return render_template('index.html')

# control console (remote)
@app.route('/console')
def console():
    return render_template('console.html')

# control console (local)
@app.route('/local_console')
def local_console():
    import json

    with open('static/json/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)
        intersectionName = data['IntersectionName']
    
    return render_template('local_console.html', intersectionName=intersectionName)

class ConfigurationForm(FlaskForm):
    hostIp = StringField(validators=[ip_address()])
    sourceDsrcDeviceIp = StringField(validators=[ip_address()])
    intersectionName = StringField()
    intersectionID = StringField()
    regionalID = StringField()
    dataCollectorIP = StringField()
    hmiControllerIP = StringField()
    messageDistributorIP = StringField()

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

# configuration viewer / editor
@app.route('/configuration/', methods = ['GET', 'POST'])
@app.route('/configuration/<ip_address>')
def configuration():
    import json

    with open('static/json/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)

    sysConfig = SysConfig(data)    
    form = ConfigurationForm(obj=sysConfig)
    
    return render_template('configuration.html', data=data, form=form)

    # Serializing json  
    #with open('mmitss-phase3-master-config.json', 'w') as json_file:
    #sys_config.hostIPtext = "10.10.10.10"
    #data['HostIp'] = sys_config.hostIPtext
    #data = json.dump(data, son_file) 

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