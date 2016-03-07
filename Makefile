PHONE_IP=192.168.1.221

build: src/*.c src/js/*.js
	pebble build

run_phone: build
	pebble install -v --phone $(PHONE_IP)

logs_phone:
	pebble logs --phone $(PHONE_IP)

run: build
	pebble install -v --emulator chalk

logs:
	pebble logs -v --emulator chalk

preivew:
	pebble emu-app-config
