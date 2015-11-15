#define _DEBUG_FLAG
#include "Bruinbase.h"
#include "PageFile.h"
#include "BTreeIndex.h"
#include "BTreeNode.h"

int main()
{
  PageFile pf("test.data", 'w');
  BTLeafNode* l = 0;
  for (int i = 0; i <=100; i+=20) 
  {
    delete l;
    l = new BTLeafNode;
    for (int j = i; j<=i+10; j+=5)
    {
      RecordId rid;
      rid.pid = j << 4;
      rid.sid = j << 8;
      l->insert(j, rid);
    }
    l->setNextNodePtr(i/20+4);
    if (i == 100) l->setNextNodePtr(-1);
    l->write(i/20+3,pf);
  }
  delete l;
  BTNonLeafNode* b = 0;
  b = new BTNonLeafNode;
  b->initializeRoot(3,20,4); b->insert(40,5); b->downgrade(); b->write(1, pf);
  delete b;
  b = new BTNonLeafNode;
  b->initializeRoot(6,80,7); b->insert(100,8); b->downgrade(); b->write(2,pf);
  delete b;
  b = new BTNonLeafNode;
  b->initializeRoot(1,60,2); b->write(0,pf);
  delete b;
  pf.close();
}
