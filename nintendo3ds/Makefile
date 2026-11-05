# Run from this directory. Relative source/output paths also work in workspaces
# whose parent path contains spaces (the standard recursive template does not).
.DEFAULT_GOAL := all
DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
HOST_CXX ?= c++
ARM_CXX := $(DEVKITARM)/bin/arm-none-eabi-g++
export PATH := $(DEVKITPRO)/tools/bin:$(DEVKITARM)/bin:$(PATH)
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
COMMON := -std=gnu++14 -O2 -g -fno-strict-aliasing -ffunction-sections -fdata-sections -Iinclude -I.. -Ibuild
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
build/graphics-export: tools/graphics_export.cpp tools/asset_closure.h source/engine.cpp tests/gameplay.h tests/battle.h tests/gas.h tests/terran.h tests/terran_build.h tests/protoss.h tests/protoss_build.h tests/zerg.h tests/zerg_build.h $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) tools/graphics_export.cpp source/engine.cpp -o $@
ASSET_ROOT ?= ../data files
MAP ?= local-data/3ds/openbw/maps/probe.scx
prepare-graphics: build/graphics-export
	mkdir -p local-data/graphics-work
	./build/graphics-export "$(ASSET_ROOT)" "$(MAP)" local-data/graphics-work/raw.bin
	python3 tools/prepare_graphics.py local-data/graphics-work/raw.bin local-data/3ds/openbw/graphics.bin
test-graphics: prepare-graphics
	python3 tests/graphics.py local-data/graphics-work/raw.bin local-data/3ds/openbw/graphics.bin
build/host-sim: source/engine.cpp source/host_main.cpp tests/gameplay.h tests/battle.h tests/gas.h tests/terran.h tests/terran_build.h tests/protoss.h tests/protoss_build.h tests/zerg.h tests/zerg_build.h $(HEADERS) build/source_id.h | build
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

build/audio-export: tools/audio_export.cpp tools/asset_closure.h source/engine.cpp tests/gameplay.h tests/battle.h tests/gas.h tests/terran.h tests/terran_build.h tests/protoss.h tests/protoss_build.h tests/zerg.h tests/zerg_build.h $(HEADERS) build/source_id.h | build
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
build/sprite-cache-test: tests/sprite_cache.cpp tests/composite.h tests/fake3ds/citro2d.h source/graphics.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
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

build/reload-test: tests/reload.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
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

build/protoss-economy-test: tests/protoss_economy.cpp tests/battle.h tests/fake3ds/citro2d.h source/engine.cpp source/graphics.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds tests/protoss_economy.cpp source/engine.cpp source/graphics.cpp -o $@
.PHONY: test-protoss-economy
test-protoss-economy: build/protoss-economy-test
	ASAN_OPTIONS=detect_leaks=0 ./build/protoss-economy-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/probe.scx local-data/3ds/openbw/graphics.bin local-data/3ds/openbw/audio/protoss/effects.bin

build/cursor-test: tests/cursors.cpp tests/fake3ds/citro2d.h source/engine.cpp source/graphics.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds tests/cursors.cpp source/engine.cpp source/graphics.cpp -o $@
.PHONY: test-cursors
test-cursors: build/cursor-test
	ASAN_OPTIONS=detect_leaks=0 ./build/cursor-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/3ds/openbw/graphics-arena.bin

build/fog-test: tests/fog.cpp tests/fake3ds/citro2d.h source/engine.cpp source/graphics.cpp $(HEADERS) build/source_id.h | build
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

build/ui-export: tools/ui_export.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/ui_export.cpp -o $@
local-data/ui-work/reference.bin: build/ui-export tools/prepare_ui_reference.py
	mkdir -p local-data/ui-work
	./build/ui-export "$(ASSET_ROOT)" local-data/ui-work
	python3 tools/prepare_ui_reference.py local-data/ui-work
build/command-grid-test: tests/command_grid.cpp include/command_grid.h include/input.h | build
	$(HOST_CXX) -std=c++14 -Iinclude tests/command_grid.cpp -o $@
.PHONY: test-command-grid
test-command-grid: build/command-grid-test
	./build/command-grid-test

build/command-icons-test: tests/command_icons.cpp tests/fake3ds/citro2d.h source/engine.cpp source/graphics.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds tests/command_icons.cpp source/engine.cpp source/graphics.cpp -o $@
.PHONY: test-command-icons
test-command-icons: build/command-icons-test local-data/ui-work/reference.bin
	ASAN_OPTIONS=detect_leaks=0 ./build/command-icons-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk local-data/3ds/openbw/graphics-arena.bin local-data/ui-work/reference.bin

# Executable/save identity tracks source inputs; game assets stay private.
build/source_id.h: $(HEADERS) $(wildcard source/*.cpp) tools/build_info.py Makefile arm11.specs | build
	python3 tools/build_info.py $@
build/main.o build/engine.o: build/source_id.h

build/frontend-export: tools/frontend_export.cpp tools/original_archive.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/frontend_export.cpp -o $@
build/frontend-test: tests/frontend.cpp include/frontend.h include/pause_menu.h | build
	$(HOST_CXX) -std=c++14 -O1 -g -fsanitize=address -Iinclude tests/frontend.cpp -o $@
.PHONY: test-frontend
test-frontend: build/frontend-test
	./build/frontend-test

build/custom-match-test: tests/custom_match.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/custom_match.cpp source/engine.cpp -o $@
.PHONY: test-custom-match
test-custom-match: build/custom-match-test
	ASAN_OPTIONS=detect_leaks=0 ./build/custom-match-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/probe.scx

build/selection-info-test: tests/selection_info.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/selection_info.cpp source/engine.cpp -o $@
.PHONY: test-selection-info
test-selection-info: build/selection-info-test
	ASAN_OPTIONS=detect_leaks=0 ./build/selection-info-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/arena.chk

build/melee-test: tests/melee.cpp source/engine.cpp source/graphics.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer -Itests/fake3ds tests/melee.cpp source/engine.cpp source/graphics.cpp -o $@
.PHONY: test-melee
test-melee: build/melee-test
	ASAN_OPTIONS=detect_leaks=0 ./build/melee-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/probe.scx

build/menu-animation-test: tests/menu_animation.cpp include/menu_animation.h tests/fake3ds/citro2d.h | build
	$(HOST_CXX) -std=c++14 -O1 -g -fsanitize=address -Itests/fake3ds -Iinclude tests/menu_animation.cpp -o $@
.PHONY: test-menu-animation
test-menu-animation: build/menu-animation-test
	ASAN_OPTIONS=detect_leaks=0 ./build/menu-animation-test local-data/3ds/openbw/ui/animations.bin

build/maps-export: tools/maps_export.cpp tools/original_archive.h source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) tools/maps_export.cpp source/engine.cpp -o $@

build/map-catalogue-test: tests/map_catalogue.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/map_catalogue.cpp source/engine.cpp -o $@

build/audio-stream-test: tests/audio_stream.cpp source/audio.cpp include/audio.h tests/fake3ds/3ds.h | build
	$(HOST_CXX) -std=c++14 -O1 -g -fsanitize=address -fno-omit-frame-pointer -pthread -Itests/fake3ds -Iinclude tests/audio_stream.cpp source/audio.cpp -o $@

.PHONY: prepare-all-audio
prepare-all-audio:
	python3 tools/merge_audio.py local-data/3ds/openbw/audio

build/ai-tactics-test: tests/ai_tactics.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/ai_tactics.cpp -o $@
.PHONY: test-ai-tactics
test-ai-tactics: build/ai-tactics-test
	ASAN_OPTIONS=detect_leaks=0 ./build/ai-tactics-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/probe.scx local-data/3ds/openbw/maps/AI/islands.scm

build/observer-test: tests/observer.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/observer.cpp source/engine.cpp -o $@
.PHONY: test-observer
test-observer: build/observer-test
	ASAN_OPTIONS=detect_leaks=0 ./build/observer-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps

build/observer-maps: tools/observer_maps.cpp tools/original_archive.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/observer_maps.cpp -o $@

build/savegame-test: tests/savegame.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/savegame.cpp source/engine.cpp -o $@
.PHONY: test-savegame
test-savegame: build/savegame-test
	ASAN_OPTIONS=detect_leaks=0 ./build/savegame-test "$(ASSET_ROOT)" local-data/3ds/openbw/maps/AI/three-islands.scm local-data/3ds/openbw/maps/arena.chk

build/campaign-export: tools/campaign_export.cpp tools/original_archive.h include/campaign_map.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/campaign_export.cpp -o $@
build/campaign-test: tests/campaign.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/campaign.cpp source/engine.cpp -o $@
.PHONY: test-campaign
test-campaign: build/campaign-test
	ASAN_OPTIONS=detect_leaks=0 ./build/campaign-test "$(ASSET_ROOT)" local-data/3ds/openbw/campaign/terran01.chk

build/campaign-audio-export: tools/campaign_audio_export.cpp tools/original_archive.h include/sound_ids.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/campaign_audio_export.cpp -o $@
build/portrait-export: tools/portrait_export.cpp tools/original_archive.h $(HEADERS) | build
	$(HOST_CXX) $(COMMON) tools/portrait_export.cpp -o $@

# New HUD behavior uses native units and journal reconstruction, not UI mocks.
build/hud-controls-test: tests/hud_controls.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/hud_controls.cpp source/engine.cpp -o $@
.PHONY: test-hud-controls
test-hud-controls: build/hud-controls-test
	ASAN_OPTIONS=detect_leaks=0 ./build/hud-controls-test local-data/3ds/openbw local-data/3ds/openbw/maps/arena.chk /tmp/sc3ds-hud-controls.sav

build/campaign-openings-test: tests/campaign_openings.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/campaign_openings.cpp source/engine.cpp -o $@

build/campaign-audio-test: tests/campaign_audio.cpp source/audio.cpp include/audio.h tests/fake3ds/3ds.h | build
	$(HOST_CXX) $(COMMON) -Itests/fake3ds -O1 -g -fsanitize=address -pthread tests/campaign_audio.cpp source/audio.cpp -o $@

build/campaign-playthrough-test: tests/campaign_playthrough.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/campaign_playthrough.cpp source/engine.cpp -o $@

build/campaign-triggers-test: tests/campaign_triggers.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/campaign_triggers.cpp -o $@

build/campaign-audio-banks-test: tests/campaign_audio_banks.cpp source/audio.cpp include/audio.h include/campaign_catalogue.h tests/fake3ds/3ds.h | build
	$(HOST_CXX) $(COMMON) -Itests/fake3ds -O1 -g -fsanitize=address -pthread tests/campaign_audio_banks.cpp source/audio.cpp -o $@
build/campaign-bw-routes-test: tests/campaign_bw_routes.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/campaign_bw_routes.cpp source/engine.cpp -o $@

build/campaign-initial-flags-test: tests/campaign_initial_flags.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/campaign_initial_flags.cpp -o $@
build/campaign-initial-links-test: tests/campaign_initial_links.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/campaign_initial_links.cpp -o $@
build/campaign-bw-terran-test: tests/campaign_bw_terran.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address -fno-omit-frame-pointer tests/campaign_bw_terran.cpp source/engine.cpp -o $@

# Original mission completions and focused campaign semantics regressions.
build/campaign-beacon-bring-test: tests/campaign_beacon_bring.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address tests/campaign_beacon_bring.cpp -o $@
build/campaign-targeted-spells-test: tests/campaign_targeted_spells.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address tests/campaign_targeted_spells.cpp -o $@
build/campaign-bw-terran-second-test: tests/campaign_bw_terran_second.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address tests/campaign_bw_terran_second.cpp source/engine.cpp -o $@
build/campaign-bw-terran-third-test: tests/campaign_bw_terran_third.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address tests/campaign_bw_terran_third.cpp source/engine.cpp -o $@
build/campaign-bw-protoss-third-test: tests/campaign_bw_protoss_third.cpp source/engine.cpp $(HEADERS) build/source_id.h | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address tests/campaign_bw_protoss_third.cpp source/engine.cpp -o $@

build/campaign-rescue-triggers-test: tests/campaign_rescue_triggers.cpp $(HEADERS) | build
	$(HOST_CXX) $(COMMON) -O1 -g -fsanitize=address tests/campaign_rescue_triggers.cpp -o $@
