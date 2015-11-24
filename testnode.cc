
#define _DEBUG_FLAG
#include "Bruinbase.h"
#include "PageFile.h"
#include "BTreeIndex.h"
#include "BTreeNode.h"

#include <stdio.h>


#define NODEI(X) { 	\
  RecordId rid;		\
  rid.pid = X<<4;	\
  rid.sid = X<<8;	\
  n.insert(X, rid);	\
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


void insertNonLeafTest()
{
  BTNonLeafNode n,q;
  for (int i = 90; i >=0; i--)
  {
    printf("i = %d\tError = %d\n", i, n.insert(i, i<<4));
    printBuffer(n.getBuffer());  
  }
}

void insertLeafTest()
{
  BTLeafNode n,q;
  for (int i = 80; i >= 0; i--)
  {
    RecordId rid;
    rid.pid = i;
    rid.sid = i << 4;
    printf("i = %d\tError = %d\n", i, n.insert(i<<8, rid));
    printBuffer(n.getBuffer());  
  }
  PageFile pf ("NonLeaf.data", 'w');
  n.write(0,pf);
  pf.close();

  PageFile of ("NonLeaf.data", 'r');
  q.read(0,of);
  of.close();
  printBuffer(q.getBuffer());
}

void insertSplitNonLeafTest1()
{
  BTNonLeafNode n,q;
  for (int i = 1; i <= 80; i++)
  {
    n.insert(i*2,(i*2)<<4);
  }
  printBuffer(n.getBuffer());
  n.print();
  int x;
  n.insertAndSplit(81, 81<<4, q, x);
  printBuffer(n.getBuffer());
  n.print();
  printBuffer(q.getBuffer());
  q.print();
  printf("%d\n", x);
}


void insertSplitNonLeafTest2()
{
  BTNonLeafNode n,q;
  for (int i = 1; i <= 80; i++)
  {
    n.insert(i*2,(i*2)<<4);
  }
  printBuffer(n.getBuffer());
  n.print();
  int x;
  n.insertAndSplit(79, 79<<4, q, x);
  printBuffer(n.getBuffer());
  n.print();
  printBuffer(q.getBuffer());
  q.print();
  printf("%d\n", x);
}

void insertSplitNonLeafTest3()
{
  BTNonLeafNode n,q;
  for (int i = 1; i <= 79; i++)
  {
    n.insert(i*2,(i*2)<<4);
  }
  printBuffer(n.getBuffer());
  n.print();
  int x;
  n.insertAndSplit(81, 81<<4, q, x);
  printBuffer(n.getBuffer());
  n.print();
  printBuffer(q.getBuffer());
  q.print();
  printf("%d\n", x);
}


void insertSplitNonLeafTest4()
{
  BTNonLeafNode n,q;
  for (int i = 1; i <= 79; i++)
  {
    n.insert(i*2,(i*2)<<4);
  }
  printBuffer(n.getBuffer());
  n.print();
  int x;
  n.insertAndSplit(79, 79<<4, q, x);
  printBuffer(n.getBuffer());
  n.print();
  printBuffer(q.getBuffer());
  q.print();
  printf("%d\n", x);
}


int insertSplitNonLeafTest_None_Empty_Sibling_ERROR()
{
  BTNonLeafNode n;
  n.insert(1,1);
  BTNonLeafNode q;
  q.insert(1,1);
  int x;
  return n.insertAndSplit(1,3,q,x);
}

void insertSplitLeafTest(int t, int r)
{
  BTLeafNode n,q;
  n.setNextNodePtr(0xDEAD);
  for (int i = 1; i <= t; i++)
  {
    NODEI(i*2);
  }
  printBuffer(n.getBuffer());
  int x;
  RecordId rid;
  rid.pid = (r) << 4;
  rid.sid = (r) << 8;
  n.insertAndSplit(r, rid, q, x);
  printBuffer(n.getBuffer());
  n.print();
  printBuffer(q.getBuffer());
  q.print();
  printf("%d\n*********************\n", x);
}

void insertSplitLeafTest2()
{
  BTLeafNode n,q;
  for (int i = 1; i <= 80; i++)
  {
    RecordId rid;
    rid.pid = (i*2) << 4;
    rid.sid = (i*2) << 8;
    n.insert(i*2, rid);
  }
  printBuffer(n.getBuffer());
  int x;
  RecordId rid;
  rid.pid = (81) << 4;
  rid.sid = (81) << 8;
  n.insertAndSplit(81, rid, q, x);
  printBuffer(n.getBuffer());
  printBuffer(q.getBuffer());
  printf("%d\n", x);
}

int insertSplitLeafTest_None_Empty_Sibling_ERROR()
{
  BTLeafNode n;
  RecordId rid;
  rid.pid = (79) << 4;
  rid.sid = (79) << 8;
  n.insert(1,rid);
  BTLeafNode q;
  q.insert(1,rid);
  int x;
  return n.insertAndSplit(0,rid,q,x);
}


void locateChildPtr_Found()
{
  BTNonLeafNode n;
  for (int i = 1; i <= 80; i++)
  {
    n.insert(i,(i)<<4);
  }
  printBuffer(n.getBuffer());
  int pid;
  n.locateChildPtr(67, pid);
  printf("pid : %d\n",pid);
}

/*
void locateChildPtr_NotFound()
{
  BTNonLeafNode n;
  for (int i = 1; i <= 80; i++)
  {
    n.insert(i,(i*2)<<4);
  }
  printBuffer(n.getBuffer());
  int pid;
  printf("error: %d\n",n.locateChildPtr(67, pid));
  printf("pid : %d, %X\n",pid, pid);
}
*/

void locateChildPtr_Notfound()
{
  BTNonLeafNode n;
  n.initializeRoot(0xDEAD,1, 1<<4);
  for (int i = 2; i <=40; i++)
    n.insert(i, i<<4);
  for (int i = 42; i<=81; i++)
    n.insert(i, i<<4);
  printBuffer(n.getBuffer());
  int pid = 0xBEEF;
  n.locateChildPtr(0, pid);
  printf("0\tpid: %x\n", pid);
  n.locateChildPtr(89, pid);
  printf("89\tpid: %x\n", pid);
  n.locateChildPtr(41, pid);
  printf("41\tpid: %x\n", pid);
}

void locate_Found()
{
  BTLeafNode n;
  for (int i = 1; i <= 80; i++)
  {
    RecordId rid;
    rid.pid = (i) << 4;
    rid.sid = (i) << 8;
    n.insert(i, rid);
  }
  printBuffer(n.getBuffer());
  int eid;
  printf("error : %d\n",n.locate(67, eid));
  printf("eid : %d\n",eid);
  RecordId rid;
  n.readEntry(eid, eid, rid);
  printf("%d %d %d\n", eid, rid.pid, rid.sid);
}

void nextNodePtr()
{
  BTLeafNode n;
  NODEI(10);
  NODEI(3);
  NODEI(74);
  printBuffer(n.getBuffer());
  printf("%X\n", n.getNextNodePtr());
  n.setNextNodePtr(0xDEAD);
  printf("%X\n", n.getNextNodePtr());
  printBuffer(n.getBuffer());
}  


void locate_NotFound()
{
  BTLeafNode n;
  for (int i = 1; i <= 80; i++)
  {
    RecordId rid;
    rid.pid = (i*2) << 4;
    rid.sid = (i*2) << 8;
    n.insert(i*2, rid);
  }
  printBuffer(n.getBuffer());
  int eid;
  printf("error : %d\n",n.locate(3, eid));
  printf("eid : %d\n",eid);
}

void initializeRootTest()
{
  BTNonLeafNode n;
  n.initializeRoot(0xDEAD,0xBEEF, 0xFFFF);
  printBuffer(n.getBuffer());
}

int main()
{
  /*
  insertSplitLeafTest(80,79);
  insertSplitLeafTest(80,81);
  insertSplitLeafTest(79,79);
  insertSplitLeafTest(79,81);*/
  //locateChildPtr_Notfound();
  locate_NotFound();
}

