
#define _DEBUG_FLAG
#include "Bruinbase.h"
#include "PageFile.h"
#include "BTreeIndex.h"
#include "BTreeNode.h"

#include <stdio.h>





void readPage(int n)
{
  PageFile pf("test.data", 'r');
  if (n >= pf.endPid()) {
    printf("Error ID\n"); return;
  }
  printf("Page:\t%d\t%X\n",n,n);
  SmartNodePtr ptr;
  ptr.TreeNode = new BTNode(n,pf);
  if (ptr.TreeNode->getType() == TYPE_BTLEAF) ptr.LeafNode->print();
  else ptr.NonLeafNode->print();
  printf ("**********\n");
}

int main()
{
  PageFile pf("test.data", 'r');
  int n;
  while (scanf("%d",&n) && n != -1)
    if (n == -2){
      for (int i = 0; i < pf.endPid(); i++)
	readPage(i);
      break;
    }
    else readPage(n);
}
