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
        m_pSrc = nullptr;
        m_pTgt = nullptr;
        m_pTf = nullptr;
        m_lastFit = 0.0;

        m_rNormal = 0.2;
        m_rFeature = 0.5;
        m_maxNNnormal = 30;
        m_maxNNfpfh = 100;
    }

    _PCregistGlobal::~_PCregistGlobal()
    {
    }

    bool _PCregistGlobal::init(const json &j)
    {
        IF_F(!this->_ModuleBase::init(j));

        jVar(j, "rNormal", m_rNormal);
        jVar(j, "rFeature", m_rFeature);
        jVar(j, "maxNNnormal", m_maxNNnormal);
        jVar(j, "maxNNfpfh", m_maxNNfpfh);

        return true;
    }

    bool _PCregistGlobal::link(const json &j, ModuleMgr *pM)
    {
        IF_F(!this->_ModuleBase::link(j, pM));

        string n;

        n = "";
        jVar(j, "_PCbaseSrc", n);
        m_pSrc = (_PCframe *)(pM->findModule(n));
        IF_Le_F(!m_pSrc, "_PCbaseSrc not found: " + n);

        n = "";
        jVar(j, "_PCbaseTgt", n);
        m_pTgt = (_PCframe *)(pM->findModule(n));
        IF_Le_F(!m_pTgt, "_PCbaseTgt not found: " + n);

        n = "";
        jVar(j, "_PCtransform", n);
        m_pTf = (_PCtransform *)(pM->findModule(n));
        IF_Le_F(!m_pTf, "_PCtransform not found: " + n);

        return true;
    }

    bool _PCregistGlobal::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
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
        while (m_pT->bAlive())
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
