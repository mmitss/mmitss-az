/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */

#ifndef byte
#define byte char
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <string>
#include <math.h>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/net-snmp-includes.h"
#include "UdpSocket.h"
#include "json/json.h"
#include "LinkedList.h"
#include "ReqEntry.h"
#include "ManageRequestList.h"
#include "msgEnum.h"
#include "M_PriorityRequestServer.h"

#include "AsnJ2735Lib.h"
#include "locAware.h"
#include "geoUtils.h"
#include "SignalRequest.h"
#include <algorithm>
#include <list>

using namespace std;

// for opening the UDP socket connection
int iSockfd;
struct sockaddr_in sendaddr;
struct sockaddr_in recvaddr;
int addr_length;
char INTport[64];   // Port to connect to traffic signal controller e.g. "501"
char INTip[64];     // IP to connect to traffic signal controller e.g. "150.135.152.23";
double dTime = 0.0; // The reference time, in FIELD it will be gps time, in SIMULATION it will be VISSIM time;

int main(int argc, char *argv[])
{
    int iAddrLeng = 0;
    double dLastETAUpdateTime = 0.0;
    char temp_log[256];
    long lTimeOut = 300000;            // Time out of waiting for a new socket 0.3 sec!
    string Rsu_ID;                     // will get intersection name from "rsuid.txt"
    int ReqListUpdateFlag = NO_UPDATE; // When this flag is positive, it will identify the ReqList is updated. Therefore, the Solver needs to resolve the problem IMPORTANT
    char ConfigFile[256];
    int CombinedPhase[8] = {0};
    char rxMsgBuffer[1024]{};
    bool clearSignalControllerCommands = false; // a flag to clear all commands in the interface when the request passed

    double dCurrentTimeInCycle = 100.0; // For example, if cycle is 100 and offset is 30, this variable will be between [-30 70)
    int PhaseStatus[8];                 // Determine the phase status to generate the split phases
    ssize_t iNumOfRxBytes = 0;
    char tempMsg[1024];
    int IntersectionID{};

    Json::Value jsonObject;
    Json::Reader reader;
    Json::FastWriter fastWriter;
    std::string jsonString;

    std::ifstream jsonconfigfile(INTERSECTION_CONFIG_FILE_JSON);
    std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);
    int iPORT = (jsonObject["PortNumber"]["PriorityRequestServer"]).asInt();

    UdpSocket MsgReceiverSocket(jsonObject["PortNumber"]["PriorityRequestServer_SendSSM"].asInt());
    deleteMapPayloadFile();
    writeMAPPayloadInFile();

    creatLogFile();

    // clear the content of requests.txt and requests_combined.txt
    clearRequestFiles();

    getControllerIPaddress();

    // Get the current RSU ID from "rsuid.txt"
    Rsu_ID = getRSUid();

    //get intersectionID from IntersectionConfig.json
    IntersectionID = getIntersectionID();

    // Read the configinfo_XX.txt from ConfigInfo.txt
    getSignalConfigFile(ConfigFile, CombinedPhase);

    setupConnection(iPORT, lTimeOut);

    readPhaseTimingStatus(PhaseStatus); // First find the unused(enabled) phases if there are any.;

    LinkedList<ReqEntry> req_List; // List of all received requests

    //Main program loop
    while (true)
    {

        dTime = getSystemTime();

        // Read combined request file to see if PRS has set the update flag
        ReqListUpdateFlag = getCurrentFlagInReqFile(REQUESTFILENAME_COMBINED);

        iNumOfRxBytes = recvfrom(iSockfd, rxMsgBuffer, 1024, 0, (struct sockaddr *)&recvaddr,
                                 (socklen_t *)&iAddrLeng);

        // We have received an SRM
        if (iNumOfRxBytes > -1)
        {
            std::string receivedJsonString(rxMsgBuffer);
            //std::cout << receivedJsonString << std::endl;

            processRxMessage(rxMsgBuffer, tempMsg, Rsu_ID, IntersectionID);

            readPhaseTimingStatus(PhaseStatus); // Get the current phase status for determining the split phases

            // Update the Req List data structure considering received message
            UpdateList(req_List, tempMsg, PhaseStatus, ReqListUpdateFlag, CombinedPhase, clearSignalControllerCommands);

            sendSSM(req_List, IntersectionID, MsgReceiverSocket);
        }

        // There are updated ETA's in the request list
        if (((dTime - dLastETAUpdateTime) > COUNT_DOWN_INTERVAL_FOR_ETA) && (req_List.ListSize() > 0))
        {
#ifdef LOGGING
            sprintf(temp_log, "Updated ETAs in the list at time : %.2f \n ", dTime);
            outputlog(temp_log);
#endif

            dLastETAUpdateTime = dTime;

            startUpdateETAofRequestsInList(Rsu_ID, req_List, ReqListUpdateFlag, clearSignalControllerCommands);

            sendSSM(req_List, IntersectionID, MsgReceiverSocket);
        }

        // We need to solve
        if ((ReqListUpdateFlag > NO_UPDATE) && (req_List.ListSize() > 0))
        {
            startUpdateETAofRequestsInList(Rsu_ID, req_List, ReqListUpdateFlag, clearSignalControllerCommands);

#ifdef LOGGING
            sprintf(temp_log, "At time: %.2f. ******** Need to solve ******** \n ", dTime);
            outputlog(temp_log);
#endif
        }

        // If the request list is empty and the last vehicle just passed the intersection
        if (((ReqListUpdateFlag > NO_UPDATE) && (req_List.ListSize() == 0)) || (clearSignalControllerCommands == true))
        {
            ReqListUpdateFlag = NO_UPDATE;

            clearSignalControllerCommands = false;

            startUpdateETAofRequestsInList(Rsu_ID, req_List, ReqListUpdateFlag, clearSignalControllerCommands);

            sendClearCommandsToInterface();

            sendSSM(req_List, IntersectionID, MsgReceiverSocket);
        }
    }

    deleteMapPayloadFile();

    return 0;
}

void processRxMessage(const char *rxMsgBuffer, char tempMsg[], string &Rsu_id, const int IntersectionID)
{
    float fETA{};
    int iRequestedPhase{};
    int iPriorityLevel{};
    int iInLane = 0;
    int iOutLane = 0;

    // the following are not used in PRS or solver. They are simply passed back and forth
    double dMinGrn{};                                                                                          //unused
    int iStartMinute{}, iStartSecond{}, iEndMinute{}, iEndSecond{}, iStartHour{}, iEndHour{}, iVehicleState{}; //unused

    long lvehicleID{};
    int iMsgCnt{};
    long lintersectionID{};
    char temp_log[256]{};
    int iRequestType{};

    std::string receivedSrmJsonString = rxMsgBuffer;

    SignalRequest currentSRM;

    currentSRM.json2SignalRequest(receivedSrmJsonString);

    lintersectionID = currentSRM.getIntersectionID();

    // if the intersection ID in SRM matches the MAP ID of the intersection, SRM should be processed
    if (IntersectionID == lintersectionID)
    {
        iRequestType = currentSRM.getPriorityRequestType(); //request or request_clear

        iInLane = currentSRM.getInBoundLaneID();

        iRequestedPhase = getPhaseInfo(currentSRM);

        switch (currentSRM.getVehicleType())
        {
        case static_cast<int>(MsgEnum::vehicleType::special):
            iPriorityLevel = EV;
            break;
        case static_cast<int>(MsgEnum::vehicleType::bus):
            iPriorityLevel = TRANSIT;
            break;
        case static_cast<int>(MsgEnum::vehicleType::axleCnt4):
            iPriorityLevel = TRUCK;
            break;
        default:
            break;
        }

        fETA = static_cast<float>(currentSRM.getETA_Minute() * 60.0 + currentSRM.getETA_Second());

        lvehicleID = currentSRM.getTemporaryVehicleID();

        iMsgCnt = currentSRM.getMsgCount();

        //KLH - concerned about this being a bug
        //if (iVehicleState == 3) // vehicle is in queue
        //    dMinGrn = (double)(fETA);

        //All of the following are unused in PRS and in Solver
        //dMinGrn iInLane, iOutLane, iStartMinute iStartSecond iStartHour iEndMinute iEndSecond iEndHour iVehicleState
        sprintf(tempMsg, "%d %s %ld %d %.2f %d %.2f %.2f %d %d %d %d %d %d %d %d %d %d %f %ld",
                iRequestType,
                Rsu_id.c_str(),
                lvehicleID,
                iPriorityLevel, fETA, iRequestedPhase, dMinGrn, dTime,
                iInLane, iOutLane, iStartHour, iStartMinute, iStartSecond,
                iEndHour, iEndMinute, iEndSecond,
                iVehicleState, iMsgCnt, 0.0, lintersectionID);

#ifdef LOGGING
        sprintf(temp_log, "........... The Received SRM matches the Intersection ID  ,  at time %.2f. \n", dTime);
        outputlog(temp_log);
        sprintf(temp_log, "%s\t \n", tempMsg);
        outputlog(temp_log);
#endif
    }
}

void startUpdateETAofRequestsInList(const string &rsu_id, LinkedList<ReqEntry> &req_list, int &ReqListUpdateFlag, bool &clearSignalControllerCommands)
{
    bool isCombinedFile{};

    updateETAofRequestsInList(req_list, ReqListUpdateFlag);

    deleteThePassedVehicle(req_list, ReqListUpdateFlag, clearSignalControllerCommands);

    // Write the requests list into requests.txt,
    //isCombinedFile = false;
    PrintList2File(REQUESTFILENAME, rsu_id, req_list, ReqListUpdateFlag, isCombinedFile = false);

    //Write the requests list into  requests_combined.txt;
    //This file will be different than requests.txt when we have EV
    isCombinedFile = true;
    PrintList2File(REQUESTFILENAME_COMBINED, rsu_id, req_list, ReqListUpdateFlag, isCombinedFile = true);

#ifdef LOGGING
    printReqestFile2Log(REQUESTFILENAME_COMBINED);
#endif
}

void sendSSM(LinkedList<ReqEntry> ReqList, const int IntersectionID, UdpSocket MsgReceiverSocket)
{
    Json::Value jsonObject;
    Json::Reader reader;
    Json::FastWriter fastWriter;
    std::string jsonString;

    std::ifstream jsonconfigfile(INTERSECTION_CONFIG_FILE_JSON);
    std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);
    std::string ssmReceiverIP = (jsonObject["HostIp"]).asString();
    int ssmReceiverPort = (jsonObject["PortNumber"]["MessageTransceiver"]["MessageEncoder"]).asInt();

    int listSize = ReqList.ListSize();

    if (!ReqList.ListEmpty())
    {

        jsonObject["MsgType"] = "SSM";
        jsonObject["noOfRequest"] = listSize;
        jsonObject["SignalStatus"]["minuteOfYear"] = 0;
        jsonObject["SignalStatus"]["msOfMinute"] = 0;
        jsonObject["SignalStatus"]["sequenceNumber"] = 0;
        jsonObject["SignalStatus"]["updateCount"] = 0;
        jsonObject["SignalStatus"]["regionalID"] = 0;
        jsonObject["SignalStatus"]["intersectionID"] = IntersectionID;

        ReqList.Reset();

        for (int i = 0; !ReqList.EndOfList(); i++)
        {
            jsonObject["SignalStatus"]["requestorInfo"][i]["vehicleID"] = static_cast<int>(ReqList.Data().VehID);
            jsonObject["SignalStatus"]["requestorInfo"][i]["requestID"] = 0;
            jsonObject["SignalStatus"]["requestorInfo"][i]["msgCount"] = ReqList.Data().iMsgCnt;
            jsonObject["SignalStatus"]["requestorInfo"][i]["basicVehicleRole"] = ReqList.Data().VehClass;
            jsonObject["SignalStatus"]["requestorInfo"][i]["inBoundLaneID"] = ReqList.Data().iInLane;
            jsonObject["SignalStatus"]["requestorInfo"][i]["inBoundApproachID"] = 0;
            jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Minute"] = ReqList.Data().ETA / 60;
            jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Second"] = fmod(ReqList.Data().ETA, 60);
            jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Duration"] = 2000;
            jsonObject["SignalStatus"]["requestorInfo"][i]["priorityRequestStatus"] = ReqList.Data().iRequestType;

            ReqList.Next();
        }

        jsonString = fastWriter.write(jsonObject);
    }
    // cout<<"SSM Json String: "<<jsonString <<endl;
    MsgReceiverSocket.sendData(ssmReceiverIP, ssmReceiverPort, jsonString);
}

void getSignalConfigFile(char *ConfigFile, int *CombinedPhase)
{
    char temp_log[256];
    fstream fs;
    fstream fs_phase; //*** Read in all phases in order to find the combined phase information.***//
    string lineread;
    int phase_num;

    fs.open(CONFIG_INFO_FILE);
    getline(fs, lineread);

    if (lineread.size() != 0)
    {
        sprintf(ConfigFile, "%s", lineread.c_str());

        cout << ConfigFile << endl;

#ifdef LOGGING
        outputlog(ConfigFile);
#endif

        fs_phase.open(ConfigFile);

        getline(fs_phase, lineread); //*** Read the first line to get the number of all phases.
        sscanf(lineread.c_str(), "%*s %d ", &phase_num);

        getline(fs_phase, lineread); //*** Read the second line of the combined phase into array CombinedPhase[8]
        //*** If the phase exsits, the value is not 0; if not exists, the default value is '0'.
        sscanf(lineread.c_str(), "%*s %d %d %d %d %d %d %d %d",
               &CombinedPhase[0], &CombinedPhase[1], &CombinedPhase[2], &CombinedPhase[3],
               &CombinedPhase[4], &CombinedPhase[5], &CombinedPhase[6], &CombinedPhase[7]);

        fs_phase.close();
    }
    else
    {

        sprintf(temp_log, "Problem reading configuration file %s ", CONFIG_INFO_FILE);
        outputlog(temp_log);

        exit(0);
    }

    fs.close();
}

void readPhaseTimingStatus(int PhaseStatus[8])
{
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len;
    netsnmp_variable_list *vars;
    int status;
    init_snmp("ASC");         //Initialize the SNMP library
    snmp_sess_init(&session); //Initialize a "session" that defines who we're going to talk to
    // set up defaults
    //char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
    //char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
    char ipwithport[64];
    strcpy(ipwithport, INTip);
    strcat(ipwithport, ":");
    strcat(ipwithport, INTport); //for ASC get status, DO NOT USE port!!!
    session.peername = strdup(ipwithport);
    session.version = SNMP_VERSION_1; //for ASC intersection  set the SNMP version number
    // set the SNMPv1 community name used for authentication "public";
    char *strPtr = const_cast<char *>("public");
    session.community = reinterpret_cast<u_char *>(strPtr);
    session.community_len = strlen((const char *)session.community);
    SOCK_STARTUP;
    ss = snmp_open(&session); // establish the session
    if (!ss)
    {
        snmp_sess_perror("ASC", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    // Create the PDU for the data for our request.
    //  1) We're going to GET the system.sysDescr.0 node.

    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;

    //---#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

    char ctemp[50];

    sprintf(ctemp, "%s", PHASE_GROUP_STATUS_GREEN);
    // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
    if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
    {
        snmp_perror(ctemp);
        SOCK_CLEANUP;
        exit(1);
    }
    snmp_add_null_var(pdu, anOID, anOID_len);

    sprintf(ctemp, "%s", PHASE_GROUP_STATUS_RED);
    // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
    if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
    {
        snmp_perror(ctemp);
        SOCK_CLEANUP;
        exit(1);
    }
    snmp_add_null_var(pdu, anOID, anOID_len);

    sprintf(ctemp, "%s", PHASE_GROUP_STATUS_YELLOW);
    // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
    if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
    {
        snmp_perror(ctemp);
        SOCK_CLEANUP;
        exit(1);
    }
    snmp_add_null_var(pdu, anOID, anOID_len);

    sprintf(ctemp, "%s", PHASE_GROUP_STATUS_NEXT);
    if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
    // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
    {
        snmp_perror(ctemp);
        SOCK_CLEANUP;
        exit(1);
    }
    snmp_add_null_var(pdu, anOID, anOID_len);

    //Send the Request out.

    status = snmp_synch_response(ss, pdu, &response);

    //Process the response.

    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {

        //SUCCESS: Print the result variables

        int *out = new int[MAX_CONTROLLER_OUTPUT_VALUES];
        int j = 0;
        //~ for(vars = response->variables; vars; vars = vars->next_variable)
        //~ print_variable(vars->name, vars->name_length, vars);

        // manipuate the information ourselves
        for (vars = response->variables; vars; vars = vars->next_variable)
        {
            if (vars->type == ASN_OCTET_STR)
            {
                char *sp = (char *)malloc(1 + vars->val_len);
                memcpy(sp, vars->val.string, vars->val_len);
                sp[vars->val_len] = '\0';
                //printf("value #%d is a string: %s\n", count++, sp);
                free(sp);
            }
            else
            {

                int *aa;
                aa = (int *)vars->val.integer;
                out[j++] = *aa;
                //printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
            }
        }
        //GET the results from controller
        int iColor[2][8]{}; // iColor[1][*]  has the current phase color, and iColor[0][*] has the previous signal status phase color.
        int phaseColor[8]{};

        identifyColor(iColor, out[0], out[1], out[2], out[3]);
        whichPhaseIsGreen(phaseColor, out[0], out[1], out[2]);
        for (int i = 0; i < MAX_NO_OF_PHASES; i++)
        {
            // MZP ??? Needs more thought to get out the right split time	phase_read.phaseColor[i]=phaseColor[i];
            PhaseStatus[i] = iColor[1][i];
        }
        cout << "phasecolor" << endl;
        for (int i = 0; i < MAX_NO_OF_PHASES; i++)
        {
            cout << phaseColor[i] << " ";
        }
        cout << endl;

        cout << "iColor" << endl;
        for (int i = 0; i < MAX_NO_OF_PHASES; i++)
        {
            cout << iColor[1][i] << " ";
        }
        cout << "  " << endl;

        cout << "Signal Gourp green " << out[0] << "  red " << out[1] << " yellow " << out[2] << " next " << out[3]
             << endl;
        delete out;
    }

    else
    {
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet\nReason: %s\n",
                    snmp_errstring(response->errstat));
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
                    session.peername);
        else
            snmp_sess_perror("snmpdemoapp", ss);
    }

    // Clean up:    *  1) free the response.   *  2) close the session.

    if (response)
        snmp_free_pdu(response);

    snmp_close(ss);

    SOCK_CLEANUP;
}

void identifyColor(int i_Color[2][8], int greenGroup, int redGroup, int yellowGroup, int nextPhase)
{

    // numbers 2, 4,5,6 and 7 were the output of ASC/3 proprietry NTCIP OID for different status of a phase.   ( PHASE_STA_TIME2_ASC in the MIB.h )
    // 4 is for red phase, 7 for green 6 for yellow, 2 for the next and 5 for a phase that just gets red after being yellow.
    // Since the application was originally written based on this proprietary OID, we tranfer the obtained signal status out of red, green, yellow next group NTCIP OIDs to it.
    // ????	for(int iii=0; iii<8; iii++)
    //	{
    // ???		if (ConfigIS.Gmax[iii]==0 || (( iii == ConfigIS.MissPhase[0] && ConfigIS.MissPhase[0]>=0) || (iii == ConfigIS.MissPhase[1] && ConfigIS.MissPhase[1]>=0) ) )  // in case phase iii is missing, the max is zero and i_Color gets 3. Since the Econolite proprietory OID, returns the  missing phase as 3, we need this conversion.  We do not want to use vendors proprietary OID
    //			i_Color[1][iii]=3;
    //		else
    //			i_Color[1][iii]=4;
    //	}
    if (redGroup != 255) // all phases are not red
    {
        if (greenGroup != 0) // at least one phase is green
        {
            switch (greenGroup)
            {
            case 1:
                i_Color[1][0] = 7; // phase 1 is green
                break;
            case 2:
                i_Color[1][1] = 7; //phase 2 is green
                break;
            case 4:
                i_Color[1][2] = 7;
                break;
            case 8:
                i_Color[1][3] = 7;
                break;
            case 16:
                i_Color[1][4] = 7;
                break;
            case 32:
                i_Color[1][5] = 7;
                break;
            case 64:
                i_Color[1][6] = 7;
                break;
            case 128:
                i_Color[1][7] = 7;
                break;

            case 17:
                i_Color[1][0] = 7; // phase 1 is green
                i_Color[1][4] = 7; // phase 5 is green
                break;
            case 33:
                i_Color[1][0] = 7; //phase 2 is green
                i_Color[1][5] = 7;
                break;
            case 18:
                i_Color[1][1] = 7;
                i_Color[1][4] = 7;
                break;
            case 34:
                i_Color[1][1] = 7;
                i_Color[1][5] = 7;
                break;
            case 68:
                i_Color[1][2] = 7;
                i_Color[1][6] = 7;
                break;
            case 132:
                i_Color[1][2] = 7;
                i_Color[1][7] = 7;
                break;
            case 72:
                i_Color[1][3] = 7;
                i_Color[1][6] = 7;
                break;
            case 136:
                i_Color[1][7] = 7;
                i_Color[1][3] = 7;
                break;
            }
        }
        if (yellowGroup != 0) // at least one phase is yellow
        {
            switch (yellowGroup)
            {
            case 1:
                i_Color[1][0] = 6;
                break;
            case 2:
                i_Color[1][1] = 6;
                break;
            case 4:
                i_Color[1][2] = 6;
                break;
            case 8:
                i_Color[1][3] = 6;
                break;
            case 16:
                i_Color[1][4] = 6;
                break;
            case 32:
                i_Color[1][5] = 6;
                break;
            case 64:
                i_Color[1][6] = 6;
                break;
            case 128:
                i_Color[1][7] = 6;
                break;

            case 17:
                i_Color[1][0] = 6;
                i_Color[1][4] = 6;
                break;
            case 33:
                i_Color[1][0] = 6;
                i_Color[1][5] = 6;
                break;
            case 18:
                i_Color[1][1] = 6;
                i_Color[1][4] = 6;
                break;
            case 34:
                i_Color[1][1] = 6;
                i_Color[1][5] = 6;
                break;
            case 68:
                i_Color[1][2] = 6;
                i_Color[1][6] = 6;
                break;
            case 132:
                i_Color[1][2] = 6;
                i_Color[1][7] = 6;
                break;
            case 72:
                i_Color[1][3] = 6;
                i_Color[1][6] = 6;
                break;
            case 136:
                i_Color[1][7] = 6;
                i_Color[1][3] = 6;
                break;
            }
        }

        if (nextPhase != 0) // at least one phase is yellow and the next phase after it is known
        {
            switch (nextPhase)
            {
            case 1:
                i_Color[1][0] = 2;
                break;
            case 2:
                i_Color[1][1] = 2;
                break;
            case 4:
                i_Color[1][2] = 2;
                break;
            case 8:
                i_Color[1][3] = 2;
                break;
            case 16:
                i_Color[1][4] = 2;
                break;
            case 32:
                i_Color[1][5] = 2;
                break;
            case 64:
                i_Color[1][6] = 2;
                break;
            case 128:
                i_Color[1][7] = 2;
                break;

            case 17:
                i_Color[1][0] = 2;
                i_Color[1][4] = 2;
                break;
            case 33:
                i_Color[1][0] = 2;
                i_Color[1][5] = 2;
                break;
            case 18:
                i_Color[1][1] = 2;
                i_Color[1][4] = 2;
                break;
            case 34:
                i_Color[1][1] = 2;
                i_Color[1][5] = 2;
                break;
            case 68:
                i_Color[1][2] = 2;
                i_Color[1][6] = 2;
                break;
            case 132:
                i_Color[1][2] = 2;
                i_Color[1][7] = 2;
                break;
            case 72:
                i_Color[1][3] = 2;
                i_Color[1][6] = 2;
                break;
            case 136:
                i_Color[1][7] = 2;
                i_Color[1][3] = 2;
                break;
            }
        }

        // put the current status into the previous status
        for (int iii = 0; iii < 8; iii++)
            i_Color[0][iii] = i_Color[1][iii];
    }
    else // redGroup==255   , we need this else condition to distinguish which one of the current red phases are just turned to red from yellow
    {
        for (int iii = 0; iii < 8; iii++)
        {
            if (i_Color[0][iii] == 6) // previously was yellow
                i_Color[1][iii] = 5;  // 5 means the phase was yellow and now it is red
            else if (i_Color[0][iii] == 2)
                i_Color[1][iii] = 2;
            else
                i_Color[1][iii] = i_Color[0][iii];
        }
    }
}

void whichPhaseIsGreen(int phase_Color[8], int greenGroup, int redGroup,
                       int yellowGroup) // this function return the color of the first argument which is phaseNo.
{
    for (int i = 0; i < 8; i++)
        phase_Color[i] = REAL_RED_STATUS;

    if (redGroup != 255) // if redGroup=255 therfore all phases are RED
    {
        if (greenGroup != 0) // at least one phase is green
        {
            switch (greenGroup)
            {
            case 1:
                phase_Color[0] = REAL_GREEN_STATUS; // phase 1 is green
                break;
            case 2:
                phase_Color[1] = REAL_GREEN_STATUS; //phase 2 is greeen
                break;
            case 4:
                phase_Color[2] = REAL_GREEN_STATUS;
                break;
            case 8:
                phase_Color[3] = REAL_GREEN_STATUS;
                break;
            case 16:
                phase_Color[4] = REAL_GREEN_STATUS;
                break;
            case 32:
                phase_Color[5] = REAL_GREEN_STATUS;
                break;
            case 64:
                phase_Color[6] = REAL_GREEN_STATUS;
                break;
            case 128:
                phase_Color[7] = REAL_GREEN_STATUS;
                break;

            case 17:
                phase_Color[0] = REAL_GREEN_STATUS; // phase 1 is green
                phase_Color[4] = REAL_GREEN_STATUS; // phase 5 is green
                break;
            case 33:
                phase_Color[0] = REAL_GREEN_STATUS; //phase 2 is green
                phase_Color[5] = REAL_GREEN_STATUS;
                break;
            case 18:
                phase_Color[1] = REAL_GREEN_STATUS;
                phase_Color[4] = REAL_GREEN_STATUS;
                break;
            case 34:
                phase_Color[1] = REAL_GREEN_STATUS;
                phase_Color[5] = REAL_GREEN_STATUS;
                break;
            case 68:
                phase_Color[2] = REAL_GREEN_STATUS;
                phase_Color[6] = REAL_GREEN_STATUS;
                break;
            case 132:
                phase_Color[2] = REAL_GREEN_STATUS;
                phase_Color[7] = REAL_GREEN_STATUS;
                break;
            case 72:
                phase_Color[3] = REAL_GREEN_STATUS;
                phase_Color[6] = REAL_GREEN_STATUS;
                break;
            case 136:
                phase_Color[7] = REAL_GREEN_STATUS;
                phase_Color[3] = REAL_GREEN_STATUS;
                break;
            }
        }
        if (yellowGroup != 0) // at least one phase is green
        {
            switch (yellowGroup)
            {
            case 1:
                phase_Color[0] = REAL_YELLOW_STATUS; // phase 1 is green
                break;
            case 2:
                phase_Color[1] = REAL_YELLOW_STATUS; //phase 2 is green
                break;
            case 4:
                phase_Color[2] = REAL_YELLOW_STATUS;
                break;
            case 8:
                phase_Color[3] = REAL_YELLOW_STATUS;
                break;
            case 16:
                phase_Color[4] = REAL_YELLOW_STATUS;
                break;
            case 32:
                phase_Color[5] = REAL_YELLOW_STATUS;
                break;
            case 64:
                phase_Color[6] = REAL_YELLOW_STATUS;
                break;
            case 128:
                phase_Color[7] = REAL_YELLOW_STATUS;
                break;

            case 17:
                phase_Color[0] = REAL_YELLOW_STATUS; // phase 1 is green
                phase_Color[4] = REAL_YELLOW_STATUS; // phase 5 is green
                break;
            case 33:
                phase_Color[0] = REAL_YELLOW_STATUS; //phase 2 is green
                phase_Color[5] = REAL_YELLOW_STATUS;
                break;
            case 18:
                phase_Color[1] = REAL_YELLOW_STATUS;
                phase_Color[4] = REAL_YELLOW_STATUS;
                break;
            case 34:
                phase_Color[1] = REAL_YELLOW_STATUS;
                phase_Color[5] = REAL_YELLOW_STATUS;
                break;
            case 68:
                phase_Color[2] = REAL_YELLOW_STATUS;
                phase_Color[6] = REAL_YELLOW_STATUS;
                break;
            case 132:
                phase_Color[2] = REAL_YELLOW_STATUS;
                phase_Color[7] = REAL_YELLOW_STATUS;
                break;
            case 72:
                phase_Color[3] = REAL_YELLOW_STATUS;
                phase_Color[6] = REAL_YELLOW_STATUS;
                break;
            case 136:
                phase_Color[7] = REAL_YELLOW_STATUS;
                phase_Color[3] = REAL_YELLOW_STATUS;
                break;
            }
        }
    }
}

void sendClearCommandsToInterface()
{
    byte tmp_event_data[500];
    int size = 0;
    char *event_data;
    char temp_log[256];

    packEventList(tmp_event_data, size);

    event_data = new char[size];

    for (int i = 0; i < size; i++)
        event_data[i] = tmp_event_data[i];

    if (sendto(iSockfd, event_data, size + 1, 0, (struct sockaddr *)&sendaddr, addr_length))
    {
#ifdef LOGGING
        sprintf(temp_log,
                " The Event List sent to SignalControllerInterface to delete all previous commands, The size is %d  \n",
                size);
        outputlog(temp_log);
#endif
    }

    delete event_data;
}

// void getRSUid(string rsu_id)
string getRSUid() //Debashis: changed it to return as string
{
    string rsu_id;
    fstream fs;
    fs.open(RSUID_FILENAME);
    char temp[128];

    getline(fs, rsu_id);

    if (rsu_id.size() != 0)
    {
#ifdef LOGGING
        sprintf(temp, " RSU ID %s\n", rsu_id.c_str());
        cout << temp << endl;
        outputlog(temp);
#endif
    }
    else
    {
        sprintf(temp, "Reading RSUID.txt file problem.\n");
        cout << temp << endl;
        outputlog(temp);
        exit(0);
    }

    fs.close();
    return rsu_id;
}

void getControllerIPaddress()
{
    fstream fs;
    fs.open(IPINFO_FILENAME);
    string lineread;
    getline(fs, lineread);
    if (lineread.size() != 0)
    {
        sprintf(INTip, "%s", lineread.c_str());
        cout << "Controller IP Address is " << INTip << endl;
        getline(fs, lineread);
        sprintf(INTport, "%s", lineread.c_str());
        cout << "Controller PORT Address is " << INTport << endl;
    }
    else
    {
        cout << "A problem in reading IPinfo.txt file problem" << endl;
        exit(0);
    }
    fs.close();
}

void printReqestFile2Log(const char *resultsfile)
{
    char temp_log[256];
    fstream fss;
    fss.open(resultsfile, fstream::in);
    if (!fss)
    {
        cout << "***********Error opening the plan file in order to print to a log file!\n";
        sprintf(temp_log, "***********Error opening the plan file in order to print to a log file!\n");
        outputlog(temp_log);
        exit(1);
    }
    string lineread;
    sprintf(temp_log, " Content of request files  :");
    outputlog(temp_log);
    while (!fss.eof())
    {
        getline(fss, lineread);
        strcpy(temp_log, lineread.c_str());
        strcat(temp_log, " ");
        outputlog(temp_log);
    }
    fss.close();
}

void packEventList(char *tmp_event_data, int &size)
{
    int offset = 0;
    byte *pByte; // pointer used (by cast)to get at each byte
    // of the shorts, longs, and blobs
    // byte    tempByte;   // values to hold data once converted to final format
    unsigned short tempUShort;
    long tempLong;
    //header 2 bytes
    tmp_event_data[offset] = 0xFF;
    offset += 1;
    tmp_event_data[offset] = 0xFF;
    offset += 1;
    //MSG ID: 0x03 for signal event data send to Signal Control Interface
    tmp_event_data[offset] = 0x03;
    offset += 1;
    //No. events in R1
    int numberOfPhase = 4;
    int tempTime = 0;
    int tempCmd = 3;
    tempUShort = (unsigned short)numberOfPhase;
    pByte = (byte *)&tempUShort;
    tmp_event_data[offset + 0] = (byte) * (pByte + 1);
    tmp_event_data[offset + 1] = (byte) * (pByte + 0);
    offset = offset + 2;
    //Events in R1
    for (int iii = 0; iii < 4; iii++)
    {
        //Time
        tempLong = (long)(tempTime);
        pByte = (byte *)&tempLong;
        tmp_event_data[offset + 0] = (byte) * (pByte + 3);
        tmp_event_data[offset + 1] = (byte) * (pByte + 2);
        tmp_event_data[offset + 2] = (byte) * (pByte + 1);
        tmp_event_data[offset + 3] = (byte) * (pByte + 0);
        offset = offset + 4;
        //phase
        tempUShort = (unsigned short)iii + 1;
        pByte = (byte *)&tempUShort;
        tmp_event_data[offset + 0] = (byte) * (pByte + 1);
        tmp_event_data[offset + 1] = (byte) * (pByte + 0);
        offset = offset + 2;
        //action
        tempUShort = (unsigned short)tempCmd;
        pByte = (byte *)&tempUShort;
        tmp_event_data[offset + 0] = (byte) * (pByte + 1);
        tmp_event_data[offset + 1] = (byte) * (pByte + 0);
        offset = offset + 2;
    }

    tempUShort = (unsigned short)numberOfPhase;
    pByte = (byte *)&tempUShort;
    tmp_event_data[offset + 0] = (byte) * (pByte + 1);
    tmp_event_data[offset + 1] = (byte) * (pByte + 0);
    //Events in R
    offset = offset + 2;
    for (int iii = 0; iii < 4; iii++)
    {
        //Time
        tempLong = (long)(tempTime);
        pByte = (byte *)&tempLong;
        tmp_event_data[offset + 0] = (byte) * (pByte + 3);
        tmp_event_data[offset + 1] = (byte) * (pByte + 2);
        tmp_event_data[offset + 2] = (byte) * (pByte + 1);
        tmp_event_data[offset + 3] = (byte) * (pByte + 0);
        offset = offset + 4;
        //phase
        tempUShort = (unsigned short)iii + 5;
        pByte = (byte *)&tempUShort;
        tmp_event_data[offset + 0] = (byte) * (pByte + 1);
        tmp_event_data[offset + 1] = (byte) * (pByte + 0);
        offset = offset + 2;
        //action
        tempUShort = (unsigned short)tempCmd;
        pByte = (byte *)&tempUShort;
        tmp_event_data[offset + 0] = (byte) * (pByte + 1);
        tmp_event_data[offset + 1] = (byte) * (pByte + 0);
        offset = offset + 2;
    }
    size = offset;
}

int outputlog(char *output) // JD 12.2.11
{
    FILE *stream = fopen(LOG_FILENAME, "r");
    if (stream == NULL)
    {
        perror("Error opening file");
    }
    fseek(stream, 0L, SEEK_END);
    long endPos = ftell(stream);
    fclose(stream);
    fstream fs;
    if (endPos < 50000000)
        fs.open(LOG_FILENAME, ios::out | ios::app);
    else
        fs.open(LOG_FILENAME, ios::out | ios::trunc);
    if (!fs || !fs.good())
    {
        cout << "could not open file!\n";
        return -1;
    }
    fs << output;
    if (fs.fail())
    {
        cout << "failed to append to file!\n";
        return -1;
    }
    fs.close();
    cout << output << endl;
    return 1;
}

void xTimeStamp(char *pc_TimeStamp_)
{
    struct tm *ps_Time;
    time_t i_CurrentTime;
    char ac_TmpStr[256];

    i_CurrentTime = time(NULL);
    ps_Time = localtime(&i_CurrentTime);

    //year
    sprintf(ac_TmpStr, "%d", ps_Time->tm_year + 1900);
    strcpy(pc_TimeStamp_, ac_TmpStr);

    //month
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_mon + 1);
    strcat(pc_TimeStamp_, ac_TmpStr);

    //day
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_mday);
    strcat(pc_TimeStamp_, ac_TmpStr);

    //hour
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_hour);
    strcat(pc_TimeStamp_, ac_TmpStr);

    //min
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_min);
    strcat(pc_TimeStamp_, ac_TmpStr);

    //sec
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_sec);
    strcat(pc_TimeStamp_, ac_TmpStr);
}

double getSystemTime()
{
    struct timeval tv_tt;
    gettimeofday(&tv_tt, NULL);
    return tv_tt.tv_sec + tv_tt.tv_usec / 1.0e6;
}

void clearRequestFiles()
{
    system("\\rm /nojournal/bin/requests.txt");
    FILE *fp_req = fopen(REQUESTFILENAME, "w");
    fprintf(fp_req, "Num_req -1 0\n");
    fclose(fp_req);
    system("\\rm /nojournal/bin/requests_combined.txt");
    fp_req = fopen(REQUESTFILENAME_COMBINED, "w");
    fprintf(fp_req, "Num_req -1 0\n");
    fclose(fp_req);
}

int msleep(unsigned long milisec)
{
    struct timespec req = {0};
    time_t sec = (int)(milisec / 1000);
    milisec = milisec - (sec * 1000);
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    while (nanosleep(&req, &req) == -1)
        continue;
    return 1;
}

void setupConnection(int &iPort, long lTimeOut)
{

    // PRS sends a clear command to the traffic interface when the last priority vehicle passes the intersection
    int iPRStoInterfacePort = 44444;
    struct timeval tv; // struct for socket timeout: 1s
    tv.tv_sec = 0;
    tv.tv_usec = lTimeOut;
    addr_length = sizeof(recvaddr);

    // -------------------------Network Connection for receiving SRM from VISSIM--------//
    if ((iSockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("iSockfd");
        exit(1);
    }

    int optval = 1;
    setsockopt(iSockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    //Setup time out
    if (setsockopt(iSockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        perror("Error");

    // set up sending socket to interface to clean all commands when there is no request in the table
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_port = htons(iPort);
    recvaddr.sin_addr.s_addr = INADDR_ANY; //inet_addr("10.254.56.255") ;;
    memset(recvaddr.sin_zero, '\0', sizeof recvaddr.sin_zero);
    if (bind(iSockfd, (struct sockaddr *)&recvaddr, sizeof recvaddr) == -1)
    {
        perror("bind");
        exit(1);
    }

    // PRS sends a mesage to traffic control interface to delete all the commands when there is no request in the request table
    sendaddr.sin_port = htons(iPRStoInterfacePort);    //htons(PortOfInterface);
    sendaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY; // //INADDR_BROADCAST;
    memset(sendaddr.sin_zero, '\0', sizeof sendaddr.sin_zero);
}

void creatLogFile()
{
    //------log file name with Time stamp---------------------------
    char pctimestamp[128]; // time stamp for logging
    char log_filestr[256] = LOG_FILENAME;
    xTimeStamp(pctimestamp);
    strcat(log_filestr, pctimestamp);
    strcat(log_filestr, ".log");
    fstream ftemp;
    ftemp.open(LOG_FILENAME, fstream::out);
    if (!ftemp.good())
    {
        perror("Open logfilename failed!");
        exit(1);
    }
    else
    {
        ftemp << "Start logging PRS at time:\t" << time(NULL) << endl;
        ftemp.close();
    }
}

int getIntersectionID(void)
{
    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream jsonconfigfile(INTERSECTION_CONFIG_FILE_JSON);

    std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);

    return (jsonObject["IntersectionID"]).asInt();
}

int getPhaseInfo(SignalRequest signalRequest)
{
    int phaseNo{};
    bool singleFrame = false;
    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream jsonconfigfile(INTERSECTION_CONFIG_FILE_JSON);

    std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);

    int intersectionID = (jsonObject["IntersectionID"]).asInt();
    int regionalID = (jsonObject["RegionalID"]).asInt();
    // std::string fmap = (jsonObject["IntersectionInfo"]["mapFileDirectory"]).asString();
    std::string intersectionName = (jsonObject["IntersectionName"]).asString();
    std::string fmap = "/nojournal/bin/" + intersectionName + ".map.payload";

    LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
    int approachID = plocAwareLib->getApproachIdByLaneId(regionalID, intersectionID, static_cast<uint8_t>(signalRequest.getInBoundLaneID()));
    phaseNo = unsigned(plocAwareLib->getControlPhaseByIds(regionalID, intersectionID, approachID,
                                                          static_cast<uint8_t>(signalRequest.getInBoundLaneID())));

    delete plocAwareLib;
    return phaseNo;
}

// int getPriorityRequestStatus(LinkedList<ReqEntry> ReqList)
// {
//     int priorityRequestStatus{};
    
//     std::list<ReqEntry>::iterator FindEV_VehClassInList = std::find_if(std::begin(ReqList), std::end(ReqList), [&](ReqEntry const &p) { return p.VehClass == 2; });
//     std::list<ReqEntry>::iterator FindTransit_VehClassInList = std::find_if(std::begin(ReqList), std::end(ReqList), [&](ReqEntry const &p) { return p.VehClass == 1; });
//     std::list<ReqEntry>::iterator FindTruck_VehClassInList = std::find_if(std::begin(ReqList), std::end(ReqList), [&](ReqEntry const &p) { return p.VehClass == 3; });

//     if(!ReqList.empty())
//     {
//         for (size_t i = 0; i < ReqList.size(); i++)
//         {
//             if(ReqList[i].VehClass == 2 )
//             {
//                 priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
//             }

//             else if(ReqList[i].VehClass == 1 && FindEV_VehClassInList == ReqList.end())
//             {
//                 priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
//             }

//             else if(ReqList[i].VehClass == 1 && FindEV_VehClassInList != ReqList.end())
//             {
//                 priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::rejected);
//             }

//             else if(ReqList[i].VehClass == 3 && FindEV_VehClassInList == ReqList.end() && FindTransit_VehClassInList == ReqList.end())
//             {
//                 priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
//             }

//             else if(ReqList[i].VehClass == 3 && FindEV_VehClassInList != ReqList.end())
//             {
//                 priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::rejected);
//             }
//         }
//     }
	
//     return priorityRequestStatus;
// }

void writeMAPPayloadInFile()
{
    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream jsonconfigfile(INTERSECTION_CONFIG_FILE_JSON);

    std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);

    std::string intersectionName = (jsonObject["IntersectionName"]).asString();
    std::string mapPayload = (jsonObject["MapPayload"]).asString();

    const char *path = "/nojournal/bin";
    std::stringstream ss;
    ss << path;
    std::string s;
    ss >> s;
    std::ofstream outputfile;

    outputfile.open(s + "/" + intersectionName + ".map.payload");

    outputfile << "payload"
               << " "
               << intersectionName << " " << mapPayload << std::endl;
    outputfile.close();
}

void deleteMapPayloadFile()
{
    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream jsonconfigfile(INTERSECTION_CONFIG_FILE_JSON);

    std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);

    std::string intersectionName = (jsonObject["IntersectionName"]).asString();

    std::string deleteFileName = "/nojournal/bin/" + intersectionName + ".map.payload";
    remove(deleteFileName.c_str());
}