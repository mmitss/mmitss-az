#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <cstring>
#include "json.h"
#include <cassert>
#include <iterator>
#include <algorithm>
#include <jsoncpp/json/json.h>

using namespace std;

// int main()
// {
//     ifstream ifs;
//     ifs.open("output.json");
//     assert(ifs.is_open());

//     Json::Reader reader;
//     Json::Value root;
//     if (!reader.parse(ifs, root, false))
//     {
//         cout << "not parsed" << endl;
//         return -1;
//     }

//     for (Json::Value::iterator it = root.begin(); it != root.end(); ++it) {
//   int str = (*it)["msgCount"].asInt();
//   cout << str << endl;
// }

//     return 0;
// }

int main()
{


    Json::Value root;               // Json root
    Json::Reader parser;            // Json parser
    ifstream ifs;
    ifs.open("output.json");
    //assert(ifs.is_open());
    int noOfRequest;
    string laneID = "inBoundLaneID";
    

    // Json content
    // std::string strCarPrices ="{ \"Car \": [{\"Toyota\":\"$393,695\", \"BMW\":\"$40,250\",\"Porsche\":\"$59,000\",\"Koenigsegg Agera\":\"$2.1 Million\"}]},{\"Car Prices\": [{\"Aventador\":\"$393,695\", \"BMW\":\"$40,250\",\"Porsche\":\"$59,000\",\"Koenigsegg Agera\":\"$2.1 Million\"}]}";
   // std::string strCarPrices = "{\"MmitssSignalRequest\":[{\"intersectionID\":\"1003\",\"regionalID\":\"0\",\"intersectionID\":\"1005\"}]}";
    // std::ifstream jsonfile("srm_input.json");
    // std::string strCarPrices((std::istreambuf_iterator<char>(jsonfile)),
    // std::istreambuf_iterator<char>());
    
    
    // Parse the json
    bool bIsParsed = parser.parse( ifs, root );
    if (bIsParsed == true)
    {   
        cout<<"parsed successfully"<<endl;
        noOfRequest = (root["noOfRequest"]).asInt();
        int laneid[noOfRequest];
        // Get the values
        const Json::Value values = root["MmitssSignalStatus"]["requestorInfo"];

        // Print the objects
        //for ( int i = 0; i < values.size(); i++ )
        for ( int i = 0; i < noOfRequest; i++ )
        {
            // Print the values
            //cout << values[i] << endl;

            // Print the member names and values individually of an object
            for(int j = 0; j < values[i].getMemberNames().size(); j++)
            {
                // Member name and value
                cout << values[i].getMemberNames()[j] << ": " << values[i][values[i].getMemberNames()[j]].asString() << endl;
                if (values[i].getMemberNames()[j]==laneID)
                    laneid[i] = values[i][values[i].getMemberNames()[j]].asInt();
            }
            cout<<"LaneID: "<<laneid[0]<<endl;
            cout<<"LaneID: "<<laneid[1]<<endl;
            cout<<"LaneID: "<<laneid[2]<<endl;
        }
    }
    else
    {
        cout << "Cannot parse the json content!" << endl;
    }
    return 0;   
}