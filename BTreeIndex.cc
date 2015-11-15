/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#define BUFFER_SIZE 1024

#include "BTreeIndex.h"

using namespace std;

void BTreeIndex::locateLeafNode(SmartNodePtr& ptr, int searchKey, PageId& pid)
{
  ptr.TreeNode = new BTNode(0, pf);
  addPtr(ptr);
  while (ptr.TreeNode->getType() != TYPE_BTLEAF)
  {
    ptr.NonLeafNode->locateChildPtr(searchKey, pid);
    ptr.TreeNode = new BTNode(pid, pf);
    addPtr(ptr);
  }
}
/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    buf = (SmartNodePtr*) malloc(sizeof(SmartNodePtr) * BUFFER_SIZE);
    buf_size = BUFFER_SIZE;
    size = 0;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    return pf.open(indexname, mode);
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    SmartNodePtr ptr;
    ptr.TreeNode = new BTNode(0, pf);
    addPtr(ptr);
    PageId pid;
    while (ptr.TreeNode->getType() != TYPE_BTLEAF)
    {
      ptr.NonLeafNode->locateChildPtr(key, pid);
      ptr.TreeNode = new BTNode(pid, pf);
    }
    
    if (!ptr.LeafNode->insert(key, rid)) { 
      ptr.LeafNode->write(pf);
      clear();
      return 0;
    }

    // leafnode overflow
    SmartNodePtr ptrS; ptrS.LeafNode = new BTLeafNode;
    int keyS;
    ptr.LeafNode->insertAndSplit(key, rid, *ptrS.LeafNode, keyS);
    pid = pf.endPid();
    ptr.LeafNode->setNextNodePtr(pid);
    ptr.LeafNode->write(pf);
    ptrS.LeafNode->write(pid, pf);
    delete ptrS.LeafNode;
    ptrS.LeafNode = NULL;
    delPtr();
    bool flag = false;

    //nonleafnode overflow
    while (getPtr().NonLeafNode->insert(keyS, pid)||getPtr().NonLeafNode->write(pf))
    {
      delete ptrS.TreeNode;
      ptrS.NonLeafNode = new BTNonLeafNode;
      getPtr().NonLeafNode->insertAndSplit(keyS, pid, *ptrS.NonLeafNode, keyS);
      getPtr().NonLeafNode->write(pf);
      delPtr();
      pid = pf.endPid();
      ptrS.NonLeafNode->write(pid, pf);
      if (size == 0) { flag = true; break; }
    }
    delete ptrS.TreeNode;
    
    // root overflow
    if (flag) {
      ptr.NonLeafNode = new BTNonLeafNode;
      ptr.NonLeafNode->initializeRoot(pf.endPid(), keyS, pf.endPid()-1);
      ptrS.NonLeafNode = new BTNonLeafNode;
      ptrS.NonLeafNode->read(0, pf);
      ptrS.NonLeafNode->downgrade();
      ptrS.NonLeafNode->write(pf.endPid(), pf);
      delete ptrS.TreeNode;
      ptr.NonLeafNode->write(0, pf);
      delete ptr.NonLeafNode;
    }
    clear();
    return 0;
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
  SmartNodePtr ptr;
  PageId pid;
  locateLeafNode(ptr, searchKey, pid);
  if (ptr.LeafNode->locate(searchKey, cursor.eid)) {
    clear();
    return RC_NO_SUCH_RECORD; }
  clear();
  cursor.pid = pid;
  return 0; 
}

RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    SmartNodePtr ptr;
    ptr.TreeNode = new BTNode(cursor.pid, pf);
    if (ptr.LeafNode->readEntry(cursor.eid++, key, rid))
      return RC_NO_SUCH_RECORD;
    if (!ptr.LeafNode->validEntry(cursor.eid))
    {
      cursor.eid--;
      PageId pid = ptr.LeafNode->getNextNodePtr();
      if (pid  == -1) return RC_END_OF_TREE;
      cursor.pid = pid; cursor.eid = 0;
    }
    return 0;
}
