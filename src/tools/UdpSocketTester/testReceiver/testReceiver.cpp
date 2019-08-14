#include <iostream>
#include <string.h>

#include "../UdpSocket.h"
#include "BasicVehicle.h"

using std::cout;
using std::endl;
using std::string;

const int RECVPORTNO = 1516;

int main()
{
    UdpSocket receiveSocket(RECVPORTNO,3,0);
    char receiveDataBuffer[5120]{};
    bool isTimeOutOccured = receiveSocket.receiveData(receiveDataBuffer, sizeof(receiveDataBuffer));

    if (isTimeOutOccured != 1)
    {
        BasicVehicle basicVehicle;
        basicVehicle.json2BasicVehicle(string(receiveDataBuffer));

        cout << "TemporaryID: " << basicVehicle.getTemporaryID() << endl;
        cout << "Type: " << basicVehicle.getType() << endl;
        cout << "Latitude: " << basicVehicle.getLatitude_DecimalDegree() << endl;
        cout << "Longitude: " << basicVehicle.getLongitude_DecimalDegree() << endl;
        cout << "Elevation: " << basicVehicle.getElevation_Meter() << endl;
        cout << "Heading:" << basicVehicle.getHeading_Degree() << endl;
        cout << "Speed: " << basicVehicle.getSpeed_MeterPerSecond() << endl;
    }

    else
    {cout << "Timeout occurred!" << endl;}
    
    receiveSocket.closeSocket();
    return 0;
}
