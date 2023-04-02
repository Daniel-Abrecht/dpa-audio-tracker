
LDLIBS += -lm
CFLAGS += -std=c11 -Wall -Wextra -pedantic
CFLAGS += -Iinclude
CFLAGS += -O0 -g

volume ?= 16
export volume

all: bin/main bin/midi2trk

bin/main: src/main.c
	mkdir -p bin
	$(CC) -o $@ $(CFLAGS) $^ $(LDLIBS)

bin/midi2trk: src/midi.c src/midi2trk.c src/ringbuffer.c
	mkdir -p bin
	$(CC) -o $@ $(CFLAGS) $^ $(LDLIBS)

.SECONDARY:
.ONESHELL:

%.wav: %.trk bin/main
	./bin/main <"$<" >"$@"

play: play//input

play//%: %.wav
	set -ex
	factor="$$(echo "$$(getfattr -n user.stats.factor --only-value "$<")" \* 0.7 \* $$volume / 100 | bc)"
	sox -v "$$factor" "$<" -t wav - | aplay -

clean:
	rm -f bin/main bin/midi2trk
