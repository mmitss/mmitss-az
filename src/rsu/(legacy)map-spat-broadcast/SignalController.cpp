#include "SignalController.h"


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

int outputlog(char *output)
{
	FILE * stream = fopen( logfilename, "r" );
	fseek( stream, 0L, SEEK_END );
	long endPos = ftell( stream );
	fclose( stream );

	std::fstream fs;
	if (endPos <10000000)
		fs.open(logfilename, std::ios::out | std::ios::app);
	else
		fs.open(logfilename, std::ios::out | std::ios::trunc);
	if (!fs || !fs.good())
	{
		std::cout << "could not open file!\n";
		return -1;
	}
	fs << output;// << std::endl;

	if (fs.fail())
	{
		std::cout << "failed to append to file!\n";
		return -1;
	}
	return 1;
}



int  SignalController::CurTimingPlanRead()
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
       strcat(ipwithport,INTport);
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

		// ----------get the current timing plan----------------//
        CurTimingPlan=out[0];

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

    return CurTimingPlan;

}





SignalController::SignalController(void)
{
	for( int i=0;i<numPhases;i++)
	{
		phase_read.phaseColor[i]=0;
	}
}

SignalController::~SignalController(void)
{
}


void SignalController::PhaseControl(int phase_control, int Total,char YES)
    {
    char tmp_log[64];
    char tmp_int[16];
    char buffer[16];
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    netsnmp_pdu *response;
	int verbose=0;
	if(YES=='y' || YES=='Y') verbose=1;

    oid anOID[MAX_OID_LEN];
    size_t anOID_len;

    netsnmp_variable_list *vars;
    int status;
    int count=1;
    int  failures = 0;

    //int number=pow(2.0,phaseNO-1);
    int number=Total;

    //itoa(number,buffer,2);

    sprintf(tmp_int,"%d",number);

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
    //session.version = SNMP_VERSION_2c;
    session.version = SNMP_VERSION_1;

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


		if (verbose)
		{
			sprintf(tmp_log,"HOLD control! Number (%d)\n",Total);
			std::cout <<tmp_log; outputlog(tmp_log);
		}

        }
    else if (PHASE_FORCEOFF==phase_control)
        {
        if (!snmp_parse_oid(MIB_PHASE_FORCEOFF, anOID, &anOID_len))
            {
            snmp_perror(MIB_PHASE_FORCEOFF);
            failures++;
            }
		if (verbose)
		{
			sprintf(tmp_log,"FORCEOFF control! Number (%d)\n",Total);
			std::cout <<tmp_log;
			outputlog(tmp_log);
		}


        }
    else if (PHASE_OMIT==phase_control)
        {
        if (!snmp_parse_oid(MIB_PHASE_OMIT, anOID, &anOID_len))
            {
            snmp_perror(MIB_PHASE_OMIT);
            failures++;
            }
		if (verbose)
		{
        sprintf(tmp_log,"OMIT control! Number (%d)\n",Total);
        std::cout <<tmp_log;
        outputlog(tmp_log);
		}
        }
    else if (PHASE_VEH_CALL==phase_control)
        {
        if (!snmp_parse_oid(MIB_PHASE_VEH_CALL, anOID, &anOID_len))
            {
            snmp_perror(MIB_PHASE_VEH_CALL);
            failures++;
            }
		if (verbose)
		{
        sprintf(tmp_log,"VEH CALL to ASC controller! Number (%d)\n",Total);
        std::cout <<tmp_log;
		outputlog(tmp_log);
		}
        }


    //snmp_add_var() return 0 if success
    if (snmp_add_var(pdu, anOID, anOID_len,'i', tmp_int))
        {
        snmp_perror(MIB_PHASE_HOLD);
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


void SignalController::PhaseRead()
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
	    session.version = SNMP_VERSION_1; //for ASC intersection  /* set the SNMP version number */
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

void SignalController::UpdatePhase()
{
	Phases.UpdatePhase(phase_read);
}

void SignalController::SPATSet(int value)
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


    //itoa(number,buffer,2);

    


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
    //session.version = SNMP_VERSION_2c;
    session.version = SNMP_VERSION_1;

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


	char ctemp[50];


	        sprintf(ctemp,"%s",asc3ViiMessageEnable);

	        if (!snmp_parse_oid(ctemp, anOID, &anOID_len)) // Phase sequence in the controller: last bit as enabled or not: "1"  enable; "0" not used
	        {
	            snmp_perror(ctemp);
	            SOCK_CLEANUP;
	            exit(1);
	        }
	
			sprintf(tmp_int,"%d",value);

    //snmp_add_var() return 0 if success
    if (snmp_add_var(pdu, anOID, anOID_len,'i', tmp_int))
        {
        snmp_perror(MIB_PHASE_HOLD);
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
		cout<<"Set SPAT broadcast successfully!"<<endl;
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
