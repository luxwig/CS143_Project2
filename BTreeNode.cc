#include <string.h>
#include "BTreeNode.h"

#define GET_ITEM_NONLEAF(INDEX) buffer.NonLeaf.item[INDEX]
#define GET_ITEM_LEAF(INDEX) 	buffer.Leaf.item[INDEX]
#define GET_ITEM(TYPE,INDEX) 	GET_ITEM_##TYPE(INDEX)

#define GET_PTR_NONLEAF		buffer.NonLeaf.ptr
#define GET_PTR_LEAF		buffer.Leaf.ptr
#define GET_PTR(TYPE)		GET_PTR_##TYPE

#define INSERT_NODE(TYPE, DATA) {							\
  int n = getKeyCount();								\
  LOCATE_NODE(TYPE, key)								\
  int i = result + 1;									\
  memmove(&GET_ITEM(TYPE,i+1), &GET_ITEM(TYPE,i), sizeof(ITEM##TYPE)*(n-i+1));		\
  GET_ITEM(TYPE,i).m_data = DATA;							\
  GET_ITEM(TYPE,i).m_key = key;								\
}

#define INSERT_SPLIT_NODE(TYPE, DATA) {\
  if (sibling.getKeyCount() != 0) return RC_NODE_FULL;					\
  INSERT_NODE(TYPE, DATA)								\
  int n = getKeyCount() + 1,								\
      left = n/2; 									\
  PageId ptr = GET_PTR(TYPE); 								\
  char buf_old[PageFile::PAGE_SIZE] = {},						\
       buf_new[PageFile::PAGE_SIZE] = {};						\
  memcpy(buf_old + sizeof(int)*2, &GET_ITEM(TYPE, 0), (left)*sizeof(ITEM##TYPE));	\
  memcpy(buf_new + sizeof(int)*2, &GET_ITEM(TYPE, left),	 			\
	 PageFile::PAGE_SIZE - left*sizeof(ITEM##TYPE));				\
  buffercpy(buf_old);									\
  sibling.buffercpy(buf_new);  								\
  GET_PTR(TYPE) = ptr;									\
  sibling.GET_PTR(TYPE) = ptr;								\
  setKeyCount(left);									\
  sibling.setKeyCount(n-left);								\
}

#define LOCATE_NODE(TYPE, KEY) 			\
  int high = getKeyCount() - 1,			\
      low = 0,					\
      mid = (high + low)/2;			\
  while (low <= high) {				\
    mid = (high + low)/2;			\
    int midKey = GET_ITEM(TYPE, mid).m_key;	\
    if (midKey == KEY) break;			\
    if (KEY > midKey) low = mid + 1;		\
    else high = mid - 1;			\
  }						\
  int result;					\
  if (GET_ITEM(TYPE, mid).m_key == KEY)		\
  	result = mid;				\
  else result = high;				

using namespace std;

typedef LeafItem 	ITEMLEAF;
typedef NonLeafItem 	ITEMNONLEAF;

// ************************
// BTNode
// ************************

int BTNode::getKeyCount() 
{
  return buffer.Node.count;
}

RC BTNode::read(PageId pid, const PageFile &opf)
{
  return opf.read(pid, buffer._buffer);
}

RC BTNode::write(PageId pid, PageFile &pf)
{
  return pf.write(pid, buffer._buffer);
} 

int BTNode::getType() { return buffer.Node.type; }

int BTNode::setKeyCount(int n)
{
  if (n > KEY_NUM) return -1;
  buffer.Node.count = n;
  return 0;
}


// ************************
// *BTLeafNode
// ************************

RC BTLeafNode::insert(int key, const RecordId& rid)
{
  if (getKeyCount() == KEY_NUM) return RC_NODE_FULL;
  INSERT_NODE(LEAF, rid);
  setKeyCount(getKeyCount() + 1);
  return 0;
}

bool BTLeafNode::validEntry(int eid) {return eid < getKeyCount();}

RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{
  INSERT_SPLIT_NODE(LEAF, rid);
  siblingKey = sibling.GET_ITEM(LEAF, 0).m_key;
  return 0;
} 

RC BTLeafNode::locate(int searchKey, int& eid)
{
  LOCATE_NODE(LEAF, searchKey);
  if (GET_ITEM(LEAF, result).m_key == searchKey) {
    eid = result;
    return 0;
  }
  return RC_NO_SUCH_RECORD; 

}

RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
  if (!validEntry(eid)) return RC_NO_SUCH_RECORD;
  key = GET_ITEM(LEAF, eid).m_key;
  rid = GET_ITEM(LEAF, eid).m_data;
  return 0; 
} 

PageId BTLeafNode::getNextNodePtr()
{ return buffer.Leaf.ptr; }

RC BTLeafNode::setNextNodePtr(PageId pid)
{
  GET_PTR(LEAF) = pid; 
  return 0; 
} 

// ************************
// *BTNonLeafNode
// ************************

RC BTNonLeafNode::insert(int key, PageId pid)
{
  if (getKeyCount() == KEY_NUM) return RC_NODE_FULL;
  INSERT_NODE(NONLEAF, pid);
  setKeyCount(getKeyCount() + 1);
  return 0;
}


RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
  INSERT_SPLIT_NODE(NONLEAF, pid);
  midKey = sibling.GET_ITEM(NONLEAF,0).m_key;
  sibling.GET_PTR(NONLEAF) = sibling.GET_ITEM(NONLEAF,0).m_data;
  memmove(&sibling.GET_ITEM(NONLEAF,0),
          &sibling.GET_ITEM(NONLEAF,1),
	  PageFile::PAGE_SIZE - sizeof(int)*2 - sizeof(ITEMNONLEAF));
  sibling.setKeyCount(sibling.getKeyCount() - 1);
  return 0;
} 
#include <stdio.h>
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
  LOCATE_NODE(NONLEAF, searchKey);
  pid = (result==-1) ? GET_PTR(NONLEAF) : GET_ITEM(NONLEAF, result).m_data;
  return 0;
}

RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
  insert(key, pid2);
  GET_PTR(NONLEAF) = pid1;
  upgrade();
  return 0;
} 
