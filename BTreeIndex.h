/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#ifndef BTREEINDEX_H
#define BTREEINDEX_H

#include <stdlib.h>

#include "Bruinbase.h"
#include "PageFile.h"
#include "RecordFile.h"
#include "BTreeNode.h"
/**
 * The data structure to point to a particular entry at a b+tree leaf node.
 * An IndexCursor consists of pid (PageId of the leaf node) and 
 * eid (the location of the index entry inside the node).
 * IndexCursor is used for index lookup and traversal.
 */
typedef struct {
  // PageId of the index entry
  PageId  pid;  
  // The entry number inside the node
  int     eid;  
} IndexCursor;


union SmartNodePtr{
  BTNode*  	 TreeNode;
  BTLeafNode*    LeafNode;
  BTNonLeafNode* NonLeafNode;
  void*		 Node;
};
/*
SmartNodePtr& operator=(SmartNodePtr& lhs, SmartNodePtr rhs)
{ lhs.TreeNode = }*/

/**
 * Implements a B-Tree index for bruinbase.
 * 
 */
class BTreeIndex {
 public:
  BTreeIndex();

  /**
   * Open the index file in read or write mode.
   * Under 'w' mode, the index file should be created if it does not exist.
   * @param indexname[IN] the name of the index file
   * @param mode[IN] 'r' for read, 'w' for write
   * @return error code. 0 if no error
   */
  RC open(const std::string& indexname, char mode);

  /**
   * Close the index file.
   * @return error code. 0 if no error
   */
  RC close();
    
  /**
   * Insert (key, RecordId) pair to the index.
   * @param key[IN] the key for the value inserted into the index
   * @param rid[IN] the RecordId for the record being inserted into the index
   * @return error code. 0 if no error
   */
  RC insert(int key, const RecordId& rid);

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
   * @return 0 if searchKey is found. Othewise, an error code
   */
  RC locate(int searchKey, IndexCursor& cursor);

  /**
   * Read the (key, rid) pair at the location specified by the index cursor,
   * and move foward the cursor to the next entry.
   * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
   * @param key[OUT] the key stored at the index cursor location
   * @param rid[OUT] the RecordId stored at the index cursor location
   * @return error code. 0 if no error
   */
  RC readForward(IndexCursor& cursor, int& key, RecordId& rid);
  
  ~BTreeIndex() {clear(); free(buf);}
#ifdef _DEBUG_FLAG
  void addPtr(void *p)
  {SmartNodePtr ptr; ptr.Node = p; addPtr(ptr);}
#endif
 private:
  void addPtr(SmartNodePtr p)
  {
    if (size == buf_size) 
    { buf = (SmartNodePtr*) realloc(buf, buf_size*2); buf_size*=2;}
    buf[size++] = p;
  }
   
  void delPtr() { delete buf[size-1].TreeNode; size--; }
   SmartNodePtr getPtr() {return buf[size-1];}
  void clear() {while (size!=0) delPtr();}

  void locateLeafNode(SmartNodePtr&, int, PageId&);
  PageFile pf;         /// the PageFile used to store the actual b+tree in disk
  SmartNodePtr* buf;
  int		size;
  int		buf_size;
};

#endif /* BTREEINDEX_H */
