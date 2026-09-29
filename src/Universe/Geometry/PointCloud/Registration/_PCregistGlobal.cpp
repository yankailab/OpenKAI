/*
 * _PCregistGlobal.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCregistGlobal.h"
#include "PCLframeToOpen3D.h"

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

    bool _PCregistGlobal::link(InstanceMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(pM));
        const json &j = *m_pJ;

        string n;

        n = "";
        jKv(j, "PCLframeSrc", n);
        m_pSrc = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pSrc, "PCLframeSrc not found: " + n);

        n = "";
        jKv(j, "PCLframeTgt", n);
        m_pTgt = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
        IF_Le_F(!m_pTgt, "PCLframeTgt not found: " + n);

        n = "";
        jKv(j, "_PCtransform", n);
        m_pTf = (_PCtransform *)(pM->findModule(n));
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

        open3d::geometry::PointCloud pcSrc = pclFrameToOpen3D(*m_pSrc);
        open3d::geometry::PointCloud pcTgt = pclFrameToOpen3D(*m_pTgt);

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

    std::shared_ptr<Feature> _PCregistGlobal::preprocess(open3d::geometry::PointCloud &pc)
    {
        pc.EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(m_rNormal, m_maxNNnormal));
        return ComputeFPFHFeature(
            pc,
            open3d::geometry::KDTreeSearchParamHybrid(m_rFeature, m_maxNNfpfh));
    }

    void _PCregistGlobal::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
        ((_Console *)pConsole)->addMsg("Fitness = " + f2str((float)m_RR.fitness_) + ", Inliner_rmse = " + f2str((float)m_RR.inlier_rmse_));
    }

}
#endif
