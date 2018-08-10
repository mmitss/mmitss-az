#pragma once


#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <assert.h>
#include "LinkedList.h"
#include "IDMSGcnt.h"

using namespace std;

/*==========================================================================*/
//------------Function Added by DJ

bool FindInReqList(LinkedList<IDMSGcnt> &ReqList, IDMSGcnt TestEntry);
void SortReqList(LinkedList<IDMSGcnt> &ReqList);
void ReqListOutFile(LinkedList<IDMSGcnt> ReqList, ofstream &file);
int FindInReqListID(LinkedList<IDMSGcnt> &ReqList, int TempID);

IDMSGcnt RetrieveEntry(LinkedList<IDMSGcnt>& ReqList, int TempID);
int UpdateReqList(LinkedList<IDMSGcnt> &ReqList, int TempID);


int UpdateReqList(LinkedList<IDMSGcnt> &ReqList, int TempID)
{
	if (FindInReqListID(ReqList,TempID)<0)
	{
		IDMSGcnt temp(TempID,0);
		ReqList.InsertRear(temp);
		return 0;
	} 
	else
	{
		ReqList.Reset();
		while(!ReqList.EndOfList())
		{
			if(TempID==ReqList.Data().TempID)
			{
				ReqList.Data().MSGcnt++;

				return(ReqList.Data().MSGcnt);
				//break;
			}

			ReqList.Next();
		}
	}

}
void SortReqList(LinkedList<IDMSGcnt> &ReqList)
    {
          cout<<"Sorting...\n";
    if(ReqList.ListEmpty())
        {
        return;
        }
    else
        {
        LinkedList<IDMSGcnt> Templist;//temp ordered request list

        ReqList.Reset();

        Templist.InsertAfter(ReqList.Data()); // Insert the first Data to Temp list

        ReqList.Next();

        while(!ReqList.EndOfList())
            {

            Templist.Reset();

            while(!Templist.EndOfList())
                {
                if(ReqList.Data().TempID<=Templist.Data().TempID)
                    {
                    Templist.InsertAt(ReqList.Data());
                    break;
                    }
                else
                    {
                    if(Templist.CurrentPosition()==Templist.ListSize()-1)
                        {
                        Templist.InsertRear(ReqList.Data());
                        }
                    Templist.Next();
                    }
                }

            ReqList.Next();
            }

        ReqList = Templist;

        }
    }

bool FindInReqList(LinkedList<IDMSGcnt> &ReqList, IDMSGcnt TestEntry)
    {

    SortReqList(ReqList);

    ReqList.Reset();

    bool temp=false;
	if(ReqList.ListEmpty()) 
	{
		return temp;
	}
	else
	{

    while(!ReqList.EndOfList())
        {
        if(ReqList.Data().TempID > TestEntry.TempID)
            {
            return false;
            }
        else if(ReqList.Data().TempID == TestEntry.TempID)
            {
             return true;
            }
        else
            {
            ReqList.Next();
            }        
        }
	}

    return temp;
    }


int FindInReqListID(LinkedList<IDMSGcnt> &ReqList, int TempID)
    {    
	SortReqList(ReqList);
    ReqList.Reset();

    int temp=-1;

	if(ReqList.ListEmpty()) 
	{
		return temp;
	}
	else
	{
		while(!ReqList.EndOfList())
        {
        if(ReqList.Data().TempID > TempID)
            {
            return temp;
            }
        else if(ReqList.Data().TempID == TempID)
            {
             return ReqList.CurrentPosition();
            }
        else
            {
            ReqList.Next();
            }        
        }
	}

    

    return temp;
    }


void ReqListOutFile(LinkedList<IDMSGcnt> ReqList, ofstream &file)
    {
    ReqList.Reset();
    if(ReqList.ListEmpty())
        return;
    else
        {
        while(!ReqList.EndOfList())
            {
			//file<<"  ID "<<ReqList.Data().TempID<<" Time "<<ReqList.Data().Time
			//	<<" Position "<<ReqList.Data().GPSPos<<endl;
			file<<ReqList.Data()<<endl;
            ReqList.Next();
            }
        }
	file<<endl;

    }



IDMSGcnt RetrieveEntry(LinkedList<IDMSGcnt> & ReqList, int TempID)
{
	// TempID should be in the List...
	 ReqList.Reset();

	IDMSGcnt Temp;
    while(!ReqList.EndOfList())
        {
        if(ReqList.Data().TempID == TempID)
            {
				Temp=ReqList.Data();
				ReqList.DeleteAt();
            }
        else
            {
            ReqList.Next();
            }        
        }

	return Temp;
}


