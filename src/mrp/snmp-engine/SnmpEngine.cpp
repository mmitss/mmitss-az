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
# include "SnmpEngine.h"

/* Instantiates an object of the SnmpEngine class and establishes an SNMP session 
   with the target SNMP device. 
 - arguments:
    (1) IP address of the target SNMP device (std::string)
    (2) NTCIP port of the target SNMP device. (int)
*/
SnmpEngine::SnmpEngine(std::string ip, int port)
{
    // Check if the target SNMP device is in the network by executing a ping test.
    std::cout << "Target device IP address: " << ip << std::endl;
    std::cout << "Target device NTCIP port: " << port << std::endl;
    std::cout << "\nVerifying the network connection with the target SNMP device" << std::endl;
    
    std::string pingCommand = "ping -c 1 " + ip;
    int pingStatus = system(pingCommand.c_str());  
    
    // If the target Snmp device is not reachable in the network, the ping test will return -1.
    if (-1 != pingStatus) 
    { 
        bool ping_ret = WEXITSTATUS(pingStatus);

        if(ping_ret==0)
            std::cout << "\nSuccessfully verified the network connection with the target SNMP device!" << std::endl; 
        else
        {
            std::cout<<"Unable to verify the network connection with the target SNMP device.\nThe program will exit now."<< std::endl;
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
        snmp_sess_perror("Error in opening SNMP session! Program will exit now.", &session);
        exit(1);
    }
    else
    {
        std::cout << "Ready to forward SNMP GET/SET requests." << std::endl;
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
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
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
            std::cout << "SUCCESS in GET for OID:" << inputOid << " at time: " << timenow << ". Value=" << value << std::endl;
        }
        else
        {
            std::cout << "SUCCESS in SET for OID:" << inputOid << " at time: " << timenow << ". Value=" << value << std::endl;
        }
        
    }
    else // Identify the reason of failure
    {
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet. Reason: %s\n",snmp_errstring(response->errstat));     
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from the target SNMP device.\n");
        else
            snmp_sess_perror("Unknown SNMP Error!\n", ss);
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if(requestType == "get")
            std::cout << "FAILURE in GET for OID:" << inputOid << " at time:" << timenow << std::endl;
        else
        {
            std::cout << "FAILURE in SET for OID:" << inputOid << " at time: " << timenow << ". Value=" << value << std::endl;
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
    // Close the opened session
    snmp_close(ss);
}

/*
    IMPORTANT NOTE:
    ---------------

    In MMITSS, only the required portion of NetSnmp libraries is precompiled. 
    Due to this, the some MIBs are not present in this custom distribution. 
    Therefore, at the start of the application following warnings are expected:
    
    =============== WARNINGS START ===============

    MIB search path: /home/nvaltekar/.snmp/mibs:/usr/local/share/snmp/mibs
    Cannot find module (SNMPv2-MIB): At line 0 in (none)
    Cannot find module (IF-MIB): At line 0 in (none)
    Cannot find module (IP-MIB): At line 0 in (none)
    Cannot find module (TCP-MIB): At line 0 in (none)
    Cannot find module (UDP-MIB): At line 0 in (none)
    Cannot find module (HOST-RESOURCES-MIB): At line 0 in (none)
    Cannot find module (NOTIFICATION-LOG-MIB): At line 0 in (none)
    Cannot find module (DISMAN-EVENT-MIB): At line 0 in (none)
    Cannot find module (DISMAN-SCHEDULE-MIB): At line 0 in (none)
    Cannot find module (UCD-SNMP-MIB): At line 0 in (none)
    Cannot find module (UCD-DEMO-MIB): At line 0 in (none)
    Cannot find module (SNMP-TARGET-MIB): At line 0 in (none)
    Cannot find module (NET-SNMP-AGENT-MIB): At line 0 in (none)
    Cannot find module (HOST-RESOURCES-TYPES): At line 0 in (none)
    Cannot find module (SNMP-FRAMEWORK-MIB): At line 0 in (none)
    Cannot find module (SNMP-MPD-MIB): At line 0 in (none)
    Cannot find module (SNMP-USER-BASED-SM-MIB): At line 0 in (none)
    Cannot find module (SNMP-VIEW-BASED-ACM-MIB): At line 0 in (none)
    Cannot find module (SNMP-COMMUNITY-MIB): At line 0 in (none)
    Cannot find module (IPV6-ICMP-MIB): At line 0 in (none)
    Cannot find module (IPV6-MIB): At line 0 in (none)
    Cannot find module (IPV6-TCP-MIB): At line 0 in (none)
    Cannot find module (IPV6-UDP-MIB): At line 0 in (none)
    Cannot find module (IP-FORWARD-MIB): At line 0 in (none)
    Cannot find module (NET-SNMP-PASS-MIB): At line 0 in (none)
    Cannot find module (NET-SNMP-EXTEND-MIB): At line 0 in (none)
    Cannot find module (UCD-DLMOD-MIB): At line 0 in (none)
    Cannot find module (SNMP-NOTIFICATION-MIB): At line 0 in (none)
    Cannot find module (SNMPv2-TM): At line 0 in (none)
    Cannot find module (NET-SNMP-VACM-MIB): At line 0 in (none)

    =============== WARNINGS END ===============

    These warnings do not affect the working of this applciation, as we do not use any 
    of the MIBs that are not available. If one wishes to suppress these warnings,
    set the following environment variable:
    MIBS=ALL

    this can be accomplished by entering following command in the terminal before starting 
    the application:
    export MIBS=ALL
*/