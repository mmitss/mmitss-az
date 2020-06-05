# pragma once

# include <string>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

class SnmpEngine
{
    private:
        struct snmp_session session, *ss;
        struct variable_list *vars;
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;
        size_t anOID_len = MAX_OID_LEN;
        oid anOID[MAX_OID_LEN];
        int status;


    public:
        SnmpEngine(std::string ip, int port);
        int processSnmpRequest(std::string requestType, std::string inputOid, int value);
        ~SnmpEngine();
};