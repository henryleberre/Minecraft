CLIENT_SRCS = $(wildcard ./**/*.c) $(wildcard ./*.c)
CLIENT_OBJS = $(patsubst %.c,%.o,$(CLIENT_SRCS))

CCFLAGS = -O3 -I../

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -c $< -o $@

client: $(CLIENT_OBJS)
	mkdir -p ../bin/
	$(CC) $(CLIENT_OBJS) ../bin/engine.a -o ../bin/client

clean:
	rm -f $(CLIENT_OBJS) ../bin/client