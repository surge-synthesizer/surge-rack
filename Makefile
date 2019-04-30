RACK_DIR ?= ../..

ifneq ("$(wildcard $(RACK_DIR)/helper.py)","")
	RACK_VERSION=1
	RACK_FLAG=-DRACK_V1
	RACK_GO=(cd ~/dev/VCVRack/V1/Rack && make run)
else
	RACK_VERSION=062
	RACK_FLAG=-DRACK_V062
	SLUG=SurgeRack
	VERSION=0.6.0
	RACK_GO=	unzip -o dist/$(SLUG)-$(VERSION)-$(ARCH).zip -d ~/Documents/Rack/plugins && (cd ~/Documents/Rack && /Applications/Rack.app/Contents/MacOS/Rack ) 
endif

include $(RACK_DIR)/arch.mk

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -Isurge/src/common -Isurge/src/common/dsp \
	-Isurge/libs/xml \
	-Isurge/libs/filesystem \
	-Isurge/src/headless \
	-DRELEASE=1 \
	-DTARGET_HEADLESS \
	-DTARGET_RACK

FLAGS += $(RACK_FLAG)

CFLAGS +=
CXXFLAGS += 

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

SRG=surge/src/common
SOURCES += $(SRG)/Parameter.cpp \
	$(SRG)/Sample.cpp \
	$(SRG)/SampleLoadRiffWave.cpp \
	$(SRG)/SurgeError.cpp \
	$(SRG)/SurgePatch.cpp \
	$(SRG)/SurgeStorage.cpp \
	$(SRG)/SurgeStorageLoadWavetable.cpp \
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
    $(SRG)/dsp/effect/PhaserEffect.cpp \
    $(SRG)/dsp/effect/Reverb1Effect.cpp \
    $(SRG)/dsp/effect/Reverb2Effect.cpp \
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
	$(SRL)/xml/tinyxmlparser.cpp

ifdef ARCH_MAC
SOURCES += $(SRL)/filesystem/filesystem.cpp

LDFLAGS += -framework CoreFoundation -framework CoreServices
endif

ifdef ARCH_WIN
SOURCES += $(SRL)/filesystem/filesystem.cpp
endif

ifdef ARCH_LIN
SOURCES += surge/src/linux/ConfigurationXml.S
LDFLAGS += -lstdc++fs

# This is really a hack but...
build/surge/src/linux/ConfigurationXml.S.o: surge/src/linux/ConfigurationXml.S
	mkdir -p build/surge/src/linux
	cd surge/src/linux && $(CC) -c ConfigurationXml.S -o ../../../$@

endif

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res docs patches README.md

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
	FLAGS += -Isurge/src/linux
endif

ifdef ARCH_WIN
	FLAGS += -Wno-suggest-override -Wno-sign-compare \
		 -Wno-ignored-qualifiers \
		 -Wno-unused-variable -Wno-char-subscripts -Wno-reorder \
		 -Wno-int-in-bool-context 
	FLAGS += -DWINDOWS -Isurge/src/windows
endif

COMMUNITY_ISSUE=https://github.com/VCVRack/community/issues/FIXME

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

install_local:	dist
	unzip -o dist/$(SLUG)-$(VERSION)-$(ARCH).zip -d ~/Documents/Rack/plugins

run_local:	install_local
	/Applications/Rack.app/Contents/MacOS/Rack

missing_symbols:	dist
	nm plugin.dylib  | grep " U " | c++filt


go:	dist
	$(RACK_GO)

dbg:	dist
	(cd ~/dev/VCVRack/V1/Rack && make && lldb -- ./Rack -d)

# Special target since we don't have zip on azure (fix this later)
win-dist: all
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
