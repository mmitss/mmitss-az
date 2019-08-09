#include<iostream>
#include <fstream>
#include <cstring>
#include "json.h"
#include "UdpSocket.h"
#include "BasicVehicle.h"

using std::ifstream;
using std::cout;
using std::endl;

int main()
{
    ifstream json_file("BSM.json");
    std::string str((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
    BasicVehicle basicVehicle;
    basicVehicle.json2BasicVehicle(str);
    cout << "Elevation: " << basicVehicle.getElevation_Meter() << endl;
    return 0;
}