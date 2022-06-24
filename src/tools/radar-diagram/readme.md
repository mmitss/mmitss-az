# Software Component Description: Radar-Diagram
The radar-diagram component is responsible for generating radar diagram which allows to visualize the performance of different algorithms along different movement.

## Work-flow
The radar-diagram python component reads the csv files (specified in the configuration and generate the diagrams

## Requirements
- complete offline processing of the data files and store it in a csv file

## Configuration
In the `configuration.json` (config) file following keys need to be assigned with appropriate values:
- `NoOfCircle`: no of circle requires to display largest data
- `ytick`: y-axis tick value
- `ytickString`: y-axis tick value for display
- `Offset`: required angels to show the movements

## Known issues/limitations
- None
