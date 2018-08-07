#pragma once

template <class T>
class ReqNode
{
 private:
  ReqNode<T> *next;
 public:
  T data;
public:
  ReqNode(T ietm,ReqNode<T> *prenext= NULL);
  void InsertAfter(ReqNode<T>* p);
  ReqNode<T>* DeleteAfter(void);
  ReqNode<T>* NextReqNode(void) const;
public:
	~ReqNode(void);
};

template <class T>
ReqNode<T>::ReqNode(T ietm,ReqNode<T> *prenext)
{
 data=ietm;
 next=prenext;
}


template <class T>
ReqNode<T>* ReqNode<T>::NextReqNode(void) const
{
 return next;
}


template <class T>
void ReqNode<T>::InsertAfter(ReqNode<T>* p)
{
 p->next=next;
 next=p;
}


template <class T>
ReqNode<T>* ReqNode<T>::DeleteAfter(void)
{
 ReqNode<T>* temptr=next;
 if(next==NULL)
  return NULL;
 else
  {
   next=temptr->next;
   return temptr;
  }
}

template <class T>
ReqNode<T>::~ReqNode(void)
{
}
