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
OBJECTS := build/main.o build/engine.o build/graphics.o build/audio.o
LIBS := -L$(DEVKITPRO)/libctru/lib -lcitro2d -lcitro3d -lctru -lm

.PHONY: all check-toolchain verify-arm11 host test prepare-graphics test-graphics prepare-audio test-audio clean
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
build/graphics-export: tools/graphics_export.cpp source/engine.cpp tests/gameplay.h tests/battle.h tests/gas.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/graphics_export.cpp source/engine.cpp -o $@
ASSET_ROOT ?= ../data files
MAP ?= local-data/3ds/openbw/maps/probe.scx
prepare-graphics: build/graphics-export
	mkdir -p local-data/graphics-work
	./build/graphics-export "$(ASSET_ROOT)" "$(MAP)" local-data/graphics-work/raw.bin
	python3 tools/prepare_graphics.py local-data/graphics-work/raw.bin local-data/3ds/openbw/graphics.bin
test-graphics: prepare-graphics
	python3 tests/graphics.py local-data/graphics-work/raw.bin local-data/3ds/openbw/graphics.bin
build/host-sim: source/engine.cpp source/host_main.cpp tests/gameplay.h tests/battle.h tests/gas.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) source/engine.cpp source/host_main.cpp -o $@
build/scheduler-test: tests/scheduler.cpp include/scheduler.h | build
	$(HOST_CXX) -std=c++14 -Iinclude tests/scheduler.cpp -o $@
build/controls-test: tests/controls.cpp include/controls.h | build
	$(HOST_CXX) -std=c++14 -Iinclude tests/controls.cpp -o $@
build/input-test: tests/input.cpp include/input.h | build
	$(HOST_CXX) -std=c++14 -Iinclude tests/input.cpp -o $@
test: build/host-sim build/scheduler-test build/controls-test build/input-test
	./build/scheduler-test
	./build/controls-test
	./build/input-test
	./build/host-sim --self-test
	python3 tests/preflight.py
clean:
	rm -rf build out
-include build/main.d build/engine.d build/graphics.d build/audio.d

build/audio-export: tools/audio_export.cpp source/engine.cpp tests/gameplay.h tests/battle.h tests/gas.h $(HEADERS) | build
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
