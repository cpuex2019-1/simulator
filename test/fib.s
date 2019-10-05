Main:   addi $4, $0, 13  # fib(immediate)
        jal	Fibonacci
		j	Exit
Fibonacci:	addi	$29,	$29,	-8	#shelter of registers
		sw	$4,	4($29)
		sw	$31,	0($29)
		addi	$8,	$0,	1
        slt     $9, $8, $4     #if(1<n)
        beq     $9, $8 ,True   #if(1<n)
		move	$2,	$4		#n<=1
		j	Return
True:		addi	$4,	$4,	-1	#n>1
		jal	Fibonacci		#fibonacci(n-1)
		addi	$29,	$29,	-4	#shelter of return value
		sw	$2,	0($29)
		addi	$4,	$4,	-1
		jal	Fibonacci		#fibonacci(n-2)
		lw	$16,	0($29)		#restore of return value
		addi	$29,	$29,	4
		add	$2,	$2,	$16	#fibonacci(n-1) + fibonacci(n-2)
		j	Return
Return:		lw	$31,	0($29)		#shelter of registers
		lw	$4,	4($29)
		addi	$29,	$29,	8
		jr	$31
Exit:
