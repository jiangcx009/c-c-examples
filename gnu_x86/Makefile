CFLAGS= -c -g
LDFLAGS= -lpthread -levent
# LDFLAGS= -lpthread 

OBJS +=	test-ctrl-c	 		
OBJS += test-snprintf 		
OBJS += test-extern-c 		
OBJS += test-strtok 		
OBJS += test-null 			
OBJS += test-thread-join 	
OBJS += test-syslog 		
OBJS += test-local-ip 		
OBJS += test-listen-sock 	
OBJS += cmdparse 			
OBJS += bin2hex 			
OBJS += reverse4bin 		
OBJS += game 				
OBJS += char2point
OBJS += test-libevent-timer 
OBJS += union_test
OBJS += struct_test
OBJS += OOP_test
#OBJS += verify				
#OBJS += test-r 				

all:$(OBJS)
.PHONY: all

%.o:%.c
	cc $(CFLAGS) $< -o $@

%.o:%.cc
	g++ $(CFLAGS) $< -o $@

char2point: char2point.o
	cc -o char2point char2point.o
######################################following is zwb############################33
test-ctrl-c: test-ctrl-c.o
	cc -o test-ctrl-c test-ctrl-c.o

test-snprintf: test-snprintf.o
	cc -o test-snprintf test-snprintf.o

test-extern-c: test-extern-c.o test-extern-cc.o
	g++ $^ -o $@

test-strtok: test-strtok.o
	cc $^ -o $@

test-null: test-null.o
	cc $^ -o $@

test-thread-join: test-thread-join.o
	cc $^ -o $@ $(LDFLAGS)

test-syslog: test-syslog.o
	cc $^ -o $@ $(LDFLAGS)

test-libevent-timer: test-libevent-timer.o
	cc $^ -o $@ $(LDFLAGS)

test-local-ip: test-local-ip.o
	cc $^ -o $@ $(LDFLAGS)

test-listen-sock: test-listen-sock.o
	cc $^ -o $@ $(LDFLAGS)
	
#test-r: test-r.o
#	cc $^ -o $@ $(LDFLAGS)

cmdparse:cmd_parse.c
	cc $^ -o $@ $(LDFLAGS) -g

bin2hex:bin2hex.c
	cc $^ -o $@ $(LDFLAGS) -g

#verify:verify.c
#	cc $^ -o $@ $(LDFLAGS) -g

reverse4bin: reverse4bin.c
	cc $^ -o $@ $(LDFLAGS) -g

game: game.cc
	g++ $^ -o $@ $(LDFLAGS) -g

union_test: union_test.c
	cc $^ -o $@ $(LDFLAGS) -g

struct_test: struct_test.c
	cc $^ -o $@ $(LDFLAGS) -g

OOP_test: OOP_test.cc
	g++ $^ -o $@ $(LDFLAGS) -g
clean:
	rm *.o 
	rm $(OBJS)


