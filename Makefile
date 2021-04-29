FORCE:

engine: FORCE
	(cd engine && $(MAKE) CFLAGS="$(CFLAGS)")

client: FORCE
	(cd client && $(MAKE) CFLAGS="$(CFLAGS)")

server: 
	(cd server && $(MAKE) CFLAGS="$(CFLAGS)")

all: engine client server

clean:
	(cd engine/ && $(MAKE) clean)
	(cd client/ && $(MAKE) clean)
	(cd server/ && $(MAKE) clean)

.DEFAULT_GOAL := all