RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

EXTRA_CMAKE :=
RACK_PLUGIN_NAME := plugin
RACK_PLUGIN_EXT := so

ifdef ARCH_WIN
  RACK_PLUGIN_EXT := dll
endif

ifdef ARCH_MAC
  EXTRA_CMAKE := -DCMAKE_OSX_ARCHITECTURES="x86_64"
  RACK_PLUGIN_EXT := dylib
  ifdef ARCH_ARM64
    EXTRA_CMAKE := -DCMAKE_OSX_ARCHITECTURES="arm64"
  endif
endif

RACK_PLUGIN := $(RACK_PLUGIN_NAME).$(RACK_PLUGIN_EXT)

CMAKE_BUILD ?= dep/cmake-build
cmake_rack_plugin := $(CMAKE_BUILD)/$(RACK_PLUGIN)

# create empty plugin lib to skip the make target execution
$(shell touch $(RACK_PLUGIN))
$(info cmake_rack_plugin target is '$(cmake_rack_plugin)')

# trigger CMake build when running `make dep`
DEPS += $(cmake_rack_plugin)

$(cmake_rack_plugin): CMakeLists.txt
	$(CMAKE) -B $(CMAKE_BUILD) -DRACK_SDK_DIR=$(RACK_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$(CMAKE_BUILD)/dist $(EXTRA_CMAKE)
	cmake --build $(CMAKE_BUILD) -- -j $(shell getconf _NPROCESSORS_ONLN)
	cmake --install $(CMAKE_BUILD)

rack_plugin: $(cmake_rack_plugin)
	cp -vf $(cmake_rack_plugin) .

# Add files to the ZIP package when running `make dist`
dist: rack_plugin build/surge-data res

build/surge-data:
	mkdir -p build/surge-data
	cp surge/resources/surge-shared/windows.wt build/surge-data
	cp surge/resources/surge-shared/configuration.xml build/surge-data
	cp -R surge/resources/data/wavetables build/surge-data/wavetables
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
