compile_blocking:
	gcc blocking_2019022.c -lpthread

run_blocking: compile_blocking
	./a.out

compile_non_blocking:
	gcc nonblocking_2019022.c -lpthread

run_non_blocking: compile_non_blocking
	./a.out

clear:
	rm a.out