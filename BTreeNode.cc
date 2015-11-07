#include "BTreeNode.h"
#include <string.h>
using namespace std;



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
/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */

/******* THE FOLLOWING 3 FUNCTIONS ARE IMPLEMENTED BY BASS CLASS **********

RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ return 0; }
    
 *
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 *

 RC BTLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }

 *
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 *
int BTLeafNode::getKeyCount()
{ return 0; }
*/

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
  int n = getKeyCount();
  if (n == KEY_NUM) return RC_NODE_FULL;
  int i;
  for (i = 0; i < n; i++)
    if (buffer.Leaf.item[n].m_key > key) break;
  memmove(buffer.Leaf.item+i+1, 
          buffer.Leaf.item+i, 
	  sizeof(LeafItem)*(n-i+1));
  buffer.Leaf.item[i].m_rid = rid;
  buffer.Leaf.item[i].m_key = key;
  n++;
  return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{return 0; } // TODO

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ return 0; } // TODO

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ return 0; } // TODO

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ return 0; } // TODO

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ return 0; } // TODO


/******* THE FOLLOWING 3 FUNCTIONS ARE IMPLEMENTED BY BASS CLASS **********

 *
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 *
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ return 0; }
    
 *
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 *
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }

 *
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 *
int BTNonLeafNode::getKeyCount()
{ return 0; }
*/

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
  int n = getKeyCount();
  if (n == KEY_NUM) return RC_NODE_FULL;
  int i;
  for (i = 0; i < n; i++)
    if (buffer.NonLeaf.item[n].m_key > key) break;
  memmove(buffer.NonLeaf.item+i+1, 
          buffer.NonLeaf.item+i, 
	  sizeof(NonLeafItem)*(n-i+1));
  buffer.NonLeaf.item[i].m_pid = pid;
  buffer.NonLeaf.item[i].m_key = key;
  n++;
  return 0;
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; } // TODO

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; } // TODO

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ return 0; } // TODO

