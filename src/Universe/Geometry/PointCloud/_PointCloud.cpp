/*
 * _PointCloud.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PointCloud.h"

namespace kai
{

    _PointCloud::_PointCloud()
    {
        m_type = geometry_pointCloud;
    }

    _PointCloud::~_PointCloud()
    {
        m_grPt.release();
    }

    bool _PointCloud::init(const json &j)
    {
        IF_F(!this->_GeometryBase::init(j));

        int nP = 1000;
        jKv(j, "nP", nP);
        IF_Le_F(nP <= 0, "Invalid nP: " + i2str(nP));
        IF_Le_F(!m_grPt.alloc(nP), "Alloc faild with nP: " + i2str(nP));

        clear();
        return true;
    }

    void _PointCloud::clear(void)
    {
        std::lock_guard<std::mutex> lock(m_mtxPt);
        m_grPt.clear();
        m_bFrame = false;
        m_vFrameBuilding.clear();
        m_vFrameLast.clear();
        m_tStampFrame = 0;
    }

    bool _PointCloud::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    bool _PointCloud::check(void)
    {
        return this->_GeometryBase::check();
    }

    void _PointCloud::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            updatePointCloud();
        }
    }

    void _PointCloud::updatePointCloud(void)
    {
        IF_(!check());
    }

    int _PointCloud::get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire)
    {
        std::lock_guard<std::mutex> lock(m_mtxPt);
        return copy(&m_grPt, pOut, tExpire);
    }

    void _PointCloud::add(const Vector3f &vP, const Vector3f &vC, uint64_t tStamp)
    {
        std::lock_guard<std::mutex> lock(m_mtxPt);
        GEOMETRY_POINT gP;
        gP.m_vP = m_mPosef * vP;
        gP.m_vC = vC;
        gP.m_tStamp = tStamp;

        m_grPt.add(gP);
        if (m_bFrame)
            m_vFrameBuilding.push_back(gP);
    }

    void _PointCloud::frameStart(void)
    {
        std::lock_guard<std::mutex> lock(m_mtxPt);
        m_vFrameBuilding.clear();
        m_bFrame = true;
    }

    void _PointCloud::frameStop(void)
    {
        std::lock_guard<std::mutex> lock(m_mtxPt);
        IF_(!m_bFrame);
        m_bFrame = false;
        m_vFrameLast.swap(m_vFrameBuilding);
        m_tStampFrame = m_vFrameLast.empty() ? 0 : m_vFrameLast.front().m_tStamp;
    }

    int _PointCloud::getLastFrame(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t &tStamp)
    {
        std::lock_guard<std::mutex> lock(m_mtxPt);
        tStamp = m_tStampFrame;
        NULL__(pvP, -1);
        pvP->clear();
        pvP->reserve(m_vFrameLast.size());
        if (pvC)
        {
            pvC->clear();
            pvC->reserve(m_vFrameLast.size());
        }
        for (const auto &point : m_vFrameLast)
        {
            pvP->push_back(point.m_vP);
            if (pvC)
                pvC->push_back(point.m_vC);
        }
        return static_cast<int>(m_vFrameLast.size());
    }

    int _PointCloud::copy(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pIn, GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire)
    {
        NULL__(pIn, 0);
        NULL__(pOut, 0);

        int nP = 0;
        int nPin = pIn->nT();
        int iP = pIn->iT();

        while (nP < nPin)
        {
            GEOMETRY_POINT *pGp = pIn->get(iP);
            if (!pGp)
                break;
            if (bExpired(pGp->m_tStamp, tExpire))
                break;

            pOut->add(*pGp);
            nP++;

            iP = pIn->iDec(iP);
        }

        return nP;
    }

    GEOMETRY_RINGBUF<GEOMETRY_POINT> *_PointCloud::getRingBuf(void)
    {
        return &m_grPt;
    }

    void _PointCloud::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_GeometryBase::console(pConsole);
    }

    void _PointCloud::console(const json &j, void *pJSONbase)
    {
        // _JSONbase *pJb = (_JSONbase *)pJSONbase;
        string cmd;
        IF_(!jKv(j, "cmd", cmd));

        // if (cmd == "savePly")
        // {
        //     string fPly;
        //     IF_(!jKv(j, "fNamePly", fPly));

        //     bool bR = saveFile(fPly);

        //     NULL_(pJb);
        //     json jr = json::object();
        //     jr["cmd"] = "savePly";
        //     jr["bSuccess"] = bR;
        //     pJb->sendJson(jr);
        // }
    }

}

