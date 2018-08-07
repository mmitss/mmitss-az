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

class ElementID
{
 public:
   int Approach;
   int Lane;
   int Node;
};

class LaneNodes			// lane nodes
{
public:
    ElementID index;
	int ID;
	double E_Offset;
	double N_Offset;
	double Latitude;
	double Longitude;
	float Heading;  //this is the node heading
};

class LaneConnection
{
public:
	ElementID ConnectedLaneName;
	int Maneuver;
};

class ReferenceLane
{
public:
	int LaneID;
	double LateralOffset;
};

class Lane
{
public:
	string Lane_Name;
	int ID;
	int Attributes[16];  //16 bits of attributes
	int Connection_No;
	vector<LaneConnection> Connections;
	int Direction;  //This is direction of the lane Northbound=1;Eastbound=2;Southbound=3;Westbound=4; reserved! not use now
	int Node_No;
	vector<LaneNodes> Nodes;
	ReferenceLane RefLane;
	int Type;           //currently all 1: motorized vehicle lane
	int Width;   //in centermeters
};

class Approach
{
public:
	int ID;
	int Direction; //not use yet
	int type;      // 1: approach; 2:engress
	int Lane_No;
	vector<Lane> Lanes;
};

class Intersection
{
public:
	int ID;
	int Attributes[8];  //8 bits of attributes;
	double Ref_Lat;
	double Ref_Long;
	double Ref_Ele;
	int Appro_No;
	vector<Approach> Approaches;
	string Map_Name;
	string Rsu_ID;
	//method
};

class MAP
{
public:
	Intersection intersection;
	int ID;
	int Version;
	vector<LaneNodes> MAP_Nodes;
	//method
	int ParseIntersection(char* filename);
};
