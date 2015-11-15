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
#define KEY_NUM		4

#include "RecordFile.h"
#include "PageFile.h"
#include <string.h>

#ifdef _DEBUG_FLAG
  #include <stdio.h>
#endif
typedef struct {
  RecordId	m_data;
  int		m_key;
} LeafItem;

typedef struct {
  PageId	m_data;
  int		m_key;
} NonLeafItem;


union Buffer {
  char _buffer[PageFile::PAGE_SIZE];
  struct {
     int      count;
     PageId   ptr;
     int      type;
     LeafItem item[KEY_NUM];
  } Leaf;
  struct  {
     int         count;
     PageId      ptr;
     int         type;
     NonLeafItem item[KEY_NUM];
  } NonLeaf;
  struct {
    int		count;
    PageId	ptr;
    int		type;
  } Node;
};

class BTNode {
  public:
    
    int getKeyCount();
    int setKeyCount(int n);
    int getType();

    RC read(PageId pid, const PageFile &pf);
    RC write(PageId pid, PageFile &pf);
    RC write(PageFile &pf);
    BTNode(int classType) {memset(&buffer, 0, PageFile::PAGE_SIZE); buffer.Node.type = classType; };
    BTNode(PageId pid, const PageFile& pf)  {read(pid, pf);};

    /*
     **** CAUTIONS : ******
     * Following functions have not been tested and ARE BUGGY
    BTNode(const BTNode& src) { memcpy(buffer._buffer,
				       src.buffer._buffer,
				       PageFile::PAGE_SIZE); }
    void swap(BTNode& src) {
      char t[PageFile::PAGE_SIZE];
      memcpy(t, buffer._buffer, PageFile::PAGE_SIZE);
      memcpy(buffer._buffer, 
	     src.buffer._buffer,
	     PageFile::PAGE_SIZE);
      memcpy(src.buffer._buffer, t, PageFile::PAGE_SIZE);
    }
    BTNode & operator = (const BTNode & src) { BTNode t(src); t.swap(*this); return *this;}
    */

    bool upgrade() { if (buffer.Node.type != TYPE_BTNONLEAF) return false; buffer.Node.type = TYPE_BTROOT; return true; }
    bool downgrade() { buffer.Node.type = TYPE_BTNONLEAF; return true; }
    virtual ~BTNode() {};


#ifdef _DEBUG_FLAG    
    // DEBUGGING INFO
    LeafItem getLItem(int n) { return buffer.Leaf.item[n]; }
    NonLeafItem getNItem(int n) { return buffer.NonLeaf.item[n]; }
    char* getBuffer() {return buffer._buffer; }
    void print()
    {
      printf("Count: %d\n", getKeyCount());
      if (buffer.Node.type == TYPE_BTLEAF)
	  printf("Leaf\nP\tKey\tpid\tsid\tKey\tpid\tsid\n");
      else
	printf("Nonleaf\nP\tKey\tpid\tKey\tpid\n");
      for (int i = 0 ; i < getKeyCount(); i++)
      	if (buffer.Node.type == TYPE_BTLEAF)
	  printf("%d\t%d\t%d\t%d\t%X\t%X\t%X\n",i,
	      buffer.Leaf.item[i].m_key,
	      buffer.Leaf.item[i].m_data.pid,
      	      buffer.Leaf.item[i].m_data.sid,
	      buffer.Leaf.item[i].m_key,
	      buffer.Leaf.item[i].m_data.pid,
	      buffer.Leaf.item[i].m_data.sid);
      	else
	  printf("%d\t%d\t%d\t%X\t%X\n",i,
	      buffer.NonLeaf.item[i].m_key,
	      buffer.NonLeaf.item[i].m_data,
	      buffer.NonLeaf.item[i].m_key,
	      buffer.NonLeaf.item[i].m_data);
      printf("\nPtr: %d %X\n", *(int*)(buffer._buffer+sizeof(int)),*(int*)(buffer._buffer+sizeof(int)));
    }
#endif

  protected:
    Buffer buffer;
    void buffercpy(char* b) { memcpy(buffer._buffer, b, PageFile::PAGE_SIZE); } 
  private : 
    PageId m_pid;
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
    bool validEntry(int);

   /**** FOLLOWING FUNCTION IMPLEMENTED BY BASE CALSS *****/

    // int getKeyCount();
    // RC read(PageId pid, const PageFile& pf);
    // RC write(PageId pid, PageFile& pf);
}; 


/**
 * BTNonLeafNode: The class representing a B+tree nonleaf node.
 */
class BTNonLeafNode : public BTNode{
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

}; 

#endif /* BTREENODE_H */
