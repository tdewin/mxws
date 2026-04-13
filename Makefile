build:
	cc -lncurses -o mxws mxws.c
test:
	ifconfig | ./mxws --word 192
