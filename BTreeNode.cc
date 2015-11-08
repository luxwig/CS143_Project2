#include <string.h>
#include "BTreeNode.h"
#include <stdio.h>
#define GET_ITEM_NONLEAF(INDEX) buffer.NonLeaf.item[INDEX]
#define GET_ITEM_LEAF(INDEX) 	buffer.Leaf.item[INDEX]
#define GET_ITEM(TYPE,INDEX) 	GET_ITEM_##TYPE(INDEX)

// FIXME :: the sareching method should be updated once locate is implented

#define INSERT_NODE(TYPE, DATA) {		\
  int n = getKeyCount();			\
  if (n == KEY_NUM) return RC_NODE_FULL;	\
  int i;					\
  for (i = 0; i < n; i++)			\
    if (GET_ITEM(TYPE,i).m_key > key) break;	\
  memmove(&GET_ITEM(TYPE,i+1), 			\
          &GET_ITEM(TYPE,i), 			\
	  sizeof(ITEM##TYPE)*(n-i+1));		\
  GET_ITEM(TYPE,i).m_data = DATA;		\
  GET_ITEM(TYPE,i).m_key = key;			\
  setKeyCount(n+1);				\
  return 0;					\
}


#define INSERT_SPLIT_NODE(TYPE, DATA) {\
  if (sibling.getKeyCount() != 0) return RC_NODE_FULL;	\
  int n = getKeyCount(),				\
      leftKey = GET_ITEM(TYPE, n/2-1).m_key,		\
      left = (n-1*(leftKey > key)+1*(leftKey<key))/2;	\
  char buf_old[PageFile::PAGE_SIZE],			\
       buf_new[PageFile::PAGE_SIZE];			\
  memcpy(buf_old, 					\
         buffer._buffer,				\
         left*sizeof(ITEM##TYPE));			\
  memcpy(buf_new, 					\
         buffer._buffer + left*sizeof(ITEM##TYPE),	\
	 PageFile::PAGE_SIZE - left*sizeof(ITEM##TYPE));\
  buffercpy(buf_old);					\
  sibling.buffercpy(buf_new);  				\
  /* Memset for the sake of easy debugging		\
   * TODO : REMOVE memset */				\
  memset(buffer._buffer + left*sizeof(ITEM##TYPE),	\
         0,						\
	 PageFile::PAGE_SIZE - left*sizeof(ITEM##TYPE));\
  memset(sibling.buffer._buffer + PageFile::PAGE_SIZE - left*sizeof(ITEM##TYPE),					 \
         0,						\
	 left*sizeof(ITEM##TYPE));			\
  setKeyCount(left);					\
  sibling.setKeyCount(n-left);				\
							\
  if (key < leftKey) insert(key, DATA);			\
  else sibling.insert(key, DATA);			\
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


// ************************
// *BTLeafNode
// ************************

RC BTLeafNode::insert(int key, const RecordId& rid)
{
  INSERT_NODE(LEAF, rid);
}

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
  if (eid >= getKeyCount()) return RC_NO_SUCH_RECORD;
  key = GET_ITEM(LEAF, eid).m_key;
  rid = GET_ITEM(LEAF, eid).m_data;
  return 0; 
} 

PageId BTLeafNode::getNextNodePtr()
{ return buffer.Leaf.next; }

RC BTLeafNode::setNextNodePtr(PageId pid)
{
  buffer.Leaf.next = pid; 
  return 0; 
} 


/******* THE FOLLOWING 3 FUNCTIONS ARE IMPLEMENTED BY BASS CLASS **********
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ return 0; }
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }
int BTNonLeafNode::getKeyCount()
{ return 0; }
*/

// ************************
// *BTNonLeafNode
// ************************

RC BTNonLeafNode::insert(int key, PageId pid)
{
  INSERT_NODE(NONLEAF, pid);
}

void printBuffer1(char* b)
{
  printf("0000:\t");
  for (int i = 0; i < PageFile::PAGE_SIZE; i+=4)
  {
    printf("%.4x ", *((int*)(b+i)));
    if (i%64==60) printf("\n%.4x:\t",i);
  }
  printf("\n\n");
}

RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
  INSERT_SPLIT_NODE(NONLEAF, pid);
  midKey = sibling.GET_ITEM(NONLEAF,0).m_key;
  return 0;
} 

RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
  LOCATE_NODE(NONLEAF, searchKey);
  if (GET_ITEM(NONLEAF, result).m_key == searchKey) {
    pid = GET_ITEM(NONLEAF, result).m_data;
    return 0;
  }
  return RC_NO_SUCH_RECORD; 

}

RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
  insert(key, pid1);
  buffer.NonLeaf.next = pid2;
  upgrade();
  return 0;
} 
