RACK_DIR ?= ../..

RACK_VERSION=1
RACK_FLAG=-DRACK_V1

SURGE_RACK_BASE_VERSION=1.7
SURGE_RACK_PLUG_VERSION=$(shell git rev-parse --short HEAD)
SURGE_RACK_SURGE_VERSION=$(shell cd surge && git rev-parse --short HEAD)

include $(RACK_DIR)/arch.mk

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -Isurge/src/common -Isurge/src/common/dsp \
	-Isurge/libs/xml \
	-Isurge/libs/filesystem \
	-Isurge/libs/strnatcmp \
	-Isurge/src/headless \
        -Isurge/libs/tuning-library/include \
	-DRELEASE=1 \
	-DTARGET_HEADLESS \
	-DTARGET_RACK 


FLAGS += $(RACK_FLAG)

CFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

SRG=surge/src/common
SOURCES += $(SRG)/Parameter.cpp \
	$(SRG)/WavSupport.cpp \
	$(SRG)/SurgeError.cpp \
	$(SRG)/SurgePatch.cpp \
	$(SRG)/SurgeStorage.cpp \
	$(SRG)/SurgeSynthesizer.cpp \
	$(SRG)/SurgeSynthesizerIO.cpp \
	$(SRG)/UserDefaults.cpp \
	$(SRG)/precompiled.cpp \
    $(SRG)/dsp/AdsrEnvelope.cpp \
    $(SRG)/dsp/BiquadFilter.cpp \
    $(SRG)/dsp/BiquadFilterSSE2.cpp \
    $(SRG)/dsp/DspUtilities.cpp \
    $(SRG)/dsp/FMOscillator.cpp \
    $(SRG)/dsp/FilterCoefficientMaker.cpp \
    $(SRG)/dsp/LfoModulationSource.cpp \
    $(SRG)/dsp/Oscillator.cpp \
    $(SRG)/dsp/QuadFilterChain.cpp \
    $(SRG)/dsp/QuadFilterUnit.cpp \
    $(SRG)/dsp/SampleAndHoldOscillator.cpp \
    $(SRG)/dsp/SurgeSuperOscillator.cpp \
    $(SRG)/dsp/SurgeVoice.cpp \
    $(SRG)/dsp/VectorizedSvfFilter.cpp \
    $(SRG)/dsp/Wavetable.cpp \
    $(SRG)/dsp/WavetableOscillator.cpp \
    $(SRG)/dsp/WindowOscillator.cpp \
    $(SRG)/dsp/effect/ConditionerEffect.cpp \
    $(SRG)/dsp/effect/DistortionEffect.cpp \
    $(SRG)/dsp/effect/DualDelayEffect.cpp \
    $(SRG)/dsp/effect/Effect.cpp \
    $(SRG)/dsp/effect/FreqshiftEffect.cpp \
    $(SRG)/dsp/effect/FlangerEffect.cpp \
    $(SRG)/dsp/effect/PhaserEffect.cpp \
    $(SRG)/dsp/effect/Reverb1Effect.cpp \
    $(SRG)/dsp/effect/Reverb2Effect.cpp \
    $(SRG)/dsp/effect/RingModulatorEffect.cpp \
    $(SRG)/dsp/effect/RotarySpeakerEffect.cpp \
    $(SRG)/dsp/effect/VocoderEffect.cpp \
    $(SRG)/vt_dsp/basic_dsp.cpp \
    $(SRG)/vt_dsp/halfratefilter.cpp \
    $(SRG)/vt_dsp/lipol.cpp \
    $(SRG)/vt_dsp/macspecific.cpp \
    $(SRG)/thread/CriticalSection.cpp

# ASM ERRORS need fixing


SRL=surge/libs
SOURCES += $(SRL)/xml/tinystr.cpp \
	$(SRL)/xml/tinyxml.cpp \
	$(SRL)/xml/tinyxmlerror.cpp \
	$(SRL)/xml/tinyxmlparser.cpp \
	$(SRL)/strnatcmp/strnatcmp.cpp

ifdef ARCH_MAC
SOURCES += $(SRL)/filesystem/filesystem.cpp

LDFLAGS += -framework CoreFoundation -framework CoreServices
endif

ifdef ARCH_WIN
SOURCES += $(SRL)/filesystem/filesystem.cpp
LDFLAGS += -lwinmm
endif

ifdef ARCH_LIN
SOURCES += surge/src/linux/ConfigurationXml.S
LDFLAGS += -lstdc++fs -pthread
FLAGS += -DUSE_STD_EXPERIMENTAL_FILESYSTEM

# This is really a hack but...
build/surge/src/linux/ConfigurationXml.S.o: surge/src/linux/ConfigurationXml.S
	mkdir -p build/surge/src/linux
	cd surge/src/linux && $(CC) -c ConfigurationXml.S -o ../../../$@

endif

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
dist:	build/surge-data res

build/surge-data:
	mkdir -p build/surge-data
	cp surge/resources/data/configuration.xml build/surge-data
	cp surge/resources/data/paramdocumentation.xml build/surge-data
	cp surge/resources/data/windows.wt build/surge-data
	cp -R surge/resources/data/wavetables build/surge-data/wavetables
	cp -R surge/resources/data/patches_factory build/surge-data/patches_factory
	rm build/surge-data/patches_factory/leads/*computer.fxp

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
