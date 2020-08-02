#include "SurgeModuleCommon.hpp"
#include <string>
#include <atomic>

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
        /*
        if( tsbpmLabel && ts_id >= 0 && m->getParam(ts_id) > 0.5 )
        {
            char ntxt[1024];
            snprintf(ntxt, 1024, "%s @ %5.1lf bpm", txt, m->lastBPM );
            strcpy(txt, ntxt);
        }
        */
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

    std::atomic<bool> showedPathsOnce(false);
    if( ! showedPathsOnce )
    {
        showedPathsOnce = true;
        INFO("[SurgeRack] storage::dataPath = '%s'", storage->datapath.c_str());
        INFO("[SurgeRack] storage::userDataPath = '%s'", storage->userDataPath.c_str());
    }

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
        if( pbn )
        {
            std::cout << "getDisplayValueString " << pbn->valCache.value << std::endl;
            return pbn->valCache.value;
        }
    }

    return ParamQuantity::getDisplayValueString();
}

void SurgeRackParamQuantity::setDisplayValueString(std::string s) {
    SurgeModuleCommon *mc = static_cast<SurgeModuleCommon *>(module);
    //std::cout << "SRPQ for " << paramId << std::endl;
    if( mc )
    {
        auto pbn = mc->pb[paramId];
        //std::cout << "PBN is " << pbn.get() << std::endl;
        if( pbn )
        {
            auto p = pbn->p;
            //std::cout << "p is " << p << std::endl;
            if( p->can_setvalue_from_string() )
            {
                p->set_value_from_string(s);
                setValue(pbn->p->get_value_f01());
                return;
            }

        }
    }

    ParamQuantity::setDisplayValueString(s);
}

   
    
