/***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************
SnmpEngine.cpp
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

# include <iostream>
# include <bits/stdc++.h> 
# include <chrono>
# include <sstream>
# include "SnmpEngine.h"
# include "Timestamp.h"

/* Instantiates an object of the SnmpEngine class and establishes an SNMP session 
   with the target SNMP device. 
 - arguments:
    (1) IP address of the target SNMP device (std::string)
    (2) NTCIP port of the target SNMP device. (int)
*/

SnmpEngine::SnmpEngine(std::string ip, int port)
{
    Json::Value jsonObject_config;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject_config, &errors);        
    delete reader;
    configJson.close();

    logging = jsonObject_config["Logging"].asBool();
    consoleOutput = jsonObject_config["ConsoleOutput"].asBool();

    if(logging==true)
    {
        initializeLogFile(jsonObject_config);
    }

        
    logAndOrDisplay(("Target device IP address: " + ip));
    logAndOrDisplay(("Target device NTCIP port: " + std::to_string(port)));
    logAndOrDisplay("Verifying the network connection with the target SNMP device...");
    logAndOrDisplay("PING test begins");

    
    // Check if the target SNMP device is in the network by executing a ping test.
    std::string pingCommand = "ping -c 1 " + ip;
    int pingStatus = system(pingCommand.c_str());  
    
    // If the target Snmp device is not reachable in the network, the ping test will return -1.
    if (-1 != pingStatus) 
    { 
        bool ping_ret = WEXITSTATUS(pingStatus);

        if(ping_ret==0)
        {
            logAndOrDisplay("PING test successful");
            logAndOrDisplay("Successfully verified the network connection with the target SNMP device");
        }
        else
        {
            if(consoleOutput==true)
            {
                logAndOrDisplay("PING test failed");
                logAndOrDisplay("Unable to verify the network connection with the target SNMP device. The program will exit now.");
            }
            exit(0);
        }
    }

    // Bring the IP address and port of the target SNMP device in the required form, which is "IPADDRESS:PORT":
    char ip_port[64];
    strcpy(ip_port,ip.c_str());
    strcat(ip_port,":");
    strcat(ip_port,(std::to_string(port)).c_str()); 
    
    // Initialize the SNMP library. Lets call this library "mmitss". We will use the SNMP version=1:
    init_snmp("mmitss");
    snmp_sess_init(&session);
    session.peername = ip_port;
    session.version = SNMP_VERSION_1;
   
    // Establish the session parameters.
    unsigned char comm[] = "public";
    session.community = comm;
    session.community_len = strlen((const char *)session.community);  

    // After we have established the session, we then need to open it. 
    // Opening it returns a pointer to another session that we should use for all our future calls: 
    ss = snmp_open(&session);                    

    // For some reason, the session opens even if there is no communicatiosn with the target SNMP device.
    // However, if the session fails to open, throw an error and exit from the program.
    if (!ss)
    {
        logAndOrDisplay("Error in opening SNMP session! Program will exit now.");
        exit(1);
    }
    else
    {
        logAndOrDisplay("Ready to forward SNMP GET/SET requests");
    }
}

void SnmpEngine::initializeLogFile(Json::Value jsonObject_config)
{
    std::string intersectionName = jsonObject_config["IntersectionName"].asString();

    time_t now = time(0);
    struct tm tstruct;
    char logFileOpenningTime[80];
    tstruct = *localtime(&now);
    strftime(logFileOpenningTime, sizeof(logFileOpenningTime), "%m%d%Y_%H%M%S", &tstruct);

    std::string logfileName = "/nojournal/bin/log/" + intersectionName + "_snmpEngineLog_" + logFileOpenningTime + ".log";
    logFile.open(logfileName);
}

void SnmpEngine::logAndOrDisplay(std::string logString)
{
    double timestamp = getPosixTimestamp();

    if(consoleOutput==true)
    {
        std::cout << "[" << std::fixed << std::showpoint << std::setprecision(4) << timestamp << "] ";
        std::cout << logString << std::endl;
    }

    if(logging==true)
    {
        logFile << "[" << std::fixed << std::showpoint << std::setprecision(4) << timestamp << "] ";
        logFile << logString << std::endl;
    }
}

/* 
    Processes the SnmpSet or SnmpGet request received in a JSON string.
    - Arguments:
        (1) Type of request. Supported requests: SnmpGet and SnmpSet
        (2) OID for which a particular request needs to be processed
        (3) A value that needs to be set for the OID. For SnmpGet request, value=1.
*/
int SnmpEngine::processSnmpRequest(std::string requestType, std::string inputOid, int value)
{
    
    // Transform the request type string to lowercase
    std::transform(requestType.begin(), requestType.end(), requestType.begin(), ::tolower); 
    
    // Create an appropriate PDU based on the type of request:    
    if (requestType == "get")
        pdu = snmp_pdu_create(SNMP_MSG_GET);
    else if (requestType == "set")
        pdu = snmp_pdu_create(SNMP_MSG_SET);

    // Read the input OID into the anOID variable:
    read_objid(inputOid.c_str(), anOID, &anOID_len);

    // If this is a get request, add null variable, else add the appropriate value
    if (requestType == "get")
        snmp_add_null_var(pdu, anOID, anOID_len);
    else if (requestType == "set")
        snmp_add_var(pdu, anOID, anOID_len, 'i', (std::to_string(value)).c_str());
    
    // Now the request is ready. Send the request out:
        status = snmp_synch_response(ss, pdu, &response);

    // Process the response
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
        if (requestType == "get")
        {
            int out[50]{};
            int i{};
            for(vars = response->variables; vars; vars = vars->next_variable)
            {
                int *aa{};
                aa =(int *)vars->val.integer;
                out[i++] = *aa;
                value = out[0];
            }  
            logAndOrDisplay(("SUCCESS in GET for OID:" + inputOid + "; Value=" + std::to_string(value)));
        }
        else
        {
            logAndOrDisplay(("SUCCESS in SET for OID:" + inputOid + "; Value=" + std::to_string(value)));
        }
        
    }
    else // Identify the reason of failure
    {
        if (status == STAT_SUCCESS)
        {
            logAndOrDisplay(("Error in packet. Reason:" + static_cast<std::string>(snmp_errstring(static_cast<int>(response->errstat)))));
        }
        else if (status == STAT_TIMEOUT)
            {
                logAndOrDisplay("Timeout: No response from the target SNMP device");
            }
        else
            {
                logAndOrDisplay("Unknown SNMP Error");
            }
        if(requestType == "get")
            {
                logAndOrDisplay(("FAILURE in GET for OID:" + inputOid + "; Value=" + std::to_string(value)));
            }
        else
        {
            logAndOrDisplay(("FAILURE in SET for OID:" + inputOid + "; Value=" + std::to_string(value)));
        }
        value = -1;

    }
    
    // Free the response for further operations
     if (response)
     {
         snmp_free_pdu(response);
         anOID_len = MAX_OID_LEN;
     }
    return value;
}


SnmpEngine::~SnmpEngine()
{
    logFile.close();
    // Close the opened session
    snmp_close(ss);
}

