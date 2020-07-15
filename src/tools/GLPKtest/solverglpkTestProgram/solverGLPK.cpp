#include "glpk.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "time.h"
#include <sys/time.h>

using std::cout;
using std::endl;

double GetSeconds()
{
    struct timeval tv_tt;
    gettimeofday(&tv_tt, NULL);
    return (tv_tt.tv_sec + tv_tt.tv_usec / 1.0e6);
}

double startOfSolve{};
double endOfSolve{};

int main()
{
    char modFile[128] = "../creatingPriorityRequestList/NewModel_EV.mod";
    glp_prob *mip;
    glp_tran *tran;
    int ret{};
    int success = 1;
    mip = glp_create_prob();
    tran = glp_mpl_alloc_wksp();

    ret = glp_mpl_read_model(tran, modFile, 1);

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating model\n");
        goto skip;
    }

    ret = glp_mpl_read_data(tran, "../creatingPriorityRequestList/NewModelData.dat");

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating data\n");
        goto skip;
    }

    ret = glp_mpl_generate(tran, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Error on generating model\n");
        goto skip;
    }

    glp_mpl_build_prob(tran, mip);
    glp_simplex(mip, NULL);
    success = glp_intopt(mip, NULL);
    endOfSolve = GetSeconds();
    cout << "Success=" << success << endl;
    cout << "Time of Solve" << endOfSolve - startOfSolve << endl;
    ret = glp_mpl_postsolve(tran, mip, GLP_MIP);
    if (ret != 0)
        fprintf(stderr, "Error on postsolving model\n");
skip:
    glp_mpl_free_wksp(tran);
    glp_delete_prob(mip);
}