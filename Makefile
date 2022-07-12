RACK_DIR ?= ../..

RACK_VERSION=1
RACK_FLAG=-DRACK_V1

SURGE_RACK_BASE_VERSION=XT1-0-1
SURGE_RACK_PLUG_VERSION=$(shell git rev-parse --short HEAD)
SURGE_RACK_SURGE_VERSION=$(shell cd surge && git rev-parse --short HEAD)

include $(RACK_DIR)/arch.mk

CMAKE_TOOLCHAIN =
LIBLUAJIT_PATH_PREFIX =
LIBFILESYSTEM = surge/ignore/rack-build/libs/filesystem/libfilesystem.a

ifdef ARCH_WIN
CMAKE_TOOLCHAIN += -DCMAKE_TOOLCHAIN_FILE=$(PLUGIN_DIR)/mingw-w64-x86_64.cmake
LIBFILESYSTEM = 
endif

ifdef ARCH_LIN
CMAKE_TOOLCHAIN += -DCMAKE_TOOLCHAIN_FILE=$(PLUGIN_DIR)/x86_64-ubuntu16.04-linux-gnu.cmake
endif

ifdef ARCH_MAC
LIBLUAJIT_PATH_PREFIX += luajit/bin
endif

libsurge := surge/ignore/rack-build/src/common/libsurge-common.a
# Build the static library into your plugin.dll/dylib/so
# TODO: This needs to be platform variated and we need to see which we need
OBJECTS += $(libsurge) \
	surge/ignore/rack-build/src/common/libsurge-common-binary.a \
	surge/ignore/rack-build/src/lua/libsurge-lua-src.a \
	surge/ignore/rack-build/libs/tinyxml/libtinyxml.a \
    surge/ignore/rack-build/libs/libsamplerate/src/libsamplerate.a \
    surge/ignore/rack-build/libs/fmt/libfmt.a \
    surge/ignore/rack-build/libs/strnatcmp/libstrnatcmp.a \
    $(LIBFILESYSTEM) \
    surge/ignore/rack-build/libs/oddsound-mts/liboddsound-mts.a \
    surge/ignore/rack-build/libs/sqlite-3.23.3/libsqlite.a \
    surge/ignore/rack-build/libs/airwindows/libairwindows.a \
    surge/ignore/rack-build/libs/LuaJitLib/$(LIBLUAJIT_PATH_PREFIX)/libluajit.a \
    surge/ignore/rack-build/libs/eurorack/libeurorack.a \
    surge/ignore/rack-build/src/platform/libsurge-platform.a

# Trigger the static library to be built when running `make dep`
DEPS += $(libsurge)

$(libsurge):
	# Out-of-source build dir
	echo $(CMAKE)
	cd surge && CFLAGS= && $(CMAKE) -Bignore/rack-build -G "Unix Makefiles"
	#cd surge && CFLAGS= && cmake $(CMAKE_TOOLCHAIN) -Bignore/rack-build -G "Unix Makefiles"
	# $(CMAKE) doesn't work here since the arguments are borked so use make directly. Sigh.
	cd surge/ignore/rack-build && CFLAGS= && make -j 1 surge-common

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -Isurge/src/common \
	-Isurge/src/common/dsp \
	-Isurge/src/common/dsp/filters \
	-Isurge/src/common/dsp/vembertech \
	-Isurge/src/common/dsp/utilities \
	-Isurge/src/common/dsp/oscillators \
	-Isurge/src/common/dsp/modulators \
	-Isurge/src/surge-testrunner \
	-Isurge/libs/tinyxml/include \
	-Isurge/libs/filesystem \
	-Isurge/libs/LuaJitLib/LuaJIT/src  \
	-Isurge/ignore/rack-build/libs/filesystem/include \
	-Isurge/libs/strnatcmp \
	-Isurge/src/headless \
        -Isurge/libs/tuning-library/include \
        -include limits \
	-DRELEASE=1 \

# to understand that -include limits, btw: Surge 1.7 doesn't include it but uses numeric_limits. The windows
# toolchain rack uses requires the install (the surge toolchain implicitly includes it). Rather than patch 
# surge 1.7.1 for an include, just slap it in the code here for now. See #307


FLAGS += $(RACK_FLAG) -DTIXML_USE_STL=1

CFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

# ASM ERRORS need fixing


ifdef ARCH_MAC
FLAGS += -std=c++17 -fvisibility=hidden -fvisibility-inlines-hidden
LDFLAGS += -framework CoreFoundation -framework CoreServices
endif

ifdef ARCH_WIN
FLAGS += -std=c++17 -fvisibility=hidden -fvisibility-inlines-hidden
LDFLAGS += -lwinmm
endif

ifdef ARCH_LIN
FLAGS += -std=c++17 -fvisibility=hidden -fvisibility-inlines-hidden
LDFLAGS += -pthread
endif

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
dist:	build/surge-data res

build/surge-data:
	mkdir -p build/surge-data

DISTRIBUTABLES += $(wildcard LICENSE*) res docs patches presets README.md build/surge-data

# Include the VCV plugin Makefile framework
include $(RACK_DIR)/plugin.mk


# Add Surge Specific make flags based on architecture
ifdef ARCH_MAC
# Obvioulsy get rid of this one day
	FLAGS += 	-Wno-undefined-bool-conversion \
	-Wno-unused-variable \
	-Wno-reorder \
	-Wno-char-subscripts \
	-Wno-sign-compare \
	-Wno-ignored-qualifiers \
	-Wno-c++17-extensions
	FLAGS += -DMAC -D"_aligned_malloc(x,a)=malloc(x)" -D"_aligned_free(x)=free(x)"
endif

ifdef ARCH_LIN
	FLAGS += -DLINUX -D"_aligned_malloc(x,a)=malloc(x)" -D"_aligned_free(x)=free(x)"
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
	FLAGS += -DWINDOWS -Isurge/src/windows
endif

FLAGS += -DSURGE_RACK_BASE_VERSION=$(SURGE_RACK_BASE_VERSION)
FLAGS += -DSURGE_RACK_PLUG_VERSION=$(SURGE_RACK_PLUG_VERSION)
FLAGS += -DSURGE_RACK_SURGE_VERSION=$(SURGE_RACK_SURGE_VERSION)

COMMUNITY_ISSUE=https://github.com/VCVRack/community/issues/565

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


# Special target since we don't have zip on azure (fix this later)
win-dist: all build/surge-data
	rm -rf dist
	mkdir -p dist/$(SLUG)
	@# Strip and copy plugin binary
	cp $(TARGET) dist/$(SLUG)/
ifdef ARCH_MAC
	$(STRIP) -S dist/$(SLUG)/$(TARGET)
else
	$(STRIP) -s dist/$(SLUG)/$(TARGET)
endif
	@# Copy distributables
	cp -R $(DISTRIBUTABLES) dist/$(SLUG)/
	@# Create ZIP package
	echo "cd dist && 7z.exe a $(SLUG)-$(VERSION)-$(ARCH).zip -r $(SLUG)"
	cd dist && 7z.exe a $(SLUG)-$(VERSION)-$(ARCH).zip -r $(SLUG)
