
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
    printf("%.4x ", *((int*)(b+i)));
    if (i%64==60) printf("\n%.4x:\t",i);
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

int main()
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
