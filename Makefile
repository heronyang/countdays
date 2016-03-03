build: src/*.c
	pebble build

run: build
	pebble install -v --emulator chalk
