/***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************
SnmpEngine.h
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
SnmpEngine utilizes the c++ net-snmp library that facilitates the interaction with 
target Snmp devices. This class provides an API for the following:
 (1) Establishing the SNMP session with the target SNMP device (in the constructor)
 (2) Process and execute the SnmpSet or SnmpGet request.

Complete official tutorial on building a C++ application for SnmpSet and SnmpGet
requests is available at: 
http://net-snmp.sourceforge.net/wiki/index.php/TUT:Simple_Application

***************************************************************************************/
#pragma once

// Import the headers required to utilise Net-Snmp library:
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <fstream>
# include "json/json.h"

class SnmpEngine
{
    private:
        
        // A variable to store an Snmp session. Same session will be reused for all requests
        struct snmp_session session, *ss;

        // A variables to  send out the Snmp request and  store the received variables from the Snmp request
        struct variable_list *vars;
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;

        // Variables to store the OID
        size_t anOID_len = MAX_OID_LEN;
        oid anOID[MAX_OID_LEN];
        int status;
        
        bool consoleOutput = false;
        bool logging = false;
        std::ofstream logFile{};

    public:
        SnmpEngine(std::string ip, int port);
        int processSnmpRequest(std::string requestType, std::string inputOid, int value);
        void initializeLogFile(Json::Value jsonObject_config);
        void logAndOrDisplay(std::string logString);
        ~SnmpEngine();
};