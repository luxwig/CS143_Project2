
#define _DEBUG_FLAG
#include "Bruinbase.h"
#include "PageFile.h"
#include "BTreeIndex.h"
#include "BTreeNode.h"

#include <stdio.h>

#undef KEY_NUM

#define KEY_NUM 4

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
  //	i.addPtr((BTNode*)(q<<8));
  	;
}



void generating()
{
  PageFile pf("test.data", 'w');
  BTLeafNode* l = 0;
  for (int i = 1; i <=41; i+=8) 
  {
    delete l;
    l = new BTLeafNode;
    for (int j = i; j<=i+4; j+=2)
    {
      RecordId rid;
      rid.pid = j << 4;
      rid.sid = j << 8;
      l->insert(j, rid);
    }
    l->setNextNodePtr((i-1)/8+4);
    if (i == 41) l->setNextNodePtr(-1);
    l->write((i-1)/8+3,pf);
  }
  delete l;
  BTNonLeafNode* b = 0;
  b = new BTNonLeafNode;
  b->initializeRoot(3,9,4); b->insert(17,5); b->downgrade(); b->write(1, pf);
  delete b;
  b = new BTNonLeafNode;
  b->initializeRoot(6,33,7); b->insert(41,8); b->downgrade(); b->write(2,pf);
  delete b;
  b = new BTNonLeafNode;
  b->initializeRoot(1,25,2); b->write(0,pf);
  delete b;
  pf.close();
}

void generatig_test()
{
  PageFile pf("test.data", 'r');
  for (int i = 0; i<=2; i++)
  {
    BTNonLeafNode b;
    b.read(i,pf); b.print();
    printf("TYPE:\t%d",b.getType());
  }
  for (int i = 3; i <=8; i++)
  {
    BTLeafNode b;
    b.read(i,pf); b.print();
    printf("TYPE:\t%d\n",b.getType());
  }
}

void locate_test()
{
  generating();
  printf("NUMBER\tEID\tPID\tNUMBER\tEID\tPID\n");
  for (int i = 0; i <46; i++) {
  BTreeIndex it;
  it.open("test.data", 'r');
  IndexCursor c;
  if (!it.locate(i,c)) printf("%d\t%d\t%d\t0x%X\t0x%X\t0x%X\n",i,c.eid,c.pid,i,c.eid,c.pid);
  else printf("%d\t---\t---\t0x%X\t---\t---\n",i,i);
  it.close();
  }

}
int main()
{
}

