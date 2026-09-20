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
        m_framing.clear();
        m_framed.clear();
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

        m_tStamp = tStamp;
    }

    void _PointCloud::frameStart(void)
    {
        m_framing.start(m_grPt.m_iT);
    }

    void _PointCloud::frameStop(void)
    {
        std::lock_guard<std::mutex> lock(m_mtxFrame);
        m_framing.stop(m_grPt.m_iT, m_grPt.m_nT, m_tStamp);
        std::swap(m_framing, m_framed);
    }

    int _PointCloud::getLastFrame(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t& tStamp)
    {
        NULL__(pvP, -1);

        int iP;
        int iPto;
        int nP = 0;

        {
            std::lock_guard<std::mutex> lock(m_mtxFrame);

            pvP->clear();
            pvP->reserve(m_framed.m_nP);
            if (pvC)
            {
                pvC->clear();
                pvC->reserve(m_framed.m_nP);
            }

            iP = m_framed.m_iPfrom;
            iPto = m_framed.m_iPto;
            tStamp = m_tStamp;
        }

        while (iP != iPto)
        {
            GEOMETRY_POINT *pGp = m_grPt.get(iP);
            if (!pGp)
                break;

            pvP->push_back(pGp->m_vP);
            if (pvC)
                pvC->push_back(pGp->m_vC);

            nP++;
            if (++iP >= m_grPt.m_nT)
                iP = 0;
        }

        return nP;
    }

    int _PointCloud::copy(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pIn, GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire)
    {
        NULL__(pIn, -1);
        NULL__(pOut, -1);

        int nP = 0;
        int nPin = pIn->nT();
        int iP = pIn->iLastT();

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
