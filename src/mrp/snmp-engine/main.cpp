# include <iostream>
# include <string>
#include "SnmpEngine.h"

int main()
{
    SnmpEngine snmp("10.12.6.102", 501);
    std::string oid = "1.3.6.1.4.1.1206.3.5.2.9.44.1.1";
    std::cout << snmp.getValue(oid) << std::endl;
    snmp.setValue(oid, 0);
    std::cout << snmp.getValue(oid) << std::endl;
    return 0;
}