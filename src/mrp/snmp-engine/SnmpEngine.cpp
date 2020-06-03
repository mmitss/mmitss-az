# include <iostream>
# include "SnmpEngine.h"

SnmpEngine::SnmpEngine(std::string ip, int port)
{
    char ip_port[64];
    strcpy(ip_port,ip.c_str());
    strcat(ip_port,":");
    strcat(ip_port,(std::to_string(port)).c_str()); 
    

    // Initialize SNMP library
    init_snmp("mmitss");
    snmp_sess_init(&session);
    session.peername = ip_port;
    session.version = SNMP_VERSION_1;
   
    /* set the SNMPv1 community name used for authentication */
    unsigned char comm[] = "public";
    session.community = comm;
    session.community_len = strlen((const char *)session.community);  
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
    {
        snmp_sess_perror("Error in establishing SNMP session! Program will exit now!", &session);
        exit(1);
    }    
}

int SnmpEngine::getValue(std::string inputOid)
{
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;
    int out[50]{};
    int i =0;


    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


if (!snmp_parse_oid(inputOid.c_str(), anOID, &anOID_len))
    {
        std::cout << "No such OID exists!" << std::endl;
    }

    snmp_add_null_var(pdu, anOID, anOID_len);


    /*
    * Send the Request out.
    */
    status = snmp_synch_response(ss, pdu, &response);

    /*
    * Process the response.
    */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
        /*
        * SUCCESS: Print the result variables
        */
        for(vars = response->variables; vars; vars = vars->next_variable)
        {
            int *aa{};
            aa =(int *)vars->val.integer;
            out[i++] = *aa;
        }  
      
    }
    else
    {
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet");
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from the controller!\n");
        else
            snmp_sess_perror("Unknown SNMP Error!", ss);
    }
    ////////////////////////////////////////////////////
    // Free the response for further operations
     if (response)
         snmp_free_pdu(response);
    ////////////////////////////////////////////////////


        return out[0];
}

void SnmpEngine::setValue(std::string inputOid, int value)
{
    pdu = snmp_pdu_create(SNMP_MSG_SET);
    anOID_len = MAX_OID_LEN;
    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


if (!snmp_parse_oid(inputOid.c_str(), anOID, &anOID_len))
    {
        std::cout << "No such OID exists!" << std::endl;
    }

    snmp_add_var(pdu, anOID, anOID_len, 'i', (std::to_string(value)).c_str());


    /*
    * Send the Request out.
    */
    status = snmp_synch_response(ss, pdu, &response);

    /*
    * Process the response.
    */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
    }
    else
    {
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet");
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from the controller!\n");
        else
            snmp_sess_perror("Unknown SNMP Error!", ss);
    }
    ////////////////////////////////////////////////////
    // Free the response for further operations
     if (response)
         snmp_free_pdu(response);
    ////////////////////////////////////////////////////
        
}

SnmpEngine::~SnmpEngine()
{
    // Close the opened session
    snmp_close(ss);
}