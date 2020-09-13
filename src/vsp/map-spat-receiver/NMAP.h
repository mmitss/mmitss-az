#pragma once

#include <vector>
#include <string>
#include "stdio.h"
#include <iostream>
#include <fstream>
#include "stdlib.h"
#include "math.h"

#ifndef M_PI
    #define M_PI           3.14159265358979323846
#endif

using namespace std;

class N_ElementID
{
 public:
   int Approach;
   int Lane;
   int Node;
};

class N_LaneNodes			// lane nodes
{
public:
    N_ElementID index;
	int ID;
	double E_Offset;
	double N_Offset;
	double Latitude;
	double Longitude;
};

class N_LaneConnection
{
public:
	N_ElementID ConnectedLaneName;
	int Maneuver;
};

class N_ReferenceLane
{
public:
	int LaneID;
	double LateralOffset;
};

class N_Lane
{
public:
	string Lane_Name;
	int ID;
	int Attributes[16];  //16 bits of attributes
	int Connection_No;
	vector<N_LaneConnection> Connections;
	int Direction;  //This is direction of the lane Northbound=1;Eastbound=2;Southbound=3;Westbound=4; reserved! not use now
	int Node_No;
	vector<N_LaneNodes> Nodes;
	N_ReferenceLane RefLane;
	int Type;           //currently all 1: motorized vehicle lane
	int Width;   //in centermeters
};

class N_Approach
{
public:
	int ID;
	int Direction; //not use yet
	int type;      // 1: approach; 2:engress
	int Lane_No;
	vector<N_Lane> Lanes;
};

class N_Intersection
{
public:
	int ID;
	int Attributes[8];  //8 bits of attributes;
	double Ref_Lat;
	double Ref_Long;
	double Ref_Ele;
	int Appro_No;
	vector<N_Approach> Approaches;
	string Map_Name;
	string Rsu_ID;
	//method
};

class NMAP
{
public:
	N_Intersection intersection;
	int ID;
	int Version;
	//method
	int ParseIntersection(char* filename);
};
