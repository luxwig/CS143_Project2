/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <limits.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

#define MAX(X,Y) ((X>Y)?X:Y)
#define MIN(X,Y) ((X<Y)?X:Y)
#define GOTO_NEXT {flag = false; break;}

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  BTreeIndex index;

  // open the table file
 
  int cond_size = cond.size();
  bool only_value = true;
  bool need_value = false;
  if (cond_size == 0) only_value = false;
  for (int i = 0; i < cond_size; i++) {
    if (cond[i].attr == 1 && cond[i].comp!=SelCond::NE) only_value = false;
    if (cond[i].attr == 2) need_value = true; }
  if (only_value || (index.open(table + ".idx", 'r') != 0)) {
  
  // scan the table file from the beginning
  
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  rid.pid = rid.sid = 0;
  count = 0;
  while (rid < rf.endRid()) {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }

    // check the conditions on the tuple
    for (unsigned i = 0; i < cond.size(); i++) {
      // compute the difference between the tuple value and the condition value
      switch (cond[i].attr) {
      case 1:
	diff = key - atoi(cond[i].value);
	break;
      case 2:
	diff = strcmp(value.c_str(), cond[i].value);
	break;
      }

      // skip the tuple if any condition is not met
      switch (cond[i].comp) {
      case SelCond::EQ:
	if (diff != 0) goto next_tuple;
	break;
      case SelCond::NE:
	if (diff == 0) goto next_tuple;
	break;
      case SelCond::GT:
	if (diff <= 0) goto next_tuple;
	break;
      case SelCond::LT:
	if (diff >= 0) goto next_tuple;
	break;
      case SelCond::GE:
	if (diff < 0) goto next_tuple;
	break;
      case SelCond::LE:
	if (diff > 0) goto next_tuple;
	break;
      }
    }

    // the condition is met for the tuple. 
    // increase matching tuple counter
    count++;

    // print the tuple 
    switch (attr) {
    case 1:  // SELECT key
      fprintf(stdout, "%d\n", key);
      break;
    case 2:  // SELECT value
      fprintf(stdout, "%s\n", value.c_str());
      break;
    case 3:  // SELECT *
      fprintf(stdout, "%d '%s'\n", key, value.c_str());
      break;
    }

    // move to the next tuple
    next_tuple:
    ++rid;
  }

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  index.close();
  return rc;
  }

  //// index  used
  // set range to read
  int range_min = INT_MIN,
      range_max = INT_MAX;
  for (int i = 0; i < cond_size; i++)
    if(cond[i].attr == 1)
      switch (cond[i].comp) {
	case SelCond::EQ: 
	  	 range_min = MAX(range_min, atoi(cond[i].value));
		 range_max = MIN(range_max, atoi(cond[i].value));
		 break;
	case SelCond::LT: 
	case SelCond::LE:
		 range_max = MIN(range_max, atoi(cond[i].value)-(cond[i].comp==SelCond::LT));
		 break;
	case SelCond::GT: 
	case SelCond::GE:
		 range_min = MAX(range_min, atoi(cond[i].value)+(cond[i].comp==SelCond::GT));
		 break;
      }
  if (range_min>range_max) goto no_result;
 
  // locate starting entry
  IndexCursor ic;
  count = 0;
  rc = index.locate(range_min, ic);
  if (ic.pid == -1) goto no_result;

  if (attr == 2 || attr == 3) need_value = true;
  // iteration
  
  if (need_value && (rc = rf.open(table + ".tbl", 'r')) < 0) {
      fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
      goto exit_select;
   }

  while (index.readForward(ic, key, rid) != RC_NO_SUCH_RECORD)
  {
    if (key > range_max) break; 

    // read value on demand
    if (need_value &&(rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }

    bool flag = true;
    // check constrain
    for (int i = 0; i < cond_size; i++)
    {
      switch (cond[i].attr) {
      case 1:
	diff = key - atoi(cond[i].value);
	break;
      case 2:
	diff = strcmp(value.c_str(), cond[i].value);
	break;
      }

      // skip the tuple if any condition is not met
      if (cond[i].comp == SelCond::EQ && diff != 0) GOTO_NEXT
      if (cond[i].comp == SelCond::NE && diff == 0) GOTO_NEXT
      if (cond[i].comp == SelCond::GT && diff <= 0) GOTO_NEXT
      if (cond[i].comp == SelCond::LT && diff >= 0) GOTO_NEXT
      if (cond[i].comp == SelCond::GE && diff < 0) GOTO_NEXT
      if (cond[i].comp == SelCond::LE && diff > 0) GOTO_NEXT
    }

    if (!flag) continue;
    switch (attr) {
      case 1:  // SELECT key
        fprintf(stdout, "%d\n", key);
        break;
      case 2:  // SELECT value
        fprintf(stdout, "%s\n", value.c_str());
        break;
      case 3:  // SELECT *
        fprintf(stdout, "%d '%s'\n", key, value.c_str());
        break;
    }
    count++;
  }
  if (attr == 4) printf("%d\n", count);
  goto exit_select;


no_result:
  rc = 0; if (attr==4) printf("0\n"); goto exit_select;
 
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
// Open loadfile for reading
  RecordFile rf;   
  RecordId   rid; 
  RC     rc;
  BTreeIndex tree;  
  
  string line; 
  int    key; 
  string value; 

  ifstream tableData(loadfile.c_str());
  
  //check if loadfile can be opened
  if(!tableData.is_open())
  fprintf(stderr, "Error: loadfile %s cannot be opened\n", loadfile.c_str());
  
  //open or create specified table file
  rc = rf.open(table + ".tbl", 'w');
  
  if(index)
  {
  tree.open(table + ".idx", 'w');
  
  //get line from loadfile
    while(getline(tableData, line))
    {
    parseLoadLine(line, key, value);
    if(rf.append(key, value, rid)!=0)
      return RC_FILE_WRITE_FAILED;
    
    //insert (key, rid) pair into BTree for indexing
    if(tree.insert(key, rid)!=0)
      return RC_FILE_WRITE_FAILED;
    }
  
  tree.close();
  }
  else
  {
    //get line from loadfile
    while(getline(tableData, line))
    {
    parseLoadLine(line, key, value);
    rc = rf.append(key, value, rid);  
    }
  }
  
  //close RecordFile and the loadfile
  rf.close();
  tableData.close();
  
  return rc;
}
RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
