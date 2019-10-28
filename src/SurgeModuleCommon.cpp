#include "SurgeModuleCommon.hpp"
#include <string>

void SurgeRackParamBinding::updateFloat(const ParamCache &pc, int polyChannel, SurgeModuleCommon *m)
{
    bool paramChanged = false;
    if(pc.changed(param_id,m) || (ts_id >= 0 && pc.changed(ts_id, m) ) || forceRefresh)
    {
        char txt[1024];
        p->set_value_f01(m->getParam(param_id));
        if( ts_id >= 0 )
            p->temposync = m->getParam(ts_id) > 0.5;
        
        p->get_display(txt, false, 0);
        if( tsbpmLabel && ts_id >= 0 && m->getParam(ts_id) > 0.5 )
        {
            char ntxt[1024];
            snprintf(ntxt, 1024, "%s @ %5.1lf bpm", txt, m->lastBPM );
            strcpy(txt, ntxt);
        }
        valCache.reset(txt);
        paramChanged = true;
    }
    if(forceRefresh)
    {
        nameCache.reset(p->get_name());
    }
    
    if( paramChanged || forceRefresh || (cv_id >= 0 && m->inputConnected(cv_id)) )
        p->set_value_f01(m->getParam(param_id) + m->inputs[cv_id].getPolyVoltage(polyChannel) / 10.0);
}

void SurgeRackParamBinding::updateBool(const ParamCache &pc, int polyChannel, SurgeModuleCommon *m, bool notIt)
{
    if(pc.changed(param_id,m) || forceRefresh)
    {
        char txt[1024];

        bool current = m->getParam(param_id) > 0.5;
        if( notIt ) current = ! current;

        if( ( current != p->val.b ) || forceRefresh )
        {
            p->val.b = current;
            p->get_display(txt, false, 0);
            valCache.reset(txt);
        }
    }

    if(forceRefresh)
    {
        nameCache.reset(p->get_name());
    }
}


void SurgeRackParamBinding::updateInt(const ParamCache &pc, int polyChannel, SurgeModuleCommon *m)
{
    if(pc.changed(param_id,m) || forceRefresh)
    {
        char txt[1024];

        int current = (int)m->getParam(param_id);

        if( ( current != p->val.i ) || forceRefresh )
        {
            p->val.i = current;
            p->get_display(txt, false, 0);
            valCache.reset(txt);
        }
    }

    if(forceRefresh)
    {
        nameCache.reset(p->get_name());
    }
}

void SurgeModuleCommon::setupSurgeCommon(int NUM_PARAMS) 
{
    std::string dataPath;
    dataPath = rack::asset::plugin(pluginInstance, "build/surge-data/");
    std::string cxml = dataPath + "configuration.xml";
    FILE *cxmlF = fopen(cxml.c_str(), "r");
    if( cxmlF )
    {
        fclose(cxmlF);
    }
    else
    {
        dataPath = rack::asset::plugin(pluginInstance, "surge-data/" );
    }

    
    showBuildInfo();
    storage.reset(new SurgeStorage(dataPath));
    
    INFO("[SurgeRack] storage::dataPath = '%s'", storage->datapath.c_str());
    INFO("[SurgeRack] storage::userDataPath = '%s'", storage->userDataPath.c_str());

    onSampleRateChange();

    pb.resize(NUM_PARAMS);
    pc.resize(NUM_PARAMS);
}

std::string SurgeRackParamQuantity::getLabel() {
    SurgeModuleCommon *mc = static_cast<SurgeModuleCommon *>(module);
    if( mc )
    {
        std::shared_ptr<SurgeRackParamBinding> p = mc->pb[paramId];
        if( p.get() )
        {
            return p->nameCache.value;
        }
        if( ts_companion == -2 )
        {
            for( auto srpb : mc->pb )
                if(srpb.get() && srpb->ts_id == paramId)
                    ts_companion = srpb->param_id;
            if( ts_companion == -2 ) ts_companion = -1; // only check once
        }
        if( ts_companion >= 0 )
        {
            char txt[1024];
            snprintf(txt, 1024, "TempoSync %s", mc->pb[ts_companion]->nameCache.value.c_str() );
            return txt;
        }
    }
    return ParamQuantity::getLabel();
}

std::string SurgeRackParamQuantity::getDisplayValueString() {
    SurgeModuleCommon *mc = static_cast<SurgeModuleCommon *>(module);
    if( mc )
    {
        std::shared_ptr<SurgeRackParamBinding> pbn = mc->pb[paramId];
        if( pbn.get() )
        {
            switch (pbn->p->ctrltype)
            {
            case ct_lforate:
            case ct_envtime:
            case ct_envtime_lfodecay:
            case ct_reverbtime:
            case ct_delaymodtime:
                if( ! pbn->p->temposync )
                    return pbn->valCache.value;
                break;
                
            case ct_percent:
            case ct_percent_bidirectional:
            case ct_amplitude:
            case ct_freq_hpf:
            case ct_freq_audible:

            case ct_decibel:
            case ct_decibel_attenuation:
            case ct_decibel_attenuation_large:
            case ct_decibel_fmdepth:
            case ct_decibel_narrow:
            case ct_decibel_extra_narrow:

            case ct_detuning:
            case ct_bandwidth:

            case ct_freq_shift:
                
                return pbn->valCache.value;
                
            default:
                break;
            }
        }
    }

    return ParamQuantity::getDisplayValueString();
}

void SurgeRackParamQuantity::setDisplayValueString(std::string s) {
    SurgeModuleCommon *mc = static_cast<SurgeModuleCommon *>(module);
    bool foundValue = false;
    if( mc )
    {
        std::shared_ptr<SurgeRackParamBinding> pbn = mc->pb[paramId];
        if( pbn.get() )
        {
            float newValue = 0;
            switch (pbn->p->ctrltype)
            {
            case ct_lforate:
            case ct_envtime:
            case ct_envtime_lfodecay:
            case ct_reverbtime:
            case ct_delaymodtime:
                if( ! pbn->p->temposync )
                {
                    float entered = std::stof(s);
                    // hz are 2^newValue so newValue = log2(hz)
                    // seconds are also 2^value
                    newValue = log2f(entered);
                    foundValue = true;
                }
                break;
            case ct_percent:
            {
                float entered = std::stof(s);
                newValue = entered / 100.0;
                foundValue = true;
            }
            break;
            case ct_percent_bidirectional:
            {
                float entered = std::stof(s);
                newValue = entered / 100.0;
                foundValue = true;
            }
            break;
            case ct_amplitude:
            {
                // amp_to_db is 18 * log2(v) so the inverse is pow(2, inp/18)
                float entered = std::stof(s);
                newValue = pow(2.0, entered/18.0 );
                foundValue = true;
            }
            break;
            case ct_freq_hpf:
            case ct_freq_audible:
            {
                // o = 440 * 2 ^ ( v/12 ) is the string; so v = 12 * log2( o / 440 )
                float entered = std::stof(s);
                newValue = 12.0 * log2f( entered / 440.0 );
                foundValue = true;
            }
            break;
            
            case ct_decibel:
            case ct_decibel_attenuation:
            case ct_decibel_attenuation_large:
            case ct_decibel_fmdepth:
            case ct_decibel_narrow:
            case ct_decibel_extra_narrow:
            case ct_bandwidth:
            case ct_freq_shift:
            {
                float entered = std::stof(s);
                newValue = entered;
                foundValue = true;
            }
            break;
            // 13 and 33

            case ct_detuning:
            {
                float entered = std::stof(s);
                newValue = entered / 100.0;
                foundValue = true;
            }
            
            default:
                break;
            }
            if( foundValue )
            {
                pbn->p->val.f = newValue;
                pbn->p->bound_value(false);
                setValue(pbn->p->get_value_f01());
                return;
            }

        }
    }

    if( ! foundValue )
        ParamQuantity::setDisplayValueString(s);
}

   
    
