# simulator (mipsim)
## version 9.4
### how to use
Make sure you can use `g++` and `c++11`, then, compile the programs:
```
$ make
```
Use mipsim
```
$ ./mipsim test/test.s
```

Type commands to execute assembler.
```
How to use mipsim:
	   s | step | \n	: run step by step
	   e | execute 	: execute instructions for designated times
	   a | all	: run all
	   r | reg	: print register
	   m | memo	: print memory from start to end
	   p | program	: print program and line number
	   b | break	: set breakpoint
	   l | log	: change log level
	   restart	: restart from first program address
	   exit	: exit program
```
