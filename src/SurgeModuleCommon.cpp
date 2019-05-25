#include "SurgeModuleCommon.hpp"
#include <string>

void SurgeRackParamBinding::update(const ParamCache &pc, SurgeModuleCommon *m)
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
    
    if( paramChanged || forceRefresh || m->inputConnected(cv_id) )
        p->set_value_f01(m->getParam(param_id) + m->getInput(cv_id) / 10.0);
}

void SurgeModuleCommon::setupSurgeCommon(int NUM_PARAMS) 
{
    std::string dataPath;
    dataPath = rack::asset::plugin(pluginInstance, "surge-data/");
    
    showBuildInfo();
    storage.reset(new SurgeStorage(dataPath));
    
    rack::INFO("[SurgeRack] SurgeStorage::dataPath = %s", storage->datapath.c_str());
    rack::INFO("            SurgeStorage::userDataPath = %s", storage->userDataPath.c_str());
    rack::INFO("            SurgeStorage::wt_list.size() = %d", storage->wt_list.size());
    
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
    }
    rack::INFO( "GETLABEL defaulting for %s", label.c_str() );
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
