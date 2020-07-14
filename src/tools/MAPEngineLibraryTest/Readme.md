# Test Overview:
- This test program is made for testing *MAP Engine Library*. The test program will verify whether MAP Engine Library can locate vehicles accurately in MAP or not. 
- The test program will read an encoded mappayload from json configuration file and store it as ".payload" format in /map folder.
- Python test program (gpsCoordinates.py) is written for generating gps coordinates as input of "testMEL.cpp" program
- Python test program (melPlotTest.py) is written for visualization of the output of "testMEL.cpp" program

Step1:
- Define all the attributes value in "mapEngineLibraryTestConfig.json" file related to map and intersection info.

Step 2:
- Run "GPSCoordinates.py" to create gps coordinates(Latitude, Longitude, Elevation, and Heading)
- A txt format file 'GPSCoordinates.txt' will be created on the subsequent folder which will be the input of "testMEL.cpp" program. 

Step 3:
- Run "make clean" and "make linux" command.
- After the compilation process, the binary executables 'testMEL' will be created
- Run "testMEL" executable file which create local Local_GPS_Coordinates.txt

Step 4:
- Run "melPlotTest.py" to create visualization of the MapEngine Library test program 
