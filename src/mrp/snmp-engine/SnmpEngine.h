# pragma once

# include <string>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

class SnmpEngine
{
    private:
        struct snmp_session session, *ss;
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;

        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;

        struct variable_list *vars;
        int status;
   
    public:
        SnmpEngine(std::string ip, int port);
        int getValue(std::string oid);
        void setValue(std::string oid, int value);
        ~SnmpEngine();
};