# simulator for 3rd (mipsim)
## version 1.0
### how to use
Make sure you can use `g++` and `c++11`, then, compile the programs:

```
$ make
```


Use mipsim

```
$ ./mipsim test/test.s
```


Type commands to execute assembly.

```
How to use mipsim:
	   s | step | \n	: run step by step
	   e | execute 	: execute instructions for designated times
	   a | all	: run all
	   r | reg	: print register
	   m | memo	: print memory from start to end
	   c | check: check memory (int or float)
	   p | program	: print program and line number
	   b | break	: set breakpoint
	   l | log	: change log level
	   restart	: restart from first program address
	   exit	: exit program
```

The output file is `test/test.ppm`. The statistic infomation file is `test/est.statistic.out`.


Use `mipsim-all`, if you want to run assembly faster.


```
$ ./mipsim-all test/test.s
```

This command does not produce a file for statistic.
