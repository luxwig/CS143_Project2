#include <stdio.h>
#include <string.h>

#include "BTreeNode.h"


#define GET_ITEM_NONLEAF(INDEX) buffer.NonLeaf.item[INDEX]
#define GET_ITEM_LEAF(INDEX) 	buffer.Leaf.item[INDEX]
#define GET_ITEM(TYPE,INDEX) 	GET_ITEM_##TYPE(INDEX)

#define INSERT_NODE(TYPE, DATA) {\
  int n = getKeyCount();			\
  if (n == KEY_NUM) return RC_NODE_FULL;	\
  int i;					\
  for (i = 0; i < n; i++)			\
    if (GET_ITEM(TYPE,i).m_key > key) break;	\
  memmove(&GET_ITEM(TYPE,i+1), 		\
          &GET_ITEM(TYPE,i), 			\
	  sizeof(ITEM##TYPE)*(n-i+1));		\
  GET_ITEM(TYPE,i).m_data = DATA;		\
  GET_ITEM(TYPE,i).m_key = key;		\
  n++;						\
  return 0;					\
}

using namespace std;

typedef LeafItem 	ITEMLEAF;
typedef NonLeafItem 	ITEMNONLEAF;

int BTNode::getKeyCount() 
{
  return (m_class==TYPE_BTLEAF)?
    	 buffer.Leaf.count:
	 buffer.NonLeaf.count;
}

RC BTNode::read(PageId pid, const PageFile &opf)
{
  return opf.read(pid, buffer._buffer);
}

RC BTNode::write(PageId pid, PageFile &pf)
{
  return pf.write(pid, buffer._buffer);
} 

int BTNode::getType() { return m_class; }

int BTNode::setKeyCount(int n)
{
  if (n > KEY_NUM) return -1;
  if (m_class==TYPE_BTLEAF)
    buffer.Leaf.count = n;
  else
    buffer.NonLeaf.count = n;
  return 0;
}

RC BTLeafNode::insert(int key, const RecordId& rid)
{
  INSERT_NODE(LEAF, rid);
}

RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{return 0; } // TODO

RC BTLeafNode::locate(int searchKey, int& eid)
{ return 0; } // TODO

RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ return 0; } // TODO

PageId BTLeafNode::getNextNodePtr()
{ return 0; } // TODO

RC BTLeafNode::setNextNodePtr(PageId pid)
{ return 0; } // TODO


/******* THE FOLLOWING 3 FUNCTIONS ARE IMPLEMENTED BY BASS CLASS **********
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ return 0; }
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }
int BTNonLeafNode::getKeyCount()
{ return 0; }
*/

RC BTNonLeafNode::insert(int key, PageId pid)
{
  /*
  int n = getKeyCount();
  if (n == KEY_NUM) return RC_NODE_FULL;
  int i;
  for (i = 0; i < n; i++)
    if (buffer.NonLeaf.item[i].m_key > key) break;
  memmove(buffer.NonLeaf.item+i+1, 
          buffer.NonLeaf.item+i, 
	  sizeof(NonLeafItem)*(n-i+1));
  buffer.NonLeaf.item[i].m_pid = pid;
  buffer.NonLeaf.item[i].m_key = key;
  n++;
  return 0;
  */
  INSERT_NODE(NONLEAF, pid);
}

RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; } // TODO

RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; } // TODO

RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ return 0; } // TODO

