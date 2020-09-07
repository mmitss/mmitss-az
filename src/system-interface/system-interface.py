from flask import Flask, render_template, request
from flask_bootstrap import Bootstrap

app = Flask(__name__)
bootstrap = Bootstrap(app)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/console')
def console():
    return render_template('console.html')

@app.route('/local_console')
def local_console():
    import json

    with open('static/json/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)
        intersectionName = data['IntersectionName']
    
    return render_template('local_console.html', intersectionName=intersectionName)

@app.route('/configuration/')
@app.route('/configuration/<ip_address>')
#def configuration(ip_address=''):
def configuration():
    import json

    with open('static/json/mmitss-phase3-master-config.json') as json_file:
        data = json.load(json_file)
    
    return render_template('configuration.html', data=data)

    # Serializing json  
    #with open('mmitss-phase3-master-config.json', 'w') as json_file:
    #sys_config.hostIPtext = "10.10.10.10"
    #data['HostIp'] = sys_config.hostIPtext
    #data = json.dump(data, son_file) 

@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html'), 500

if __name__ == "__main__":

    app.run(debug=True)