#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "json/json.h"
#include "AsnJ2735Lib.h"
#include "locAware.h"
#include "geoUtils.h"

using std::cout;
using std::endl;
using std::fixed;
using std::ifstream;
using std::ofstream;
using std::setprecision;
using std::showpoint;
using std::string;
using std::stringstream;
using std::vector;

class MapEditor
{
private:
    int regionalId{};
    int intersectionId{};
    string intersectionName{};
    string mapPayloadFileName{};
    LocAware *plocAwareLib;

public:
    MapEditor();
    ~MapEditor();
    void readConfigFile();
    void processMapPayLoad();
    void logMsgHex(std::ofstream& OS, const uint8_t* buf, size_t size);
    void logMapPayload(const std::vector<uint8_t>& payload);
};
