# Run from this directory. Relative source/output paths also work in workspaces
# whose parent path contains spaces (the standard recursive template does not).
.DEFAULT_GOAL := all
DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
HOST_CXX ?= c++
ARM_CXX := $(DEVKITARM)/bin/arm-none-eabi-g++
export PATH := $(DEVKITPRO)/tools/bin:$(DEVKITARM)/bin:$(PATH)
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
COMMON := -std=gnu++14 -O2 -g -fno-strict-aliasing -ffunction-sections -fdata-sections -Iinclude -I..
# OpenBW uses both typeid and exceptions. Do not copy the SDK template's
# -fno-rtti/-fno-exceptions options into this build.
ARM_FLAGS := $(COMMON) $(ARCH) -mword-relocations -D__3DS__ -I$(DEVKITPRO)/libctru/include
HEADERS := $(wildcard include/*.h ../*.h)
OBJECTS := build/main.o build/engine.o build/graphics.o build/audio.o build/csv_log.o
LIBS := -L$(DEVKITPRO)/libctru/lib -lcitro2d -lcitro3d -lctru -lm

.PHONY: all check-toolchain verify-arm11 host test prepare-graphics test-graphics prepare-audio test-audio test-csv test-sprite-cache test-mpq-audio prepare-arena prepare-arena-graphics clean
all: out/openbw-3ds.3dsx verify-arm11
verify-arm11: out/openbw-3ds.elf
	python3 tools/verify_arm11.py "$(DEVKITARM)/bin/arm-none-eabi-readelf" $<
check-toolchain:
	@test -x "$(ARM_CXX)" || { echo "devkitARM missing: set DEVKITPRO (see ../PORTING_3DS.md)"; exit 1; }
	@test -f "$(DEVKITPRO)/libctru/lib/libcitro2d.a" || { echo "Install 3ds-dev (libctru/citro2d/citro3d)"; exit 1; }
	@command -v 3dsxtool >/dev/null || { echo "3dsxtool missing from DEVKITPRO/tools/bin"; exit 1; }
build out:
	mkdir -p $@
build/%.o: source/%.cpp $(HEADERS) | build check-toolchain
	"$(ARM_CXX)" $(ARM_FLAGS) -MMD -MP -c $< -o $@
out/openbw-3ds.elf: $(OBJECTS) arm11.specs Makefile | out
	"$(ARM_CXX)" $(ARCH) -specs=arm11.specs -g -Wl,--gc-sections,-Map,out/openbw-3ds.map $(OBJECTS) $(LIBS) -o $@
	"$(DEVKITARM)/bin/arm-none-eabi-size" $@
out/openbw-3ds.3dsx: out/openbw-3ds.elf
	"$(DEVKITPRO)/tools/bin/3dsxtool" $< $@
host: build/host-sim
build/graphics-export: tools/graphics_export.cpp tools/asset_closure.h source/engine.cpp tests/gameplay.h tests/battle.h tests/gas.h tests/terran.h tests/terran_build.h tests/protoss.h tests/protoss_build.h tests/zerg.h tests/zerg_build.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/graphics_export.cpp source/engine.cpp -o $@
ASSET_ROOT ?= ../data files
MAP ?= local-data/3ds/openbw/maps/probe.scx
prepare-graphics: build/graphics-export
	mkdir -p local-data/graphics-work
	./build/graphics-export "$(ASSET_ROOT)" "$(MAP)" local-data/graphics-work/raw.bin
	python3 tools/prepare_graphics.py local-data/graphics-work/raw.bin local-data/3ds/openbw/graphics.bin
test-graphics: prepare-graphics
	python3 tests/graphics.py local-data/graphics-work/raw.bin local-data/3ds/openbw/graphics.bin
build/host-sim: source/engine.cpp source/host_main.cpp tests/gameplay.h tests/battle.h tests/gas.h tests/terran.h tests/terran_build.h tests/protoss.h tests/protoss_build.h tests/zerg.h tests/zerg_build.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) source/engine.cpp source/host_main.cpp -o $@
build/scheduler-test: tests/scheduler.cpp include/scheduler.h | build
	$(HOST_CXX) -std=c++14 -Iinclude tests/scheduler.cpp -o $@
build/controls-test: tests/controls.cpp include/controls.h | build
	$(HOST_CXX) -std=c++14 -Iinclude tests/controls.cpp -o $@
build/input-test: tests/input.cpp include/input.h | build
	$(HOST_CXX) -std=c++14 -Iinclude tests/input.cpp -o $@
test: build/host-sim build/scheduler-test build/controls-test build/input-test build/frame-pacer-test
	./build/frame-pacer-test
	./build/scheduler-test
	./build/controls-test
	./build/input-test
	./build/host-sim --self-test
	python3 tests/preflight.py
clean:
	rm -rf build out
-include build/main.d build/engine.d build/graphics.d build/audio.d build/csv_log.d

build/audio-export: tools/audio_export.cpp tools/asset_closure.h source/engine.cpp tests/gameplay.h tests/battle.h tests/gas.h tests/terran.h tests/terran_build.h tests/protoss.h tests/protoss_build.h tests/zerg.h tests/zerg_build.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/audio_export.cpp source/engine.cpp -o $@
prepare-audio: build/audio-export
	mkdir -p local-data/audio-work
	./build/audio-export "$(ASSET_ROOT)" "$(MAP)" local-data/audio-work
	python3 tools/prepare_audio.py local-data/audio-work local-data/3ds/openbw/audio
build/audio-test: tests/audio.cpp tests/fake3ds/3ds.h source/audio.cpp include/audio.h | build
	$(HOST_CXX) -std=c++14 -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds -Iinclude tests/audio.cpp source/audio.cpp -o $@
test-audio: build/audio-test
	ASAN_OPTIONS=detect_leaks=0 ./build/audio-test
	python3 tests/audio_assets.py local-data/audio-work local-data/3ds/openbw/audio
build/arena-export: tools/arena_export.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/arena_export.cpp -o $@
prepare-arena: build/arena-export
	./build/arena-export "$(ASSET_ROOT)" "$(MAP)" local-data/3ds/openbw/maps/arena.chk
build/csv-test: tests/csv_log.cpp source/csv_log.cpp include/csv_log.h tests/fake3ds/3ds.h | build
	$(HOST_CXX) -std=c++14 -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds -Iinclude tests/csv_log.cpp source/csv_log.cpp -o $@
test-csv: build/csv-test
	ASAN_OPTIONS=detect_leaks=0 ./build/csv-test
build/sprite-cache-test: tests/sprite_cache.cpp tests/composite.h tests/fake3ds/citro2d.h source/graphics.cpp source/engine.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer -Itests/fake3ds tests/sprite_cache.cpp source/graphics.cpp source/engine.cpp -o $@
test-sprite-cache: build/sprite-cache-test
	ASAN_OPTIONS=detect_leaks=0 ./build/sprite-cache-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/3ds/openbw/graphics-arena.bin local-data/graphics-work/raw.bin

build/mpq-audio-test: tests/mpq_audio.cpp ../data_loading.h | build
	$(HOST_CXX) -std=c++14 -O1 -g -fsanitize=address -fno-omit-frame-pointer -I.. tests/mpq_audio.cpp -o $@
test-mpq-audio: build/mpq-audio-test
	ASAN_OPTIONS=detect_leaks=0 ./build/mpq-audio-test

prepare-arena-graphics: prepare-arena build/graphics-export
	mkdir -p local-data/graphics-work
	./build/graphics-export "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/graphics-work/arena-raw.bin
	python3 tools/prepare_graphics.py local-data/graphics-work/arena-raw.bin local-data/3ds/openbw/graphics-arena.bin
	python3 tests/graphics.py local-data/graphics-work/arena-raw.bin local-data/3ds/openbw/graphics-arena.bin

prepare-protoss-audio: build/audio-export
	mkdir -p local-data/audio-work-protoss
	./build/audio-export "$(ASSET_ROOT)" "$(MAP)" local-data/audio-work-protoss protoss
	python3 tools/prepare_audio.py local-data/audio-work-protoss local-data/3ds/openbw/audio/protoss --race protoss
	python3 tests/audio_assets.py local-data/audio-work-protoss local-data/3ds/openbw/audio/protoss protoss

prepare-protoss-graphics: build/graphics-export
	mkdir -p local-data/graphics-work
	./build/graphics-export "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/graphics-work/protoss-raw.bin protoss
test-protoss-cache: build/sprite-cache-test
	ASAN_OPTIONS=detect_leaks=0 ./build/sprite-cache-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/3ds/openbw/graphics-arena.bin local-data/graphics-work/protoss-raw.bin protoss

prepare-zerg-audio: build/audio-export
	mkdir -p local-data/audio-work-zerg
	./build/audio-export "$(ASSET_ROOT)" "$(MAP)" local-data/audio-work-zerg zerg
	python3 tools/prepare_audio.py local-data/audio-work-zerg local-data/3ds/openbw/audio/zerg --race zerg
	python3 tests/audio_assets.py local-data/audio-work-zerg local-data/3ds/openbw/audio/zerg zerg
prepare-zerg-graphics: build/graphics-export
	mkdir -p local-data/graphics-work
	./build/graphics-export "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/graphics-work/zerg-raw.bin zerg
test-zerg-cache: build/sprite-cache-test
	ASAN_OPTIONS=detect_leaks=0 ./build/sprite-cache-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/3ds/openbw/graphics-arena.bin local-data/graphics-work/zerg-raw.bin zerg

build/reload-test: tests/reload.cpp source/engine.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/reload.cpp source/engine.cpp -o $@
test-reload: build/reload-test
	ASAN_OPTIONS=detect_leaks=0 ./build/reload-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/3ds/openbw/maps/probe.scx

build/audio-memory-test: tests/audio_memory.cpp tests/fake3ds/3ds.h source/audio.cpp include/audio.h | build
	$(HOST_CXX) -std=c++14 -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds -Iinclude tests/audio_memory.cpp source/audio.cpp -o $@
.PHONY: test-audio-memory
test-audio-memory: build/audio-memory-test
	ASAN_OPTIONS=detect_leaks=0 ./build/audio-memory-test

build/frame-pacer-test: tests/frame_pacer.cpp include/frame_pacer.h | build
	$(HOST_CXX) -std=c++14 -Iinclude tests/frame_pacer.cpp -o $@

build/protoss-economy-test: tests/protoss_economy.cpp tests/battle.h tests/fake3ds/citro2d.h source/engine.cpp source/graphics.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds tests/protoss_economy.cpp source/engine.cpp source/graphics.cpp -o $@
.PHONY: test-protoss-economy
test-protoss-economy: build/protoss-economy-test
	ASAN_OPTIONS=detect_leaks=0 ./build/protoss-economy-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/probe.scx local-data/3ds/openbw/graphics.bin local-data/3ds/openbw/audio/protoss/effects.bin

build/cursor-test: tests/cursors.cpp tests/fake3ds/citro2d.h source/engine.cpp source/graphics.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds tests/cursors.cpp source/engine.cpp source/graphics.cpp -o $@
.PHONY: test-cursors
test-cursors: build/cursor-test
	ASAN_OPTIONS=detect_leaks=0 ./build/cursor-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/3ds/openbw/graphics-arena.bin

build/fog-test: tests/fog.cpp tests/fake3ds/citro2d.h source/engine.cpp source/graphics.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds tests/fog.cpp source/engine.cpp source/graphics.cpp -o $@
.PHONY: test-fog
test-fog: build/fog-test
	ASAN_OPTIONS=detect_leaks=0 ./build/fog-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/probe.scx local-data/3ds/openbw/graphics.bin

.PHONY: test-zerg-economy
test-zerg-economy: build/protoss-economy-test
	ASAN_OPTIONS=detect_leaks=0 ./build/protoss-economy-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/probe.scx local-data/3ds/openbw/graphics.bin local-data/3ds/openbw/audio/zerg/effects.bin zerg

build/fog-memory-test: tests/fog_memory.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/fog_memory.cpp -o $@
.PHONY: test-fog-memory
test-fog-memory: build/fog-memory-test
	ASAN_OPTIONS=detect_leaks=0 ./build/fog-memory-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/probe.scx
