#pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <istream>
#include <math.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "Signal.h"
#include "Array.h"

using namespace std;
//------------------START OF DEFINITION--------------------------------//
//ASC INTERSECTION MIB :NTCIP 1202
//1.3.6.1.4.1.1206.4.1.3.1.1.3
#define RED_GROUP		 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1"                     //Object
#define YELLOW_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1"
#define GREEN_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1"
#define DONOTWALK_GROUP  	"1.3.6.1.4.1.1206.4.2.1.1.4.1.5.1"
#define PEDCLEAR_GROUP 	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.6.1"
#define WALK_GROUP 		 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.7.1"
#define VEHICLE_CALL	 	"1.3.6.1.4.1.1206.4.2.1.1.4.1.8.1"
#define PEDES_CALL	 		"1.3.6.1.4.1.1206.4.2.1.1.4.1.8.1"  // AT this time just use vehicle call

//Phase control
#define MIB_PHASE_HOLD 	 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.4.1"
#define MIB_PHASE_FORCEOFF 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.5.1"
#define MIB_PHASE_OMIT 	 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.2.1"
#define MIB_PHASE_VEH_CALL 	"1.3.6.1.4.1.1206.4.2.1.1.5.1.6.1"

// Controller configure information
#define MAX_PHASE_NO	 	"1.3.6.1.4.1.1206.4.2.1.1.1"
#define PHASE_NUMBER	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.1.1"   // phase number: last ".X" is phase number, the return value is also X

#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

#define PHASE_ENABLED       "1.3.6.1.4.1.1206.4.2.1.1.2.1.21."  // Phase options: last ".X" is phase, the last bit of return result is "0", the phase is not enabled.
//------------The following from standard: only read PLAN 1---------//
#define PHASE_MIN_GRN	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.4."   // need last "x" is the phase number: return the minimun green of phase x
#define PHASE_MAX_GRN	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.6."
#define PHASE_RED_CLR	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.9."
#define PHASE_YLW_XGE	 	"1.3.6.1.4.1.1206.4.2.1.1.2.1.8."
//------------The following from ASC3: WILL BE USED---------//
#define PHASE_MIN_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.9."   // need last "x.p" x is the timing plan number,p is the phase number: x get from CUR_TIMING_PLAN
#define PHASE_MAX_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.15."
#define PHASE_RED_CLR_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.19."
#define PHASE_YLW_XGE_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.18."

//**********asc3PhaseStatusTiming
//T (1):       Phase Timing
//N (2):       Phase Next
//- (3):       Phase Not Enabled
//(space) (4): Phase Not Timing or Next
#define PHASE_STA_TIME_ASC  "1.3.6.1.4.1.1206.3.5.2.1.18.1.1."  //NEED last "p"  for the phase
//**********asc3PhaseStatusTiming2
// (1) X: XPED timing
// (2) N: Phase Next
// (3) -: Phase Not enabled
// (4) .: Phase Not Timing
// (5) R: Phase Timing RED
// (6) Y: Phase Timing YEL
// (7) G: Phase Timing GREEN

// (8) D: Phase Timing DELAY GREEN
// (9) O: Phase Timing YEL & RED
//(10) g: Phase Timing FLASHING GREEN
//(11) y: Phase Timing FLASHING YELLOW "
#define PHASE_STA_TIME2_ASC  "1.3.6.1.4.1.1206.3.5.2.1.18.1.6."  //NEED last "p"  for the phase

// Added by Shayan 5.5.14
#define SYS_DET_VOL "1.3.6.1.4.1.1206.4.2.1.2.5.4.1.1." //NEED last "d" for the detector number to collect Volume
#define SYS_DET_OCC "1.3.6.1.4.1.1206.4.2.1.2.5.4.1.2." //NEED last "d" for the detector number to collect Occupancy

//define the different phase control types
#define PHASE_FORCEOFF 0
#define PHASE_OMIT 1
#define PHASE_VEH_CALL 2
#define	PHASE_HOLD 3

#define MAX_ITEMS 50
#define PORT 15020 //PREEMPTION port
#define BROADCAST_ADDR "192.168.255.255"

//----intersection ASC controller ip port
extern char INTip[64];// = "150.135.152.23";
extern char *INTport;
//extern char ConfigFile[256];
extern PhaseStatus phase_read;
extern int CurPhaseStatus[8];
extern int PhaseDisabled[8];

extern int out[20];
extern int tot_num;
extern int *Det_Number; //Indicate the detector numbers

//------------------END OF DEFINITION--------------------------------//

int GetSignalColor(int PhaseStatus);
void PhaseTimingStatusRead();
int  CurTimingPlanRead();  // For IntersectionConfigRead()
void IntersectionConfigRead(int CurTimingPlanNo,char *ConfigOutFile);
void IntersectionPhaseControl(int phase_control, int Total,char YES);
void DetRead();

int GetSignalColor(int PhaseStatusNo)
{
    int ColorValue=RED;

    switch (PhaseStatusNo)
    {
    case 2:
    case 3:
    case 4:
    case 5:
        ColorValue=RED;
        break;
    case 6:
    case 11:
        ColorValue=YELLOW;
        break;
    case 7:
    case 8:
        ColorValue=GREEN;
        break;
    default:
        ColorValue=0;
    }
    return ColorValue;
}

// WILL use the global parameter phase_read
void PhaseTimingStatusRead()
{
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    int count=1;

    init_snmp("ASC");   //Initialize the SNMP library

    snmp_sess_init( &session );  //Initialize a "session" that defines who we're going to talk to
    /* set up defaults */
    //char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
    //char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
    char ipwithport[64];
    strcpy(ipwithport,INTip);
    strcat(ipwithport,":");
    strcat(ipwithport,INTport); //for ASC get status, DO NOT USE port!!!
    session.peername = strdup(ipwithport);
    //session.version = SNMP_VERSION_2c; //for ASC intersection  /* set the SNMP version number */
    session.version = SNMP_VERSION_1;    //for ASC/3 software  /* set the SNMP version number */
    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
    {
        snmp_sess_perror("ASC", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to GET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;

    //---#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

    char ctemp[50];

    for(int i=1;i<=8;i++)
    {
        sprintf(ctemp,"%s%d",PHASE_STA_TIME2_ASC,i);

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }


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
        int *out = new int[MAX_ITEMS];
        int i =0;
        //for(vars = response->variables; vars; vars = vars->next_variable)
            //print_variable(vars->name, vars->name_length, vars);

        /* manipuate the information ourselves */
        for(vars = response->variables; vars; vars = vars->next_variable)
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
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                //printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
            }
        }
        //****** GET the results from controller *************//
        for(int i=0;i<8;i++)
        {
            phase_read.phaseColor[i]=GetSignalColor(out[i]);

            CurPhaseStatus[i]=out[i];   // NOT converted to GYR
            //cout<<"Timing Read: "<<CurPhaseStatus[i]<<endl;
            //if(out[i]==3)       PhaseDisabled[i]=1;  // Phase i is not enabled.
        }


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

    /*
    * Clean up:    *  1) free the response.   *  2) close the session.
    */
    if (response)        snmp_free_pdu(response);

    snmp_close(ss);

    SOCK_CLEANUP;

}

int  CurTimingPlanRead()
{
    // USING the standard oid : ONLY read timing plan 1.
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    int count=1;
    int currentTimePlan; // return value

    init_snmp("RSU");   //Initialize the SNMP library

    snmp_sess_init( &session );  //Initialize a "session" that defines who we're going to talk to
    /* set up defaults */
    //char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
    //char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
    char ipwithport[64];
    strcpy(ipwithport,INTip);
    strcat(ipwithport,":");
    strcat(ipwithport,INTport); //for ASC get status, DO NOT USE port!!!
    session.peername = strdup(ipwithport);
    //session.version = SNMP_VERSION_2c; //for ASC intersection  /* set the SNMP version number */
	session.version = SNMP_VERSION_1;    //for ASC/3 software  /* set the SNMP version number */
    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
    {
        snmp_sess_perror("RSU", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to GET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;

    //---#define CUR_TIMING_PLAN     "1.3.6.1.4.1.1206.3.5.2.1.22.0"      // return the current timing plan

    char ctemp[50];

    sprintf(ctemp,"%s",CUR_TIMING_PLAN);   // WORK
    // sprintf(ctemp,"%s",PHASE_MIN_GRN_ASC_TEST); // WORK
    // sprintf(ctemp,"%s%d.%d",PHASE_MIN_GRN_ASC,2,1);        //  WORK

    if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
    {
        snmp_perror(ctemp);
        SOCK_CLEANUP;
        exit(1);
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
        int *out = new int[MAX_ITEMS];
        int i =0;
      //  for(vars = response->variables; vars; vars = vars->next_variable)
       //     print_variable(vars->name, vars->name_length, vars);

        /* manipuate the information ourselves */
        for(vars = response->variables; vars; vars = vars->next_variable)
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
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                //printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
            }
        }

        //CUR_TIMING_PLAN_NO=out[0];
        currentTimePlan=out[0];

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

    /*
    * Clean up:    *  1) free the response.   *  2) close the session.
    */
    if (response)        snmp_free_pdu(response);

    snmp_close(ss);

    SOCK_CLEANUP;

    return currentTimePlan;

}


void IntersectionConfigRead(int CurTimingPlanNo,char *ConfigOutFile)
{

    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    int count=1;
    /*
    * Initialize the SNMP library
    */
    init_snmp("RSU");

    /*
    * Initialize a "session" that defines who we're going to talk to
    */
    snmp_sess_init( &session );                   /* set up defaults */
    //char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
    //char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
    char ipwithport[64];
    strcpy(ipwithport,INTip);
    strcat(ipwithport,":");
    strcat(ipwithport,INTport); //for ASC get status, DO NOT USE port!!!
    session.peername = strdup(ipwithport);
    /* set the SNMP version number */
    //session.version = SNMP_VERSION_2c; //for ASC intersection
	session.version = SNMP_VERSION_1;    //for ASC/3 software  /* set the SNMP version number */
    //session.version = SNMP_VERSION_1; //for Rampmeter

    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
    {
        snmp_sess_perror("RSU", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to GET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    anOID_len = MAX_OID_LEN;

    //// Phase options: last ".X" is phase, the last bit of return result is "0", the phase is not enabled.
    //#define PHASE_ENABLED       "1.3.6.1.4.1.1206.4.2.1.1.2.1.21."
    ////------The following from ASC3------------//
    //#define PHASE_MIN_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.9."   // need last "x.p" x is the timing plan number,p is the phase number: x get from CUR_TIMING_PLAN
    //#define PHASE_MAX_GRN_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.15."
    //#define PHASE_RED_CLR_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.19."
    //#define PHASE_YLW_XGE_ASC 	"1.3.6.1.4.1.1206.3.5.2.1.2.1.18."
    char ctemp[50];

    for(int i=1;i<=8;i++) //PHASE_MIN_GRN_ASC
    {
        sprintf(ctemp,"%s%d.%d",PHASE_MIN_GRN_ASC,CurTimingPlanNo,i);        //  in seconds

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }

    for(int i=1;i<=8;i++)  //PHASE_YLW_XGE_ASC
    {
        sprintf(ctemp,"%s%d.%d",PHASE_YLW_XGE_ASC,CurTimingPlanNo,i);    // in tenth of seconds, for example if 35 should be 3.5 seconds

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }

    for(int i=1;i<=8;i++) //PHASE_RED_CLR_ASC
    {
        sprintf(ctemp,"%s%d.%d",PHASE_RED_CLR_ASC,CurTimingPlanNo,i);   // in tenth of seconds, for example if 35 should be 3.5 seconds

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }

    for(int i=1;i<=8;i++) // PHASE_MAX_GRN_ASC
    {
        sprintf(ctemp,"%s%d.%d",PHASE_MAX_GRN_ASC,CurTimingPlanNo,i);  // in seconds

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len))
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }
    for(int i=1;i<=8;i++)
    {
        sprintf(ctemp,"%s%d",PHASE_ENABLED,i);

        if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
        {
            snmp_perror(ctemp);
            SOCK_CLEANUP;
            exit(1);
        }

        snmp_add_null_var(pdu, anOID, anOID_len);

    }

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
        int *out = new int[MAX_ITEMS];
        int i =0;
      //  for(vars = response->variables; vars; vars = vars->next_variable)
       //     print_variable(vars->name, vars->name_length, vars);

        /* manipuate the information ourselves */
        for(vars = response->variables; vars; vars = vars->next_variable)
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
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                //printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);
            }
        }
        //FOR ASC INTERSECTIONS_ Draw the lights
        //UpdateIntersectionStatus(out[0],out[1]);
        //cout<<"Minimum Green Time for Phase 1 is:\t"<<out[0]<<" ****** "<<out[1]<<endl;
        // *** int PhaseSeq[8],MinGrn[8],Yellow[8],RedClr[8],GrnMax[8];***//

        int Result[5][8];  //*** Sequence: MinGrn[8],Yellow[8],RedClr[8],GrnMax[8],PhaseSeq[8],;***//
        /*
        Result[0][8]  ----> MinGrn[8]
        Result[1][8]  ----> Yellow[8]
        Result[2][8]  ----> RedClr[8]
        Result[3][8]  ----> GrnMax[8]
        Result[4][8]  ----> PhaseSeq[8]
        */
        for(int i=0;i<5;i++)
        {
            for(int j=0;j<8;j++)
            {
                Result[i][j]=out[j+i*8];
            }
        }
        int phaseSeq[8];
		int MinGrn[8];
		int GrnMax[8];
        float   Yellow[8],RedClr[8];

        int TotalNo=0;

        for(int i=0;i<8;i++)
        {
            //*** Last bit is 1, odd number, the '&' is 0:
            if(Result[4][i] & 1)  //*** Be careful here: it is bit "&" not "&&". ***//
            {
                phaseSeq[i]=i+1;

                MinGrn[i]=Result[0][i];
                Yellow[i]=float(Result[1][i]/10.0);
                RedClr[i]=float(Result[2][i]/10.0);
                GrnMax[i]=Result[3][i];

                TotalNo++;
            }

            else
            {
                phaseSeq[i]=0;
                MinGrn[i]=0;
                Yellow[i]=0;
                RedClr[i]=0;
                GrnMax[i]=0;
            }
        }

        fstream fs_config;
        //TODO:,char *ConfigOutFile
        //fs_config.open(ConfigFile,ios::out);
        fs_config.open(ConfigOutFile,ios::out);


        fs_config<<"Phase_Num "<<TotalNo<<endl;

        fs_config<<"Phase_Seq";
        for(int i=0;i<8;i++) fs_config<<" "<<phaseSeq[i];
        fs_config<<endl;

        fs_config<<"Gmin";
        for(int i=0;i<8;i++) fs_config<<"\t"<<MinGrn[i];
        fs_config<<endl;

        fs_config<<"Yellow";
        for(int i=0;i<8;i++) fs_config<<"\t"<<Yellow[i];
        fs_config<<endl;

        fs_config<<"Red";
        for(int i=0;i<8;i++) fs_config<<"\t"<<RedClr[i];
        fs_config<<endl;

        fs_config<<"Gmax";
        for(int i=0;i<8;i++) fs_config<<"\t"<<GrnMax[i];
        fs_config<<endl;


        fs_config.close();


    }
    else
    {
        /*
        * FAILURE: print what went wrong!
        */

        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet\nReason: %s\n",
            snmp_errstring(response->errstat));
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
            session.peername);
        else
            snmp_sess_perror("snmpdemoapp", ss);

    }

    /*
    * Clean up:
    *  1) free the response.
    *  2) close the session.
    */
    if (response)        snmp_free_pdu(response);

    snmp_close(ss);

    SOCK_CLEANUP;

}

void IntersectionPhaseControl(int phase_control, int Total,char YES)
{
    char tmp_log[64];
    char tmp_int[16];
    char buffer[16];
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    int count=1;
    int  failures = 0;

    //int number=pow(2.0,phaseNO-1);
    int number=Total;

    //itoa(number,buffer,2); // NOT WORKING

    sprintf(tmp_int,"%d",number);
    cout<<"CMD Applied to Phase: ";//<<buffer;
    binary(Total);
    cout<<"  "<<tmp_int<<"  "<<YES<<endl;

    /*
    * Initialize the SNMP library
    */
    init_snmp("RSU");

    /*
    * Initialize a "session" that defines who we're going to talk to
    */
    snmp_sess_init( &session );                   /* set up defaults */

    char ipwithport[64];
    strcpy(ipwithport,INTip);
    strcat(ipwithport,":");
    strcat(ipwithport,INTport);
    session.peername = strdup(ipwithport);

    /* set the SNMP version number */
    //session.version = SNMP_VERSION_2c;  //For Intersection Control
	session.version = SNMP_VERSION_1;    //for ASC/3 software  /* set the SNMP version number */

    /* set the SNMPv1 community name used for authentication */
    session.community = (u_char *)"public";
    session.community_len = strlen((const char *)session.community);

    SOCK_STARTUP;
    ss = snmp_open(&session);                     /* establish the session */

    if (!ss)
    {
        snmp_sess_perror("RSU", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Create the PDU for the data for our request.
    *   1) We're going to SET the system.sysDescr.0 node.
    */
    pdu = snmp_pdu_create(SNMP_MSG_SET);
    anOID_len = MAX_OID_LEN;
    if (PHASE_HOLD==phase_control)
    {
        if (!snmp_parse_oid(MIB_PHASE_HOLD, anOID, &anOID_len))
        {
            snmp_perror(MIB_PHASE_HOLD);
            failures++;
        }
        sprintf(tmp_log,"HOLD control! Number (%d), AT time:[%.2f] \n ",Total,GetSeconds());
        std::cout <<tmp_log;       //outputlog(tmp_log);

    }
    else if (PHASE_FORCEOFF==phase_control)
    {
        if (!snmp_parse_oid(MIB_PHASE_FORCEOFF, anOID, &anOID_len))
        {
            snmp_perror(MIB_PHASE_FORCEOFF);
            failures++;
        }
        sprintf(tmp_log,"FORCEOFF control! Number (%d), AT time:[%.2f] \n ",Total,GetSeconds());
        std::cout <<tmp_log;        //outputlog(tmp_log);

    }
    else if (PHASE_OMIT==phase_control)
    {
        if (!snmp_parse_oid(MIB_PHASE_OMIT, anOID, &anOID_len))
        {
            snmp_perror(MIB_PHASE_OMIT);
            failures++;
        }
        sprintf(tmp_log,"OMIT control! Number (%d), AT time:[%.2f] \n ",Total,GetSeconds());
        std::cout <<tmp_log;        //outputlog(tmp_log);
    }
    else if (PHASE_VEH_CALL==phase_control)
    {
        if (!snmp_parse_oid(MIB_PHASE_VEH_CALL, anOID, &anOID_len))
        {
            snmp_perror(MIB_PHASE_VEH_CALL);
            failures++;
        }
        sprintf(tmp_log,"VEH CALL to ASC controller! Number (%d), AT time:%ld \n ",Total,time(NULL));
        std::cout <<tmp_log;        //outputlog(tmp_log);
    }


    //snmp_add_var() return 0 if success
    if (snmp_add_var(pdu, anOID, anOID_len,'i', tmp_int))
    {
        switch(phase_control)
        {
        case PHASE_FORCEOFF:
            snmp_perror(MIB_PHASE_FORCEOFF); break;
        case PHASE_OMIT:
            snmp_perror(MIB_PHASE_OMIT); break;
        case PHASE_VEH_CALL:
            snmp_perror(MIB_PHASE_VEH_CALL); break;
        case PHASE_HOLD:
            snmp_perror(MIB_PHASE_HOLD); break;
        }

        failures++;
    }

    if (failures)
    {
        snmp_close(ss);
        SOCK_CLEANUP;
        exit(1);
    }

    /*
    * Send the Request out.
    */
    status = snmp_synch_response(ss, pdu, &response);

    /*
    * Process the response.
    */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
        //------SUCCESS: Print the result variables
        int *out = new int[MAX_ITEMS];
        int i =0;
        for(vars = response->variables; vars; vars = vars->next_variable)
            print_variable(vars->name, vars->name_length, vars);

        /* manipuate the information ourselves */
        for(vars = response->variables; vars; vars = vars->next_variable)
        {
            if (vars->type == ASN_OCTET_STR)
            {
                char *sp = (char *)malloc(1 + vars->val_len);
                memcpy(sp, vars->val.string, vars->val_len);
                sp[vars->val_len] = '\0';
                printf("value #%d is a string: %s\n", count++, sp);
                free(sp);
            }
            else
            {
                int *aa;
                aa =(int *)vars->val.integer;
                out[i++] = * aa;
                printf("value #%d is NOT a string! Ack!\n", count++);
            }
        }
    }
    else
    {
        // FAILURE: print what went wrong!
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error in packet\nReason: %s\n",
            snmp_errstring(response->errstat));
        else if (status == STAT_TIMEOUT)
            fprintf(stderr, "Timeout: No response from %s.\n",
            session.peername);
        else
            snmp_sess_perror("snmpdemoapp", ss);
    }
    //------Clean up:1) free the response. 2) close the session.
    if (response)
        snmp_free_pdu(response);
    snmp_close(ss);

    SOCK_CLEANUP;

}

void DetRead()
{
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *response;

	oid anOID[MAX_OID_LEN];
	size_t anOID_len;

	netsnmp_variable_list *vars;
	int status;
	int count=1;

	init_snmp("ASC");   //Initialize the SNMP library

	snmp_sess_init( &session );  //Initialize a "session" that defines who we're going to talk to
	/* set up defaults */
	//char *ip = m_rampmeterip.GetBuffer(m_rampmeterip.GetLength());
	//char *port = m_rampmeterport.GetBuffer(m_rampmeterport.GetLength());
	char ipwithport[64];
	strcpy(ipwithport,INTip);
	strcat(ipwithport,":");
	strcat(ipwithport,INTport);
	session.peername = strdup(ipwithport);
	//session.version = SNMP_VERSION_2c; //for ASC intersection  /* set the SNMP version number */
	session.version = SNMP_VERSION_1; //for ASC/3 Software in VISSIM  /* set the SNMP version number */
	/* set the SNMPv1 community name used for authentication */
	session.community = (u_char *)"public";
	session.community_len = strlen((const char *)session.community);

	SOCK_STARTUP;
	ss = snmp_open(&session);                     /* establish the session */

	if (!ss)
	{
		snmp_sess_perror("ASC", &session);
		SOCK_CLEANUP;
		exit(1);
	}

	/*
	 * Create the PDU for the data for our request.
	 *   1) We're going to GET the system.sysDescr.0 node.
	 */
	pdu = snmp_pdu_create(SNMP_MSG_GET);
	anOID_len = MAX_OID_LEN;

	char ctemp[50];

	
		// Reading all the available Detectors
		for(int i=0;i < tot_num; i++)
		{
		if(Det_Number[i] > 0) //It is added to get only real detectors' values
			{
				sprintf(ctemp,"%s%d",SYS_DET_VOL,Det_Number[i]);
				//cout<<"Detector number is: "<<Det_Number[i]<<endl;
				if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
				{
					snmp_perror(ctemp);
					SOCK_CLEANUP;
					exit(1);
				}

				snmp_add_null_var(pdu, anOID, anOID_len);
			}

		}
		/*
		 * Send the Request out.
		 */
		status = snmp_synch_response(ss, pdu, &response);

		/*
		 * Process the response.
		 */
		//cout<<"I am here "<<response->errstat<<" "<<SNMP_ERR_NOERROR<<endl; 
		if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
		{
			/*
			 * SUCCESS: Print the result variables
			 */
			//int *out = new int[MAX_ITEMS];
			int i =0;
			for(vars = response->variables; vars; vars = vars->next_variable)
				print_variable(vars->name, vars->name_length, vars);

			/* manipulate the information ourselves */
			for(vars = response->variables; vars; vars = vars->next_variable)
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
					aa =(int *)vars->val.integer;
					out[i++] = * aa;
					//printf("value #%d is NOT a string! Ack!. Value = %d \n", count++,*aa);	
				}
			}
			

			//cout<<"The VOLUME output of the detector information is: "<<out[0]<<" "<<out[1]<<" "<<out[2]<<" "<<out[3]<<" "<<out[4]<<" "<<out[5]<<" "<<out[6]<<" "<<out[7]<<" "<<out[8]<<" "<<out[9]<<" "<<out[10]<<" "<<out[11]<<endl;
			
			//logfile<<out[0]<<" "<<out[1]<<" "<<out[2]<<" "<<out[3]<<" "<<out[4]<<" "<<out[5]<<" "<<out[6]<<" "<<out[7]<<" "<<out[8]<<" "<<out[9]<<endl;
			

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
	
	/*
	 * Clean up:    *  1) free the response.   *  2) close the session.
	 */
	if (response)        snmp_free_pdu(response);

	snmp_close(ss);

	SOCK_CLEANUP;
}
