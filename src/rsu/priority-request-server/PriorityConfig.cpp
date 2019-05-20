
#include "PriorityConfig.h"

PriorityConfig::PriorityConfig(void) {
    dCoordinationWeight = 0.0;
    for (int i = 0; i < 2; i++)
        iCoordinatedPhase[i] = 0;
    dCoordCycle = 0.0;
    for (int i = 0; i < 8; i++)
        dCoordinationPhaseSplit[i] = 0.0;
    dCoordOffset = 0.0;
    dTransitWeight = 0.0;
    dTruckWeight = 0.0;
    bSimilarCoordinationSplit = 1;
    dSmallerCoordinationSplit = 0.0;
    dLargerCoordinationSplit = 0.0;
    iNumberOfCoordinatedPhase = 0;
    dSplitDifference = 0.0;
}

/*
void PriorityConfig::setPriorityConfig(double dCoordinationWeightAr, int iCoordinatedPhaseAr[2], double dCoordCycleAr, double dCoordinationPhaseSplitAr[8], double dCoordOffsetAr, double dTransitWeightAr, double dTruckWeightAr)
{
	dCoordinationWeight=dCoordinationWeightAr;
	for (int i=0;i<2;i++)
	{
		if (iCoordinatedPhaseAr[i]>0)
			iCoordinatedPhase[i]=iCoordinatedPhaseAr[i];
	}
    dCoordCycle=dCoordCycleAr;
    for (int i=0;i<8;i++)
		dCoordinationPhaseSplit[i]=dCoordinationPhaseSplitAr[i];
	dCoordOffset=dCoordOffsetAr;
	dTransitWeight=dTransitWeightAr;
	dTruckWeight=dTruckWeightAr;
}
*/
void PriorityConfig::readPriorityConfig(const char *filename) {
    double dCoordinationWeightTemp = 0.0;
    int iCoordinatedPhaseTemp[2];
    int iPhases[8];
    double dTransitWeightTemp = 0.0;
    double dTruckWeightTemp = 0.0;
    double dCoordOffsetTemp = 0.0;
    double dCoordCycleTemp = 0.0;
    double dCoordPhaseSplitTemp[8];
    double dPhaseClearance[8];
    string lineread;
    fstream FileRead2;
    FileRead2.open(filename, ios::in);
    if (!FileRead2) {
        cerr << "Unable to open priorityConfiguration.txt file!" << endl;
    }
    getline(FileRead2, lineread);

    // priorityConfiguration.txt 
    // # this file is to set the priority configuration and coorination setup, an example will be :
    // coordination_weight 1
    // cycle 90 
    // offset 0
    // coordinated_phase1 2
    // coordinated_phase2 6
    // phase_split 
    // 1 20
    // 2 30
    // 3 10
    // 4 30
    // 5 20
    // 6 30
    // 7 10
    // 8 30
    // transit_weight 3
    // truck_weight 1

    getline(FileRead2, lineread);
    if (lineread.size() != 0) {
        sscanf(lineread.c_str(), "%*s %lf ", &dCoordinationWeightTemp);
        getline(FileRead2, lineread);
        sscanf(lineread.c_str(), "%*s %lf ", &dCoordCycleTemp);
        getline(FileRead2, lineread);
        sscanf(lineread.c_str(), "%*s %lf ", &dCoordOffsetTemp);
        getline(FileRead2, lineread);
        sscanf(lineread.c_str(), "%*s %d ", &iCoordinatedPhaseTemp[0]);
        getline(FileRead2, lineread);
        sscanf(lineread.c_str(), "%*s %d ", &iCoordinatedPhaseTemp[1]);
        getline(FileRead2, lineread);
        for (int cnt = 0; cnt < 8; cnt++) {
            getline(FileRead2, lineread);
            sscanf(lineread.c_str(), "%d %lf", &iPhases[cnt], &dCoordPhaseSplitTemp[cnt]);
        }
        getline(FileRead2, lineread);
        sscanf(lineread.c_str(), "%*s %lf ", &dTransitWeightTemp);
        getline(FileRead2, lineread);
        sscanf(lineread.c_str(), "%*s %lf ", &dTruckWeightTemp);
        getline(FileRead2, lineread);
        for (int cnt = 0; cnt < 8; cnt++) {
            getline(FileRead2, lineread);
            sscanf(lineread.c_str(), "%*d %lf", &dPhaseClearance[cnt]);
        }
    }
    FileRead2.close();
    for (int cnt = 0; cnt < 8; cnt++) {
        dCoordPhaseSplitTemp[cnt] = dCoordPhaseSplitTemp[cnt] - dPhaseClearance[cnt];
    }


    // reset the class variables
    dCoordinationWeight = 0.0;
    for (int i = 0; i < 2; i++)
        iCoordinatedPhase[i] = 0;
    dCoordCycle = 0.0;
    for (int i = 0; i < 8; i++)
        dCoordinationPhaseSplit[i] = 0.0;
    dCoordOffset = 0.0;
    dTransitWeight = 0.0;
    dTruckWeight = 0.0;
    bSimilarCoordinationSplit = 1;
    dSmallerCoordinationSplit = 0.0;
    dLargerCoordinationSplit = 0.0;
    iNumberOfCoordinatedPhase = 0;
    dSplitDifference = 0.0;

    // fill up the variables using the Temp variable
    dCoordinationWeight = dCoordinationWeightTemp;
    for (int i = 0; i < 2; i++) {
        if (iCoordinatedPhaseTemp[i] > 0)
            iCoordinatedPhase[i] = iCoordinatedPhaseTemp[i];
    }
    dCoordCycle = dCoordCycleTemp;
    for (int i = 0; i < 8; i++)
        dCoordinationPhaseSplit[i] = dCoordPhaseSplitTemp[i];
    dCoordOffset = dCoordOffsetTemp;
    dTransitWeight = dTransitWeightTemp;
    dTruckWeight = dTruckWeightTemp;
    cout << " Coordination Weight : " << dCoordinationWeight << endl;
    if (dCoordinationWeight > 0) {
        cout << " Coordination Cycle  : " << dCoordCycle << endl;
        cout << " Coordination Offset : " << dCoordOffset << endl;
        // 		setting bSimilarCoordinationSplit , dSmallCoordinationSplit and  dBigCoordinationSplit and dCoordinationPhaseSplit[FOR COORDINATED PHASE]
        if (iCoordinatedPhase[0] > 0 && iCoordinatedPhase[1] > 0)
            iNumberOfCoordinatedPhase = 2;
        else if ((iCoordinatedPhase[0] == 0 && iCoordinatedPhase[1] > 0) ||
                 (iCoordinatedPhase[0] > 0 && iCoordinatedPhase[1] == 0))
            iNumberOfCoordinatedPhase = 1;
        else
            iNumberOfCoordinatedPhase = 0;

        for (int it = 0; it < iNumberOfCoordinatedPhase; it++) {
            if (iCoordinatedPhase[it] > 0) {
                cout << " Coordinated Phase " << it + 1 << "       :" << iCoordinatedPhase[it] << endl;
                cout << " Coordinated Phase " << it + 1 << " Split :"
                     << dCoordinationPhaseSplit[iCoordinatedPhase[it] - 1] << endl;
            }
        }

        if ((dCoordinationPhaseSplit[iCoordinatedPhase[0] - 1] > 0) &&
            (dCoordinationPhaseSplit[iCoordinatedPhase[0] - 1] == dCoordinationPhaseSplit[iCoordinatedPhase[1] - 1])) {
            bSimilarCoordinationSplit = 1;
            dSmallerCoordinationSplit = dCoordinationPhaseSplit[iCoordinatedPhase[0] -
                                                                1]; // in this case , the small and big coordinated phase split time are the same
            dLargerCoordinationSplit = dSmallerCoordinationSplit;
        } else if (dCoordinationPhaseSplit[iCoordinatedPhase[0] - 1] !=
                   dCoordinationPhaseSplit[iCoordinatedPhase[1] - 1]) {
            bSimilarCoordinationSplit = 0;
            dSmallerCoordinationSplit = min(dCoordinationPhaseSplit[iCoordinatedPhase[0] - 1],
                                            dCoordinationPhaseSplit[iCoordinatedPhase[1] - 1]);
            dLargerCoordinationSplit = max(dCoordinationPhaseSplit[iCoordinatedPhase[0] - 1],
                                           dCoordinationPhaseSplit[iCoordinatedPhase[1] - 1]);
        }
        dSplitDifference = dLargerCoordinationSplit - dSmallerCoordinationSplit;
    }

}


PriorityConfig::~PriorityConfig(void) {

}
