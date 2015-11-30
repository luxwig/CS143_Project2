Bruinbase project

- We optimized page number reading by avoiding opening the table if we can get the result by just reading the index file. 

- Sichen Zhao luxwig@ucla.edu
  Ke Liao lilyliao48@gmail.com

- We made some test files like testnode.cc testindex.cc testreadPage.cc that can help see results of different parts. 
By running make testnode or  make testindex, we can test each step for inserting and reading the tree.


