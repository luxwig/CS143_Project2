SRC = main.cc SqlParser.tab.c lex.sql.c SqlEngine.cc BTreeIndex.cc BTreeNode.cc RecordFile.cc PageFile.cc 
HDR = Bruinbase.h PageFile.h SqlEngine.h BTreeIndex.h BTreeNode.h RecordFile.h SqlParser.tab.h
SRCTEST = BTreeNode.cc BTreeIndex.cc RecordFile.cc PageFile.cc

bruinbase: $(SRC) $(HDR)
	g++ -ggdb -o $@ $(SRC)

lex.sql.c: SqlParser.l
	flex -Psql $<

SqlParser.tab.c: SqlParser.y
	bison -d -psql $<

testnode: $(SRC) $(HDR) testnode.cc
	g++ -ggdb -o test $(SRCTEST) testnode.cc

testindex: $(SRC) $(HDR) testindex.cc
	g++ -ggdb -o test $(SRCTEST) testindex.cc
clean:
	rm -f bruinbase bruinbase.exe *.o *~ lex.sql.c SqlParser.tab.c SqlParser.tab.h
	rm -f test test.exe
	rm -f *.data	
