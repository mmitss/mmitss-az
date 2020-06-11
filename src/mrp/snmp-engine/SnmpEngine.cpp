# include <iostream>
# include <bits/stdc++.h> 
# include <chrono>
# include "SnmpEngine.h"

// The constructor of the SnmpEngine class establishes an SNMP session with the target SNMP device. 
SnmpEngine::SnmpEngine(std::string ip, int port)
{
    // Check if the target SNMP device is in the network by executing a ping test.
    std::cout << "Target device IP address: " << ip << std::endl;
    std::cout << "Target device NTCIP port: " << port << std::endl;
    std::cout << "\nVerifying the network connection with the target SNMP device" << std::endl;
    std::string pingCommand = "ping -c 1 " + ip;
    int pingStatus = system(pingCommand.c_str());  
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