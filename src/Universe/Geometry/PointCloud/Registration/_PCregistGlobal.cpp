/*
 * _PCregistGlobal.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCregistGlobal.h"

namespace kai
{

    _PCregistGlobal::_PCregistGlobal()
    {
    }

    _PCregistGlobal::~_PCregistGlobal()
    {
    }

    bool _PCregistGlobal::loadConfig(void)
    {
        IF_F(!this->_ModuleBase::loadConfig());
        const json &j = *m_pJ;

        jKv(j, "rNormal", m_rNormal);
        jKv(j, "rFeature", m_rFeature);
        jKv(j, "maxNNnormal", m_maxNNnormal);
        jKv(j, "maxNNfpfh", m_maxNNfpfh);

        return true;
    }

    bool _PCregistGlobal::saveConfig(bool bExport)
    {
        IF_F(!_ModuleBase::saveConfig(false));

        json &j = *m_pJ;
        j["rNormal"] = m_rNormal;
        j["rFeature"] = m_rFeature;
        j["maxNNnormal"] = m_maxNNnormal;
        j["maxNNfpfh"] = m_maxNNfpfh;

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _PCregistGlobal::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        const json &j = *m_pJ;

        string n;

        n = "";
        jKv(j, "_PointCloudSrc", n);
        m_pSrc = (_PointCloud *)(m_pM->findModule(n));
        IF_Le_F(!m_pSrc, "_PCbaseSrc not found: " + n);

        n = "";
        jKv(j, "_PointCloudTgt", n);
        m_pTgt = (_PointCloud *)(m_pM->findModule(n));
        IF_Le_F(!m_pTgt, "_PCbaseTgt not found: " + n);

        n = "";
        jKv(j, "_PCtransform", n);
        m_pTf = (_PCtransform *)(m_pM->findModule(n));
        IF_Le_F(!m_pTf, "_PCtransform not found: " + n);

        return true;
    }

    bool _PCregistGlobal::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    bool _PCregistGlobal::check(void)
    {
        NULL_F(m_pSrc);
        NULL_F(m_pTgt);
        NULL_F(m_pTf);

        return _ModuleBase::check();
    }

    void _PCregistGlobal::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            updateRegistration();
        }
    }

    void _PCregistGlobal::updateRegistration(void)
    {
        IF_(!check());

        PointCloud pcSrc;
        m_pSrc->copyTo(&pcSrc);
        PointCloud pcTgt;
        m_pTgt->copyTo(&pcTgt);

        IF_(pcSrc.IsEmpty());
        IF_(pcTgt.IsEmpty());

        Feature spFpfhSrc = *preprocess(pcSrc);
        Feature spFpfhTgt = *preprocess(pcTgt);

        // m_RR = FastGlobalRegistration(
        //     pcSrc,
        //     pcTgt,
        //     spFpfhSrc,
        //     spFpfhTgt,
        //     FastGlobalRegistrationOption());

        // IF_(m_RR.fitness_ < m_lastFit);
        // m_lastFit = m_RR.fitness_;

        // m_pTf->setTranslationMatrix(m_RR.transformation_);
    }

    std::shared_ptr<Feature> _PCregistGlobal::preprocess(PointCloud &pc)
    {
        pc.EstimateNormals(KDTreeSearchParamHybrid(m_rNormal, m_maxNNnormal));
        return ComputeFPFHFeature(
            pc,
            KDTreeSearchParamHybrid(m_rFeature, m_maxNNfpfh));
    }

    void _PCregistGlobal::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
        ((_Console *)pConsole)->addMsg("Fitness = " + f2str((float)m_RR.fitness_) + ", Inliner_rmse = " + f2str((float)m_RR.inlier_rmse_));
    }

}
#endif
