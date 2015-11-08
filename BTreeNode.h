/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 5/28/2008
 */

#ifndef BTREENODE_H
#define BTREENODE_H

#define TYPE_BTLEAF	1
#define TYPE_BTNONLEAF	2
#define TYPE_BTROOT	4
#define KEY_NUM		80

#include "RecordFile.h"
#include "PageFile.h"
#include <stdlib.h>


struct LeafItem{
  RecordId	m_data;
  int		m_key;
};

struct NonLeafItem{
  PageId	m_data;
  int		m_key;
};


union Buffer {
  char _buffer[PageFile::PAGE_SIZE];
  struct {
     LeafItem item[KEY_NUM];
     PageId   next;
     int      count;
  } Leaf;
  struct  {
     NonLeafItem item[KEY_NUM];
     PageId      next;
     int         count;
  } NonLeaf;
};

class BTNode {
  public:
    int getKeyCount();
    int setKeyCount(int n);
    RC read(PageId pid, const PageFile &pf);
    RC write(PageId pid, PageFile &pf);
    BTNode(int classType) :m_class(classType) {};
    int getType();
     virtual ~BTNode() {};
  protected :
    Buffer buffer;
  private:
    int m_class;
};
/**
 * BTLeafNode: The class representing a B+tree leaf node.
 */
class BTLeafNode : public BTNode{
  public:
    BTLeafNode() : 
      BTNode(TYPE_BTLEAF) {};
    // virtual ~BTLeafNode();
    
    RC insert(int key, const RecordId& rid);

    RC insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey);

    RC locate(int searchKey, int& eid);

    RC readEntry(int eid, int& key, RecordId& rid);

    PageId getNextNodePtr();

    RC setNextNodePtr(PageId pid);


   /**** FOLLOWING FUNCTION IMPLEMENTED BY BASE CALSS *****/

    // int getKeyCount();
    // RC read(PageId pid, const PageFile& pf);
    // RC write(PageId pid, PageFile& pf);
  private:
}; 


/**
 * BTNonLeafNode: The class representing a B+tree nonleaf node.
 */
class BTNonLeafNode : BTNode{
  public:
    BTNonLeafNode() 
      : BTNode(TYPE_BTNONLEAF) {} ;

    // virtual ~BTNonLeafNode();
    
    RC insert(int key, PageId pid);

    RC insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey);

    RC locateChildPtr(int searchKey, PageId& pid);

    RC initializeRoot(PageId pid1, int key, PageId pid2);

   /**** FOLLOWING FUNCTION IMPLEMENTED BY BASE CALSS *****/
   
    // int getKeyCount();
    // RC read(PageId pid, const PageFile& pf);
    // RC write(PageId pid, PageFile& pf);

  private:
   /**
    * The main memory buffer for loading the content of the disk page 
    * that contains the node.
    */
}; 

#endif /* BTREENODE_H */
