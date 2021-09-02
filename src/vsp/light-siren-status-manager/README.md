# Software Component Description: Light-Siren-Status-Manager
The light-siren-status-manager component is responsible for managing the light-siren status information of emergency vehicle. The display of the automation-hat mini shows the status of light-siren interface.

## Work-flow
The light-siren-status-manager checks the input of the automationhat mini installed on the vsp (raspberry pi). If the light-siren is ON, it will displays a green signal on the display for input 2 otherwise it will be red for input 3. It sends the light-siren ON/OFF status message to the priority-request-generator (PRG) for the emergency vehicles.

### Managing light-siren status
The light-siren-status-manager continuously checks for the input of the automationhat mini. If the vehicle is used for emergency purpose a voltage signal will be received by the automationat mini of the VSP. The light-siren-status-manager sends a JSON formatted light-siren status messgae to the PRG, if the status changes. An example of such JSON formatted messages is as follows:
```
{   
    "MsgType":"LightSirenStatusMessage",
    "LightSirenStatus": "ON"
}
```

## Console output and logging
The light-siren-status-manager does not generate any log files. The console output also provides some important information about the status of the component. The console output can be redirected to a file using supervisor if mmitss is running inside container. The following information is displayed in the console:
- JSON formatted light-siren-status message

## Requirements
- None

## Configuration
In the `mmitss-phase3-master-config.json` (config) file following keys need to be assigned with appropriate values:
- `config["PortNumber"]["LightSirenStatusManager"]`:  UDP port number (integer) 

## Test
A basic test of the light-siren-status-manager software can be done by using a tool (lightSirenStatusReceiver.py script) reside on mmitss/src/vsp/light-siren-status-manager/test directory. The software has to be run inside the VSP which has arm processor. The automationhat mini is required to be installed on it.

## Known issues/limitations
- None