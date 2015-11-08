
#define _DEBUG_FLAG
#include "Bruinbase.h"
#include "PageFile.h"
#include "BTreeIndex.h"
#include "BTreeNode.h"

#include <stdio.h>


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
  int x;
  n.insertAndSplit(81, 81<<4, q, x);
  printBuffer(n.getBuffer());
  printBuffer(q.getBuffer());
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
  int x;
  n.insertAndSplit(79, 79<<4, q, x);
  printBuffer(n.getBuffer());
  printBuffer(q.getBuffer());
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

void insertSplitLeafTest1()
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
  rid.pid = (79) << 4;
  rid.sid = (79) << 8;
  n.insertAndSplit(79, rid, q, x);
  printBuffer(n.getBuffer());
  printBuffer(q.getBuffer());
  printf("%d\n", x);
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
  rid.pid = (79) << 4;
  rid.sid = (79) << 8;
  n.insertAndSplit(79, rid, q, x);
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


int main()
{
  insertSplitLeafTest1();
  insertSplitLeafTest2();
  printf("%d\n",insertSplitLeafTest_None_Empty_Sibling_ERROR());
}
