ENGINE_SRCS = $(wildcard ./private/**/*.c) $(wildcard ./*.c)
ENGINE_OBJS = $(patsubst %.c,%.o,$(ENGINE_SRCS))

CCFLAGS = -O3 -I./

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -c $< -o $@

engine: $(ENGINE_OBJS)
	mkdir -p ../bin/
	ar rcs ../bin/engine.a $(ENGINE_OBJS)

clean:
	rm -f $(ENGINE_OBJS) ../bin/engine.a