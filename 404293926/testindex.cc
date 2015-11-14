
#define _DEBUG_FLAG
#include "Bruinbase.h"
#include "PageFile.h"
#include "BTreeIndex.h"
#include "BTreeNode.h"

#include <stdio.h>


#define NODEI(N,X) { 	\
  RecordId rid;		\
  rid.pid = X<<4;	\
  rid.sid = X<<8;	\
  N.insert(X, rid);	\
}


void printBuffer(char* b)
{
  printf("0000:\t");
  for (int i = 0; i < PageFile::PAGE_SIZE; i+=4)
  {
    printf("%.4X ", *((int*)(b+i)));
    if (i%64==60) printf("\n%.4X:\t",i);
  }
  printf("\n\n");
}

void LeafNodeInit()
{
  BTLeafNode n,q;
  for (int i = 0; i < 80; i++)
  {
    NODEI(n,i);
    NODEI(q,i+80);   
  }
  n.setNextNodePtr(1);
  q.setNextNodePtr(-1);
  PageFile pf("test.data",'w');
  n.write(0, pf);
  q.write(1, pf);
  pf.close();
  pf.open("test.data", 'r');
  q.read(0, pf);
  n.read(1, pf);
  pf.close();
  q.print();
  n.print();
}

void readForward_Test()
{
  LeafNodeInit();
  BTreeIndex i;
  i.open("test.data",'r');
  IndexCursor ic;
  ic.pid = 0; ic.eid = 0;
  int key; RecordId rid;
  while (!i.readForward(ic, key, rid))
  {
    printf("pid:\t%X\teid:\t%X\tKey:%X\tRecord:\t%X\t%X\n",
	    ic.pid, ic.eid, key, rid.pid, rid.sid);
  }
}

void addPtr_Test()
{
  BTreeIndex i;
  for (int q = 0; q < 20; q++)
  	i.addPtr((BTNode*)(q<<8));
}


int main()
{
  readForward_Test();
}
