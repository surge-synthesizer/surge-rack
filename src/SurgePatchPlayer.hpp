#pragma once
#include "Surge.hpp"
#include "SurgeModuleCommon.hpp"
#include "rack.hpp"
#include <SurgeSynthesizer.h>
#include <cstring>

class HeadlessPluginLayerProxy : public SurgeSynthesizer::PluginLayer
{
  public:
    void surgeParameterUpdated(const SurgeSynthesizer::ID &id, float d) override {}
    void surgeMacroUpdated(long macroNum, float d) override {}
};

struct SurgePatchPlayer : virtual public SurgeModuleCommon
{
    enum ParamIds
    {
        OUTPUT_GAIN,
        INPUT_GAIN,

        CATEGORY_IDX,
        PATCH_IN_CATEGORY_IDX,

        LOAD_PATCH,

        NUM_PARAMS
    };
    enum InputIds
    {
        INPUT_L,
        INPUT_R,

        PITCH_CV,
        VEL_CV,
        GATE_CV,

        PITCHBEND_CV,
        MODWHEEL_CV,
        CHAN_AT_CV,
        POLY_AT_CV,
        TIMBRE_CV,

        CONTROL_0_CV,

        CLOCK_CV = CONTROL_0_CV + 8,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT_L,
        OUTPUT_R,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NEEDS_LOAD,
        NUM_LIGHTS
    };

    SurgePatchPlayer() : SurgeModuleCommon()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(OUTPUT_GAIN, 0, 1, 1, "Output Gain");
        configParam(INPUT_GAIN, 0, 1, 1, "Input Gain");

        configParam(CATEGORY_IDX, 0, 1, 0, "Category Index");
        configParam(PATCH_IN_CATEGORY_IDX, 0, 1, 0, "Patch Index");

        configParam(LOAD_PATCH, 0, 1, 0, "Load Param");
        setupSurge();
    }

    ~SurgePatchPlayer() { storage.release(); }

    virtual std::string getName() override { return "PatchPlayer"; }

    StringCache pitch0DisplayCache;

    StringCache paramNameCache[n_osc_params], paramValueCache[n_osc_params], patchInfoCache[3];
    ParamValueStateSaver knobSaver;

    std::vector<int> catOrderSkipEmpty;

    virtual void setupSurge()
    {
        setupSurgeCommon(NUM_PARAMS);
        surge_synth.reset(
            new SurgeSynthesizer(new HeadlessPluginLayerProxy(), storage->datapath.u8string()));

        storage.reset(&(surge_synth->storage));
        storage->refresh_patchlist();
        for (auto ci : storage->patchCategoryOrdering)
        {
            PatchCategory pc = storage->patch_category[ci];
            if (pc.numberOfPatchesInCatgory != 0)
            {
                catOrderSkipEmpty.push_back(ci);
            }
        }

        updatePatchIdx();
        loadPatch();

        pc.update(this);

        for (int i = 0; i < 16; ++i)
        {
            delayTrigger[i] = -1;
            noteForChannel[i] = -1;
        }

        CV_to_midi[MODWHEEL_CV] = 1;
        for (int i = 0; i < 8; ++i)
        {
            CV_to_midi[CONTROL_0_CV + i] = 41 + i;
        }
        memset(inpL, 0, BLOCK_SIZE * sizeof(float));
        memset(inpR, 0, BLOCK_SIZE * sizeof(float));
    }

    void loadPatch()
    {
        if (storage->patch_list.size() == 0)
        {
            patchInfoCache[0].reset("Error");
            patchInfoCache[1].reset("No Patches Loaded from Plugin");
            patchInfoCache[2].reset("did you 'make dist'?");
            return;
        }

        loadedPatchIdx = patchIdx;
        surge_synth->loadPatch(loadedPatchIdx);

        Patch p = storage->patch_list[patchIdx];
        PatchCategory pc = storage->patch_category[p.category];
        // Thinking of changing these? Remember we stream them into JSON below
        // so be careful
        patchInfoCache[0].reset(pc.name);
        patchInfoCache[1].reset(p.name);

        std::string aut = "author: ";
        aut = aut + storage->getPatch().author;
        patchInfoCache[2].reset(aut);
    }

    int patchIdx = 0, loadedPatchIdx = -1;
    ;
    StringCache patchCategoryName;
    StringCache patchItemName[7]; // 3 on each side

    void updatePatchIdx()
    {
        if (storage->patch_category.size() == 0)
            return;

        int priorPatchIdx = patchIdx;

        /*
        ** FIXME: So many ways to make this calculate less (like stash int of
        *patchIdx and stuf
        */
        int catIdx = getParam(CATEGORY_IDX) * catOrderSkipEmpty.size();
        if (catIdx >= catOrderSkipEmpty.size())
            catIdx--;

        PatchCategory cat = storage->patch_category[catOrderSkipEmpty[catIdx]];

        patchCategoryName.reset(cat.name);

        int patchInCatIdx = getParam(PATCH_IN_CATEGORY_IDX) * cat.numberOfPatchesInCatgory;
        if (patchInCatIdx == cat.numberOfPatchesInCatgory)
            patchInCatIdx--;

        int catO = catOrderSkipEmpty[catIdx];
        int counted = 0;
        Patch p;
        bool found = false;
        int firstCount = -1, lastCount = 0;
        ;
        for (auto pci : storage->patchOrdering)
        {
            if (storage->patch_list[pci].category == catO)
            {
                int countDiff = counted++ - patchInCatIdx;
                if (countDiff == 0)
                {
                    p = storage->patch_list[pci];
                    found = true;
                    patchIdx = pci;
                }
                countDiff += 3;

                if (countDiff >= 7)
                    break;
                if (countDiff >= 0)
                {
                    if (firstCount < 0)
                        firstCount = countDiff;
                    lastCount = countDiff;
                    patchItemName[countDiff].resetCheck(storage->patch_list[pci].name);
                }
            }
        }

        for (int i = 0; i < firstCount; ++i)
            patchItemName[i].resetCheck("-");
        for (int i = lastCount + 1; i < 7; ++i)
            patchItemName[i].resetCheck("-");

        if (!found)
            for (int i = 0; i < 7; ++i)
                patchItemName[i].resetCheck("ERROR");
    }

    rack::dsp::SchmittTrigger voiceTrigger[16];
    int delayTrigger[16];
    int noteForChannel[16];
    int processPosition = BLOCK_SIZE;

    std::map<int, int> CV_to_midi;
    float inpL[BLOCK_SIZE], inpR[BLOCK_SIZE];
    float lastClockCV = -100;

    void process(const typename rack::Module::ProcessArgs &args) override
    {
        int nChan = inputs[GATE_CV].getChannels();
        outputs[OUTPUT_L].setChannels(1);
        outputs[OUTPUT_R].setChannels(1);

        if (pc.changed(CATEGORY_IDX, this) || pc.changed(PATCH_IN_CATEGORY_IDX, this))
        {
            updatePatchIdx();
        }

        if (pc.changedAndIsNonZero(LOAD_PATCH, this) || firstRespawnIsFromJSON)
        {
            surge_synth->allNotesOff();
            loadPatch();
        }

        if (patchIdx != loadedPatchIdx)
        {
            setLight(NEEDS_LOAD, 1.0);
        }
        else
        {
            setLight(NEEDS_LOAD, 0.0);
        }

        for (int i = 0; i < nChan; ++i)
        {
            bool newVoice = false;
            if (voiceTrigger[i].process(inputs[GATE_CV].getVoltage(i)))
            {
                delayTrigger[i] = 8;
            }

            if (delayTrigger[i] == 0)
            {
                newVoice = true;
                delayTrigger[i] = -1;
            }

            if (delayTrigger[i] > 0)
                delayTrigger[i]--;

            if (newVoice)
            {
                float note = 60 + inputs[PITCH_CV].getPolyVoltage(i) * 12.0;
                float vel =
                    inputs[VEL_CV].isConnected() ? inputs[VEL_CV].getPolyVoltage(i) * 12.7 : 100;

                noteForChannel[i] = (int)note;
                surge_synth->playNote(i, (int)note, (int)vel, 0);
            }

            if (noteForChannel[i] > 0 && inputs[GATE_CV].getVoltage(i) < 0.02)
            {
                surge_synth->releaseNote(i, noteForChannel[i],
                                         0); // FIXME: Release velocity
                noteForChannel[i] = -1;
            }
        }

        if (processPosition == BLOCK_SIZE)
        {
            // BPM Check
            float thisClockCV = lastClockCV;
            if (inputConnected(CLOCK_CV))
            {
                thisClockCV = inputs[CLOCK_CV].getVoltage();
            }
            else
            {
                thisClockCV = 1;
            }
            if (thisClockCV != lastClockCV)
            {
                updateBPMFromClockCV(thisClockCV, args.sampleTime, args.sampleRate);
                lastClockCV = thisClockCV;

                surge_synth->time_data.tempo = lastBPM;
            }

            // Start with the monophonic controllers
            if (inputs[PITCHBEND_CV].isConnected())
            {
                auto pb = rack::clamp(inputs[PITCHBEND_CV].getVoltage(), -5.f, 5.f);
                int pbi = (int)pb * 8192 * 2 / 5.0;
                surge_synth->pitchBend(0, pbi);
            }

            if (inputs[CHAN_AT_CV].isConnected())
            {
                auto pb = rack::clamp(inputs[CHAN_AT_CV].getVoltage(), -5.f, 5.f);
                int pbi = (int)pb * 12.7;
                surge_synth->channelAftertouch(0, pbi);
            }

            for (auto mcc : CV_to_midi)
            {
                auto param = mcc.first;
                auto midi = mcc.second;
                if (inputs[param].isConnected())
                {
                    auto pb = rack::clamp(inputs[param].getVoltage(), 0.f, 10.f);
                    int pbi = (int)(pb * 12.7);
                    surge_synth->channelController(0, midi, pbi);
                }
            }

            // and the polyphonic ones
            bool needsMPE = false;
            if (inputs[POLY_AT_CV].isConnected())
            {
                needsMPE = true;
                for (int i = 0; i < inputs[POLY_AT_CV].getChannels(); ++i)
                {
                    if (noteForChannel[i] >= 0)
                    {
                        auto pb = rack::clamp(inputs[POLY_AT_CV].getVoltage(i), -5.f, 5.f);
                        int pbi = (int)pb * 12.7;
                        surge_synth->polyAftertouch(i, noteForChannel[i], pbi);
                    }
                }
            }

            if (inputs[TIMBRE_CV].isConnected())
            {
                needsMPE = needsMPE || (inputs[TIMBRE_CV].getChannels() > 1);
                for (int i = 0; i < inputs[TIMBRE_CV].getChannels(); ++i)
                {
                    auto pb = rack::clamp(inputs[TIMBRE_CV].getVoltage(i), -5.f, 5.f);
                    int pbi = (int)pb * 12.7;
                    surge_synth->channelController(i, 73, pbi);
                }
            }
            if (needsMPE != surge_synth->mpeEnabled)
            {
                surge_synth->mpeEnabled = needsMPE;
            }

            // Audio Routing
            if (inputs[INPUT_L].isConnected() || inputs[INPUT_R].isConnected())
            {
                surge_synth->process_input = true;
                memcpy(surge_synth->input[0], inpL, BLOCK_SIZE * sizeof(float));
                memcpy(surge_synth->input[1], inpR, BLOCK_SIZE * sizeof(float));
            }
            else
            {
                surge_synth->process_input = false;
            }

            surge_synth->process();
            processPosition = 0;
        }

        if (inputs[INPUT_L].isConnected() || inputs[INPUT_R].isConnected())
        {
            float inpG = params[INPUT_GAIN].getValue();
            float il = inputs[INPUT_L].getVoltage() * RACK_TO_SURGE_OSC_MUL;
            float ir;
            if (inputs[INPUT_R].isConnected())
                ir = inputs[INPUT_R].getVoltage() * RACK_TO_SURGE_OSC_MUL;
            else
                ir = il;
            inpL[processPosition] = il * inpG;
            inpR[processPosition] = ir * inpG;
        }

        float outG = params[OUTPUT_GAIN].getValue();
        float outl = surge_synth->output[0][processPosition] * SURGE_TO_RACK_OSC_MUL * outG;
        float outr = surge_synth->output[1][processPosition] * SURGE_TO_RACK_OSC_MUL * outG;

        if (outputs[OUTPUT_R].isConnected())
        {
            outputs[OUTPUT_L].setVoltage(outl);
            outputs[OUTPUT_R].setVoltage(outr);
        }
        else
        {
            outputs[OUTPUT_L].setVoltage((outl + outr) * 0.5);
        }

        processPosition++;
        pc.update(this);
        firstRespawnIsFromJSON = false;
    }

    std::unique_ptr<SurgeSynthesizer> surge_synth;

    virtual json_t *dataToJson() override
    {
        json_t *rootJ = makeCommonDataJson();
        json_object_set_new(rootJ, "patchCategoryName",
                            json_string(patchInfoCache[0].value.c_str()));
        json_object_set_new(rootJ, "patchItemName", json_string(patchInfoCache[1].value.c_str()));
        return rootJ;
    }

    virtual void dataFromJson(json_t *rootJ) override
    {
        readCommonDataJson(rootJ);
        // So do we have the values and if so do a reset of index
        json_t *jcat = json_object_get(rootJ, "patchCategoryName");
        json_t *jpatch = json_object_get(rootJ, "patchItemName");
        if (jcat && jpatch)
        {
            if (storage->patch_category.size() == 0)
            {
                WARN("Found no patches present in dataFromJSON; ignoring saved "
                     "state");
                return;
            }

            std::string cat = json_string_value(jcat);
            std::string patch = json_string_value(jpatch);

            int catIdx = -1;
            int catPos = 0;
            for (auto idx : catOrderSkipEmpty)
            {
                if (storage->patch_category[idx].name == cat)
                {
                    catIdx = idx;
                }
                if (catIdx < 0)
                    catPos++;
            }
            if (catIdx < 0)
                catPos = 0; // the category has been removed

            int newIdx = -1;
            int idxInCat = 0;
            for (auto pci : storage->patchOrdering)
            {
                if (storage->patch_category[storage->patch_list[pci].category].name == cat)
                {
                    if (storage->patch_list[pci].name == patch)
                    {
                        newIdx = pci;
                    }
                    if (newIdx < 0)
                        idxInCat++;
                }
            }
            if (newIdx < 0)
            {
                idxInCat = 0; // the wavetable has been removed;
                newIdx = 0;
            }

            params[CATEGORY_IDX].setValue(catPos * 1.0 / catOrderSkipEmpty.size());
            params[PATCH_IN_CATEGORY_IDX].setValue(
                idxInCat * 1.0 /
                storage->patch_category[storage->patch_list[newIdx].category]
                    .numberOfPatchesInCatgory);
            params[LOAD_PATCH].setValue(10.0);
            updatePatchIdx();
        }
    }
};
