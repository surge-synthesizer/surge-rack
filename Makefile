RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

SURGE_BLD=dep/surge-build
libsurge_rack := $(SURGE_BLD)/libsurge-rack.a

OBJECTS += $(libsurge_rack)

# Trigger the static library to be built when running `make dep`
DEPS += $(libsurge_rack)

EXTRA_CMAKE :=
ifdef ARCH_MAC
ifdef ARCH_ARM64
    EXTRA_CMAKE += -DCMAKE_OSX_ARCHITECTURES="arm64"
else
    EXTRA_CMAKE += -DCMAKE_OSX_ARCHITECTURES="x86_64"
endif
endif

$(libsurge_rack):
	$(CMAKE) -B $(SURGE_BLD) -DRACK_SDK_DIR=$(RACK_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(SURGE_BLD)/dist $(EXTRA_CMAKE)
	cmake --build $(SURGE_BLD) -- -j $(shell getconf _NPROCESSORS_ONLN)
	cmake --install $(SURGE_BLD)

# to understand that -include limits, btw: Surge 1.7 doesn't include it but uses numeric_limits. The windows
# toolchain rack uses requires the install (the surge toolchain implicitly includes it). Rather than patch
# surge 1.7.1 for an include, just slap it in the code here for now. See #307


FLAGS += $(RACK_FLAG) -DTIXML_USE_STL=1

CFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Comment this out for sanitizer then run with
# DYLD_INSERT_LIBRARIES=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/14.0.0/lib/darwin/libclang_rt.asan_osx_dynamic.dylib /Applications/VCV\ Rack\ 2\ Pro.app/Contents/MacOS/Rack
#FLAGS += -fsanitize=address -fsanitize=undefined
#LDFLAGS += -fsanitize=address -fsanitize=undefined

# Add .cpp and .c files to the build
SOURCES += src/SurgeXT.cpp

# ASM ERRORS need fixing


ifdef ARCH_MAC
FLAGS += -std=c++17 -fvisibility=hidden -fvisibility-inlines-hidden
LDFLAGS += -framework CoreFoundation -framework CoreServices
endif

ifdef ARCH_WIN
FLAGS += -std=c++17 -fvisibility=hidden -fvisibility-inlines-hidden
LDFLAGS += -L$(SURGE_BLD)/dist/lib/static \
           -lsurge-common -ljuce_dsp_rack_sub -ltinyxml -lstrnatcmp -lsst-plugininfra \
           -lfmt -lsqlite -loddsound-mts -leurorack -lairwindows \
           -lwinmm -luuid -lwsock32 -lshlwapi -lversion -lwininet -lole32 -lws2_32
endif

ifdef ARCH_LIN
FLAGS += -std=c++17 -fvisibility=hidden -fvisibility-inlines-hidden -Wno-unused-value -Wno-suggest-override -Wno-implicit-fallthrough -Wno-ignored-qualifiers
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

# Adjust CMAKE to work in the RACK build environment,
# per @vortico and @cschol
ifdef ARCH_WIN
    CMAKE += -DCMAKE_SYSTEM_NAME=Windows
endif
ifdef MSYSTEM
    CMAKE += -G "MSYS Makefiles"
endif
ifdef ARCH_MAC
    CMAKE += -DCMAKE_SYSTEM_NAME=Darwin # -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9
endif
ifdef ARCH_LIN
    CMAKE += -DCMAKE_TOOLCHAIN_FILE=
    CMAKE += -DCMAKE_SYSTEM_NAME=Linux
endif

# Add Surge Specific make flags based on architecture
ifdef ARCH_MAC
# Obvioulsy get rid of this one day
	FLAGS += 	-Wno-undefined-bool-conversion \
	-Wno-unused-variable \
	-Wno-reorder \
	-Wno-char-subscripts \
	-Wno-sign-compare \
	-Wno-ignored-qualifiers \
	-Wno-c++17-extensions \
	-Wno-unused-private-field
	FLAGS += -DMAC
endif

ifdef ARCH_LIN
	FLAGS += -DLINUX
	FLAGS += -Wno-nonnull-compare \
	-Wno-sign-compare \
	-Wno-char-subscripts \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable \
	-Wno-reorder \
	-Wno-multichar

	FLAGS += -Isurge/src/linux
endif

ifdef ARCH_WIN
	FLAGS += -Wno-suggest-override -Wno-sign-compare \
		 -Wno-ignored-qualifiers \
		 -Wno-unused-variable -Wno-char-subscripts -Wno-reorder \
		 -Wno-int-in-bool-context
	FLAGS += -DWINDOWS
endif

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
