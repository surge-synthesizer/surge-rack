RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

SURGE_BLD=dep/surge-build
libsurge_xt_rack := $(SURGE_BLD)/libSurgeXTRack.a

OBJECTS += $(libsurge_xt_rack)

# Trigger the surge-rack CMake build when running `make dep`
DEPS += $(libsurge_xt_rack)

EXTRA_CMAKE :=
ifdef ARCH_MAC
ifdef ARCH_ARM64
    EXTRA_CMAKE += -DCMAKE_OSX_ARCHITECTURES="arm64"
else
    EXTRA_CMAKE += -DCMAKE_OSX_ARCHITECTURES="x86_64"
endif
endif

$(libsurge_xt_rack):
	$(CMAKE) -B $(SURGE_BLD) -DRACK_SDK_DIR=$(RACK_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(SURGE_BLD)/dist $(EXTRA_CMAKE)
	cmake --build $(SURGE_BLD) -- -j $(shell getconf _NPROCESSORS_ONLN)
	cmake --install $(SURGE_BLD)

# Add .cpp and .c files to the build
SOURCES += src/SurgeXT.cpp

FLAGS += -fvisibility=hidden -fvisibility-inlines-hidden

ifdef ARCH_MAC
LDFLAGS += -framework CoreFoundation -framework CoreServices
LDFLAGS += -L$(SURGE_BLD)/dist/lib/static -lsurge-common -ljuce_dsp_rack_sub -ltinyxml -lstrnatcmp -lsst-plugininfra \
           -lfmt -lsqlite -leurorack -lairwindows
endif

ifdef ARCH_WIN
LDFLAGS += -L$(SURGE_BLD)/dist/lib/static \
           -lsurge-common -ljuce_dsp_rack_sub -ltinyxml -lstrnatcmp -lsst-plugininfra \
           -lfmt -lsqlite -leurorack -lairwindows \
           -lwinmm -luuid -lwsock32 -lshlwapi -lversion -lwininet -lole32 -lws2_32
endif

ifdef ARCH_LIN
LDFLAGS += -pthread
endif

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
dist:	build/surge-data res

build/surge-data:
	mkdir -p build/surge-data
	cp surge/resources/surge-shared/windows.wt build/surge-data
	cp surge/resources/surge-shared/configuration.xml build/surge-data
	cp -R surge/resources/data/wavetables build/surge-data/wavetables
	cp -R surge/resources/data/wavetables_3rdparty build/surge-data/wavetables_3rdparty
	cp -R surge/resources/data/fx_presets build/surge-data/fx_presets
	cp -R res/surge_extra_data/fx_presets build/surge-data


DISTRIBUTABLES += $(wildcard LICENSE*) res docs patches presets README.md build/surge-data

# Include the VCV plugin Makefile framework
include $(RACK_DIR)/plugin.mk

COMMUNITY_ISSUE=https://github.com/VCVRack/community/issues/745

community:
	open $(COMMUNITY_ISSUE)

issue_blurb:	dist
	git diff --exit-code
	git diff --cached --exit-code
	@echo
	@echo "Paste this into github issue " $(COMMUNITY_ISSUE)
	@echo
	@echo "* Version: v$(VERSION)"
	@echo "* Transaction: " `git rev-parse HEAD`
	@echo "* Branch: " `git rev-parse --abbrev-ref HEAD`