# Software Component Description: Time-Space-Diagram
The time-space-diagram component is responsible for generating time-space diagram which allows to visualize the performance of coordination strategy.

## Work-flow
The time-space-diagram-tool python component calls an instance of SpatDataManager class to process all the SPaT files and BSMDataManager class to process all the BSM files. Finally the timeSpaceDiagramMethod() method from TimeSpaceDiagramManager class will be called to generate the time-space diagram

## Requirements
- complete offline processing of the BSM files using MAP-BSM-Processor component before using this tool

## Configuration
In the `configuration.json` (config) file following keys need to be assigned with appropriate values:
- `DeisredSignalGroup`: a list that contains phase number of the desired coordinated movements for each intersection
- `StartTimeOfDiagram`: specify the start time of the diagram (get the log_timestamp_posix or timestamp_posix value from a SPaT file)
- `EndTimeOfDiagram`: specify the end time of the diagram (get the log_timestamp_posix or timestamp_posix value from a SPaT file)
- `NoOfIntersection`: specify the number of intersection for which the diagram will be generated
- `IntersectionName`: a list that contains name of each intersection   
- `SPaTFileDirectory`: a list that contains absolute directory of the SPaT files 
- `BsmFileDirectory`: a list that contains absolute directory of the BSM files 
- `IntersectionDistance`: a list that contains the distance of each intersection from the origin of the diagram

## Known issues/limitations
- None
