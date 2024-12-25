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
        m_rNormal = 0.2;
        m_rFeature = 0.5;
        m_maxNNnormal = 30;
        m_maxNNfpfh = 100;
        m_lastFit = 0.0;

        m_pSrc = nullptr;
        m_pTgt = nullptr;
        m_pTf = nullptr;
    }

    _PCregistGlobal::~_PCregistGlobal()
    {
    }

    int _PCregistGlobal::init(void *pKiss)
    {
        CHECK_(_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

        pK->v("rNormal", &m_rNormal);
        pK->v("rFeature", &m_rFeature);
        pK->v("maxNNnormal", &m_maxNNnormal);
        pK->v("maxNNfpfh", &m_maxNNfpfh);

        string n;

        n = "";
        pK->v("_PCbaseSrc", &n);
        m_pSrc = (_PCframe *)(pK->findModule(n));
        if(!m_pSrc)
        {
            LOG_E(n + ": not found");
            return OK_ERR_NOT_FOUND;
        }

        n = "";
        pK->v("_PCbaseTgt", &n);
        m_pTgt = (_PCframe *)(pK->findModule(n));
        if(!m_pTgt)
        {
            LOG_E(n + ": not found");
            return OK_ERR_NOT_FOUND;
        }

        n = "";
        pK->v("_PCtransform", &n);
        m_pTf = (_PCtransform *)(pK->findModule(n));
        if(!m_pTf)
        {
            LOG_E(n + ": not found");
            return OK_ERR_NOT_FOUND;
        }

        return true;
    }

    int _PCregistGlobal::start(void)
    {
        NULL__(m_pT, OK_ERR_NULLPTR);
        return m_pT->start(getUpdate, this);
    }

    int _PCregistGlobal::check(void)
    {
        NULL__(m_pSrc, OK_ERR_NULLPTR);
        NULL__(m_pTgt, OK_ERR_NULLPTR);
        NULL__(m_pTf, OK_ERR_NULLPTR);

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
        IF_(check() != OK_OK);

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

    void _PCregistGlobal::console(void* pConsole)
    {
        NULL_(pConsole);
        this->_ModuleBase::console(pConsole);
        ((_Console*)pConsole)->addMsg("Fitness = " + f2str((float)m_RR.fitness_) +
               ", Inliner_rmse = " + f2str((float)m_RR.inlier_rmse_));
    }

}
#endif
