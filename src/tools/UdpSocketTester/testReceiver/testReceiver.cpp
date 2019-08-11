#include <iostream>
#include <string.h>

#include "UdpSocket.h"
#include "BasicVehicle.h"

using std::cout;
using std::endl;
using std::string;

const int RECVPORTNO = 1516;

int main()
{
    UdpSocket receiveSocket(RECVPORTNO);
    char receiveDataBuffer[5120]{};
    receiveSocket.receiveData(receiveDataBuffer, sizeof(receiveDataBuffer));

    BasicVehicle basicVehicle;
    basicVehicle.json2BasicVehicle(string(receiveDataBuffer));

    cout << "TemporaryID: " << basicVehicle.getTemporaryID() << endl;
    cout << "Type: " << basicVehicle.getType() << endl;
    cout << "Latitude: " << basicVehicle.getLatitude_DecimalDegree() << endl;
    cout << "Longitude: " << basicVehicle.getLongitude_DecimalDegree() << endl;
    cout << "Elevation: " << basicVehicle.getElevation_Meter() << endl;
    cout << "Heading:" << basicVehicle.getHeading_Degree() << endl;
    cout << "Speed: " << basicVehicle.getSpeed_MeterPerSecond() << endl;

    receiveSocket.closeSocket();
    return 0;
}
