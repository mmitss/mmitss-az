
#include "ReqEntry.h"

ReqEntry::ReqEntry(void) {
    VehID = 0;
    VehClass = 10;  // very low priority
    ETA = 0.0;
    Phase = 0;
    dSetRequestTime = 0.0;
    MinGreen = 0.0;
    Split_Phase = -10; // means not a split phase
    iInLane = 0;
    iOutLane = 0;
    iStrHour = 0;
    iStrMinute = 0;
    iStrSecond = 0;
    iEndHour = 0;
    iEndMinute = 0;
    iEndSecond = 0;
    iVehState = 0;
    iMsgCnt = 0;
    dUpdateTimeOfETA = 0.0;
    iLeavingCounter = 0;
    dTimeInCycle = 0.0;
    lIntersectionId = 0;
    iRequestType = 0;
    ibasicVehicleRole = 0;
    ipriorityRequestStatus = 0;
}


ReqEntry::ReqEntry(long vehID, int vehClass, float eta, int phase, float mgreen, double dsetRequestTime,
                   int split_phase, int iinLane, int ioutLane, int istrHour, int istrMinute, int istrSecond,
                   int iendHour, int iendMinute, int iendSecond, int ivehState, int imsgcnt, double dupdateTimeOfETA,
                   int ileavingCounter, double dtimeInCycle, long lintersectionId, int requestType, int basicVehicleRole,
                   int priorityRequestStatus) {
    VehID = vehID;
    VehClass = vehClass;
    ETA = eta;
    Phase = phase;
    MinGreen = mgreen;
    dSetRequestTime = dsetRequestTime;
    Split_Phase = split_phase;

    iInLane = iinLane;
    iOutLane = ioutLane;
    iStrHour = istrHour;
    iStrMinute = istrMinute;
    iStrSecond = istrSecond;
    iEndHour = iendHour;
    iEndMinute = iendMinute;
    iEndSecond = iendSecond;
    iVehState = ivehState;
    iMsgCnt = imsgcnt;
    dUpdateTimeOfETA = dupdateTimeOfETA;
    iLeavingCounter = ileavingCounter;
    dTimeInCycle = dtimeInCycle;
    lIntersectionId = lintersectionId;
    iRequestType = requestType;
    ibasicVehicleRole = basicVehicleRole;
    ipriorityRequestStatus = priorityRequestStatus;
}


ReqEntry::ReqEntry(ReqEntry &Req) {
    VehID = Req.VehID;
    VehClass = Req.VehClass;
    ETA = Req.ETA;
    Phase = Req.Phase;
    MinGreen = Req.MinGreen;
    dSetRequestTime = Req.dSetRequestTime;
    Split_Phase = Req.Split_Phase;

    iInLane = Req.iInLane;
    iOutLane = Req.iOutLane;
    iStrHour = Req.iStrHour;
    iStrMinute = Req.iStrMinute;
    iStrSecond = Req.iStrSecond;
    iEndHour = Req.iEndHour;
    iEndMinute = Req.iEndMinute;
    iEndSecond = Req.iEndSecond;
    iVehState = Req.iVehState;
    iMsgCnt = Req.iMsgCnt;
    dUpdateTimeOfETA = Req.dUpdateTimeOfETA;
    iLeavingCounter = Req.iLeavingCounter;
    dTimeInCycle = Req.dTimeInCycle;
    lIntersectionId = Req.lIntersectionId;
    iRequestType = Req.iRequestType;
    ibasicVehicleRole = Req.ibasicVehicleRole;
    ipriorityRequestStatus = Req.ibasicVehicleRole;
}

ReqEntry &ReqEntry::operator=(ReqEntry &Req) {
    VehID = Req.VehID;
    VehClass = Req.VehClass;
    ETA = Req.ETA;
    Phase = Req.Phase;
    MinGreen = Req.MinGreen;
    dSetRequestTime = Req.dSetRequestTime;
    Split_Phase = Req.Split_Phase;
    iInLane = Req.iInLane;
    iOutLane = Req.iOutLane;
    iStrHour = Req.iStrHour;
    iStrMinute = Req.iStrMinute;
    iStrSecond = Req.iStrSecond;
    iEndHour = Req.iEndHour;
    iEndMinute = Req.iEndMinute;
    iEndSecond = Req.iEndSecond;
    iVehState = Req.iVehState;
    iMsgCnt = Req.iMsgCnt;
    dUpdateTimeOfETA = Req.dUpdateTimeOfETA;
    iLeavingCounter = Req.iLeavingCounter;
    dTimeInCycle = Req.dTimeInCycle;
    lIntersectionId = Req.lIntersectionId;
    iRequestType = Req.iRequestType;
    ibasicVehicleRole = Req.ibasicVehicleRole;
    ipriorityRequestStatus = Req.ibasicVehicleRole;
    return *this;
}


ReqEntry::~ReqEntry(void) {}
