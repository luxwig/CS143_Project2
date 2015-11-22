/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"
#include <set>

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
  IndexCursor cursor;

  if(cond.size() == 0 && attr == 4)
    ;
  else {
  // open the table file
    if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
      fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
      return rc;
    }
  }

 //If index exists, use index instead of table
  if (rc = index.open(table + ".idx", 'r') == 0)
  {
    int startKey = 0;
    int endKey = 2147483647;
    int startValue;
    int endValue;
    char* eqValue = NULL;

    set<RecordId> records;
    set<string> neValue;
    set<int> neKey;

    for (unsigned i = 0; i< cond.size(); i++)
    {
      switch(cond[i].comp) {
        case SelCond::EQ:
          //value equality 
          if (cond[i].attr == 2) {
            if(eqValue != NULL)
            {
              rf.close();
              return rc;
            }
            else eqValue = cond[i].value;
          }
          else {
            index.locate(atoi(cond[i].value), cursor);
            index.readForward(cursor, startValue, rid);

            //if the key stored at the corresponding is the same as our target
            if (atoi(cond[i].value) == startValue)
            {
              startKey = startValue;
              endKey = startKey + 1;
            }
            else return 0;
          }
          break;

        case SelCond::LT:
          if(atoi(cond[i].value) <= endKey)
            endKey = atoi(cond[i].value) - 1;
          break;
        case SelCond::LE:
          if(atoi(cond[i].value) < endKey)
            endKey = atoi(cond[i].value);
          break;
        case SelCond::GE:
          if(atoi(cond[i].value) > startKey)
            startKey = atoi(cond[i].value);
          break;
        case SelCond::NE:
          if(cond[i].attr == 2)
            neValue.insert(cond[i].value);
          else
            neKey.insert(atoi(cond[i].value));
          break;
      }

    }

    index.locate(startKey, cursor);

    //insert the records we need to read into the set
    while (!index.readForward(cursor, key, rid) && key < endKey)
    {
      records.insert(rid);
      count++;
    }

    set<RecordId>::iterator iter;

    for (iter = records.begin(); iter != records.end(); iter++)
    {
      rc = rf.read(*iter, key, value);

      set<int>::iterator keyIter;
      set<string>::iterator valIter;

      if(!neKey.empty() || !neValue.empty())
      {
        if(!neKey.empty())
        {
          keyIter = neKey.find(key);
          if(keyIter != neKey.end())
            continue;
        }
        if(!neValue.empty())
        {
          valIter = neValue.find(value);
          if(valIter != neValue.end())
            continue;
        }
      }

      // if the attr is val
      if(eqValue != NULL)
      {
        if(value != eqValue)
          continue;
      }

      if(attr != 4) {
        switch (attr) { 
          case 1: //SELECT key
          fprintf(stdout, "%d\n", key);
          break;
          case 2: //SELECT value
            fprintf(stdout, "%s\n", value.c_str());
            break;
          case 3:  //SELECT *
            fprintf(stdout, "%d '%s'\n", key, value.c_str());
            break;
        }
      }
    }

    if(attr == 4)
      fprintf(stdout, "%d\n", count);

    rf.close();
    return rc;
  }

  // when index does not exist, scan the table file from the beginning
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
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
// Open loadfile for reading
  ifstream lf;
  lf.open(loadfile.c_str());
  if (!lf.is_open())
  {
    fprintf(stderr, "Error: Filed opening load file");
  }

  RecordFile rf;
  if(rf.open(table + ".tbl", 'w') != 0)
  {
    fprintf(stderr, "Error: Failed accessing table\n");
    rf.close();
    lf.close();
    return RC_FILE_OPEN_FAILED;
  }

  //open index file
  BTreeIndex idx;
  if(index == true)
  {
    if(idx.open(table + ".idx", 'w') != 0)
    {
      fprintf(stderr, "Error: Failed opening index file");
      return RC_FILE_OPEN_FAILED;
    }
  }

  string line;

  while (lf.good() && getline(lf, line))
  {
    RecordId rid;
    int key;
    string value;

    parseLoadLine(line, key, value);
    rf.append(key, value,rid);

    if(index == true)
    {
      if(idx.insert(key, rid) != 0)
      {
        fprintf(stderr,"Error: Failed inserting index");
        return RC_INVALID_ATTRIBUTE;
      }
    }
  }

  rf.close();
  lf.close();
  if( index == true)
    idx.close();
  return 0;
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
