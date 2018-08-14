#pragma once
#include "ReqNode.h"
using namespace std;

template <class T>
class LinkedList
{
   private:
      // pointers maintain access to front and rear of list
      ReqNode<T> *front, *rear;
      // used for data retrieval, insertion and deletion
      ReqNode<T> *prevPtr, *currPtr;
      // number of elements in the list
      int size;
      // position in list. used by Reset method
      int position;
      // private methods to allocate and deallocate ReqNodes
      ReqNode<T> *GetReqNode(T& item,ReqNode<T> *ptrNext=NULL);
      void FreeReqNode(ReqNode<T> *p);
      // copies list L to current list
      void CopyList( LinkedList<T>& L);
   public:
      // constructors
      LinkedList(void);
      LinkedList(LinkedList<T>& L);
      // destructor
      ~LinkedList(void);
      // assignment operator
      LinkedList<T>& operator= (LinkedList<T>& L);

      // methods to check list status
      int ListSize(void) const;
      int ListEmpty(void) const;

      // Traversal methods
      void Reset(int pos = 0);
      void Next(void);
      int EndOfList(void) const;
      int CurrentPosition(void) const;

      // Insertion methods
      void InsertFront( T& item);
      void InsertRear( T& item);
      void InsertAt( T& item);
      void InsertAfter(T& item);

      // Deletion methods
      T DeleteFront(void);
      void DeleteAt(void);

      // Data retrieval/modification
      T& Data(void);

      // method to clear the list
      void ClearList(void);

	  //method to display all the data
};

template <class T>
ReqNode<T> *LinkedList<T>::GetReqNode(T& item,ReqNode<T>* ptrNext)//good!
{
   ReqNode<T> *p;
   p=new ReqNode<T>(item,ptrNext);
   if(p==NULL)
   {
      cout << "Memory allocation failure!\n";
      exit(1);
   }
   return p;
}
template <class T>
void LinkedList<T>::FreeReqNode(ReqNode<T> *p)
{
   delete p;
}

// copy L to the current list, which is assumed to be empty
template <class T>
void LinkedList<T>::CopyList( LinkedList<T>& L)
{
   // use p to chain through L
   ReqNode<T> *p = L.front;
   int pos;

   // insert each element in L at the rear of current object
   while (p != NULL)
   {
      InsertRear(p->data);
      p = p->NextReqNode();
   }
   // if list is empty return
   if (position == -1)
      return ;

   // reset prevPtr and currPtr in the new list
   prevPtr = NULL;
   currPtr = front;
   for (pos = 0; pos != position; pos++)
   {
      prevPtr = currPtr;
      currPtr = currPtr->NextReqNode();
   }
}

// create empty list by setting pointers to NULL, size to 0
// and list position to -1
template <class T>
LinkedList<T>::LinkedList(void): front(NULL), rear(NULL),
      prevPtr(NULL),currPtr(NULL), size(0), position(-1)
{}

template <class T>
LinkedList<T>::LinkedList(LinkedList<T>& L)
{
   front = rear = NULL;
   prevPtr = currPtr = NULL;
   size = 0;
   position = -1;
  // CopyList(L);

   // use p to chain through L
   ReqNode<T> *p = L.front;
   int pos;

   // insert each element in L at the rear of current object
   while (p != NULL)
   {
      InsertRear(p->data);
      p = p->NextReqNode();
   }
   // if list is empty return
   if (position == -1)
      return ;

   // reset prevPtr and currPtr in the new list
   prevPtr = NULL;
   currPtr = front;
   for (pos = 0; pos != position; pos++)
   {
      prevPtr = currPtr;
      currPtr = currPtr->NextReqNode();
   }
}
template <class T>
void LinkedList<T>::ClearList(void)
{
   ReqNode<T> *currPosition, *nextPosition;

   currPosition = front;
   while(currPosition != NULL)
   {
      // get address of next ReqNode and delete current ReqNode
      nextPosition = currPosition->NextReqNode();
      FreeReqNode(currPosition);
      currPosition = nextPosition;  // Move to next ReqNode
   }
   front = rear = NULL;
   prevPtr = currPtr = NULL;
   size = 0;
   position = -1;
}

template <class T>
LinkedList<T>::~LinkedList(void)
{
   ClearList();
}

template <class T>
LinkedList<T>& LinkedList<T>::operator=( LinkedList<T>& L)
{
   if (this == &L)      // Can't assign list to itself
     return *this;

   ClearList();
  // CopyList(L);
   // use p to chain through L
   ReqNode<T> *p = L.front;
   int pos;

   // insert each element in L at the rear of current object
   while (p != NULL)
   {
      InsertRear(p->data);
      p = p->NextReqNode();
   }
   // if list is empty return
   if (position == -1)
   {
	 LinkedList<T> s;
     return s;
   }

   // reset prevPtr and currPtr in the new list
   prevPtr = NULL;
   currPtr = front;
   for (pos = 0; pos != position; pos++)
   {
      prevPtr = currPtr;
      currPtr = currPtr->NextReqNode();
   }
   return *this;
}
template <class T>
int LinkedList<T>::ListSize(void) const
{
   return size;
}
template <class T>
int LinkedList<T>::ListEmpty(void) const
{
   return size == 0;
}

// move prevPtr and currPtr forward one ReqNode
template <class T>
void LinkedList<T>::Next(void)
{
   // if traversal has reached the end of the list or
   // the list is empty, just return
   if (currPtr!=NULL)
   {
      // advance the two pointers one ReqNode forward
      prevPtr = currPtr;
      currPtr = currPtr->NextReqNode();
      position++;
   }
}
// True if the client has traversed the list
template <class T>
int LinkedList<T>::EndOfList(void) const
{
   return currPtr==NULL;
}
// return the position of the current ReqNode
template <class T>
int LinkedList<T>::CurrentPosition(void) const
{
   return position;
}
// reset the list position to pos
template <class T>
void LinkedList<T>::Reset(int pos)
{
   int startPos;
   // if the list is empty, return
   if (front == NULL)
      return;
   // if the position is invalid, terminate the program
   if (pos < 0 || pos > size-1)
   {
      cout<< "Reset: Invalid list position: "<<pos<< endl;
      return;
   }
   // move list traversal mechanism to ReqNode pos
   if(pos == 0)
   {
      // reset to front of the list
      prevPtr = NULL;
      currPtr = front;
      position = 0;
   }
   else
   // reset currPtr, prevPtr, and position
   {
       currPtr = front->NextReqNode();
       prevPtr = front;
       startPos = 1;
	   // move right until position == pos
	   for(position=startPos; position!=pos;position++)
	   {
	       // move both traversal pointers forward
	       prevPtr = currPtr;
	       currPtr = currPtr->NextReqNode();
      }
   }
}
// return a reference to the data value in the current ReqNode
template <class T>
T& LinkedList<T>::Data(void)
{
   // error if list is empty or traversal completed
   if (size==0)
   {
      cerr << "Empty List! Data: invalid reference!" << endl;
      exit(1);
   }
   else if (currPtr==NULL)
   {
	  cerr << "Null current pointer ! Data: invalid reference!" << endl;
      exit(1);
   }
   return currPtr->data;
}

// Insert item at front of list
template <class T>
void LinkedList<T>::InsertFront(T& item)
{
   // call Reset if the list is not empty
   if (front!=NULL)
      Reset();
   InsertAt(item);        // inserts at front
}

// Insert item at rear of list
template <class T>
void LinkedList<T>::InsertRear(T& item)
{
   ReqNode<T> *newReqNode;
   prevPtr=rear;
   newReqNode=GetReqNode(item);	// create the new ReqNode
   if (rear==NULL)	  			// if list empty, insert at front
      front = rear = newReqNode;
   else
   {
      rear->InsertAfter(newReqNode);
      rear = newReqNode;
   }
   currPtr = rear;
   position = size;
   size++;
}

// Insert item at the current list position
template <class T>
void LinkedList<T>::InsertAt(T& item)
{
   ReqNode<T> *newReqNode;

   // two cases: inserting at the front or inside the list
   if (prevPtr==NULL)
   {
      // inserting at the front of the list. also places
      // ReqNode into an empty list
      newReqNode=GetReqNode(item,front);
      front=newReqNode;
   }
   else
   {
      // inserting inside the list. place ReqNode after prevPtr
      newReqNode=GetReqNode(item);
      prevPtr->InsertAfter(newReqNode);
   }

   // if prevPtr == rear, we are inserting into empty list
   // or at rear of non-empty list; update rear and position
   if (prevPtr==rear)
   {
      rear=newReqNode;
      position=size;
   }

   // update currPtr and increment the list size
   currPtr=newReqNode;
   size++;              // increment list size
}

// Insert item after the current list position
template <class T>
void LinkedList<T>::InsertAfter(T& item)
{
   ReqNode<T> *p;

   p=GetReqNode(item);
   if (front==NULL)       // inserting into an empty list
   {
      front=currPtr=rear=p;
      position=0;
   }
   else
   {
      // inserting after last ReqNode of list
      if (currPtr == NULL)
		currPtr = prevPtr;

      currPtr->InsertAfter(p);

      if (currPtr==rear)
      {
		rear=p;
		position=size;
      }
      else
	    position++;

      prevPtr=currPtr;
      currPtr=p;
   }
   size++;              // increment list size
}

// Delete the ReqNode at the front of list
template <class T>
T LinkedList<T>::DeleteFront(void)
{
   T item;

   Reset();   // Put the currPtr to the front.

   if (front == NULL)   // If the list is empty
   {
      cerr << "Invalid deletion!" << endl;
      exit(1);
   }
   item = currPtr->data;

   DeleteAt();

   return item;
}

// Delete the ReqNode at the current list position
template <class T>
void LinkedList<T>::DeleteAt(void)
{
   ReqNode<T> *p;

   // error if empty list or at end of list
   if (currPtr == NULL)
   {
      cerr << "Invalid deletion!" << endl;
      exit(1);
   }

   // deletion must occur at front ReqNode or inside the list
   if (prevPtr == NULL)
   {
      // save address of front and unlink it. if this
      // is the last ReqNode, front becomes NULL
      p = front;
      front = front->NextReqNode();
   }
   else
      // unlink interior ReqNode after prevPtr. save address
      p = prevPtr->DeleteAfter();

   // if rear is deleted, new rear is prevPtr and position
   // is decremented; otherwise, position is the same
   // if p was last ReqNode, rear = NULL and position = -1
   if (p == rear)
   {
      rear = prevPtr;
      position--;
   }

   // move currPtr past deleted ReqNode. if p is last ReqNode
   // in the list, currPtr becomes NULL
   currPtr = p->NextReqNode();

   // free the ReqNode and decrement the list size
   FreeReqNode(p);
   size--;
}
