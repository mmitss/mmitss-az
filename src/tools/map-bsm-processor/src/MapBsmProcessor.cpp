/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    main.cpp
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:

*/

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unistd.h>
#include <iomanip>
#include "json/json.h"
#include "MapEngine.h"
#include "CSV.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Please provide the path/name of raw BSM data file and the config file in the argument" << std::endl;
        return 0;
    }
    
    std::string inputFilename = argv[1];
    std::ifstream inputFile(inputFilename);

    std::string configFilename = argv[2];

    std::string outputFilename = std::regex_replace(inputFilename, std::regex(".csv"), "_processed.csv");
    std::ofstream outputFile(outputFilename);

    outputFile << "log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,temporaryId,secMark,latitude,longitude,elevation,speed,heading,type,length,width,position_on_map,current_approach,current_lane,current_signal_group,dist_to_stopbar\n";

    double log_timestamp_posix{},timestamp_posix{}, secMark{}, elevation{}, speed{}, heading{}, length{}, width{};
    double latitude{}, longitude{};
    int temporaryId{}, type{};

    MapEngine mapEngine(configFilename);

    bool isOnMap{};
    std::string onmapCsv{};

    int index{};
    
    for(CSVIterator loop(inputFile); loop != CSVIterator(); ++loop)
    {
        if (index > 0)
        {
            
            std::string log_timestamp_verbose{(*loop)[0]};            

            std::string log_timestamp_posix_str{(*loop)[1]};            
            log_timestamp_posix = std::stod(log_timestamp_posix_str);

            std::string timestamp_verbose{(*loop)[2]};            

            std::string timestamp_posix_str{(*loop)[3]};            
            timestamp_posix = std::stod(timestamp_posix_str);

            std::string temporaryId_str{(*loop)[4]};            
            temporaryId = std::stoi(temporaryId_str);

            std::string secMark_str{(*loop)[5]};            
            secMark = std::stod(secMark_str);

            std::string latitude_str{(*loop)[6]};            
            latitude = std::stod(latitude_str);

            std::string longitude_str{(*loop)[7]};            
            longitude = std::stod(longitude_str);

            std::string elevation_str{(*loop)[8]};            
            elevation = std::stod(elevation_str);

            std::string speed_str{(*loop)[9]};            
            speed = std::stod(speed_str);

            std::string heading_str{(*loop)[10]};            
            heading = std::stod(heading_str);

            std::string type_str{(*loop)[11]};            
            type = std::stoi(type_str);

            std::string length_str{(*loop)[12]};            
            length = std::stod(length_str);

            std::string width_str{(*loop)[13]};            
            width = std::stod(width_str);

            isOnMap = mapEngine.isVehicleOnMap(latitude, longitude, elevation, speed, heading);
            
            if(isOnMap==true)
            {
                onmapCsv = mapEngine.getOnMapCsv(latitude, longitude, elevation, speed, heading);
                outputFile << std::fixed << std::setprecision(3) << log_timestamp_verbose << "," << log_timestamp_posix << "," << timestamp_verbose << "," << timestamp_posix << "," << temporaryId << "," << secMark << ",";
                outputFile << std::fixed << std::setprecision(8) << latitude << "," << longitude << ",";
                outputFile << std::fixed << std::setprecision(3) << elevation << "," << speed << "," << heading << "," << type << "," << length << "," << width << "," << onmapCsv << "\n";
            }
        }
        index++;
    }

    inputFile.close();
    outputFile.close();
    
    return 0;
}