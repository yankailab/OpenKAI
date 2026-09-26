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

    bool _PointCloud::loadConfig(void)
    {
        IF_F(!this->_GeometryBase::loadConfig());
        const json &j = *m_pJ;

        int nP = 1000;
        jKv(j, "nP", nP);
        IF_Le_F(nP <= 0, "Invalid nP: " + i2str(nP));
        IF_Le_F(!m_grPt.alloc(nP), "Alloc faild with nP: " + i2str(nP));

        clear();
        return true;
    }

    bool _PointCloud::saveConfig(bool bExport)
    {
        if (!_GeometryBase::saveConfig(false))
        {
            return false;
        }

        json &j = *m_pJ;
        j["nP"] = m_grPt.m_nT;

        if (!bExport)
        {
            return true;
        }
        return m_pJcfg->saveToFile();
    }

    void _PointCloud::clear(void)
    {
        std::scoped_lock lock(m_mtxPt, m_mtxFrame);
        m_grPt.clear();
        m_framing.clear();
        m_framed.clear();
        m_bFraming = false;
        m_tStamp = 0;
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

        // Do not return mixed old/new points once a completed span is overwritten.
        if (m_framed.m_nP && m_grPt.m_iT == m_framed.m_iPfrom)
            m_framed.clear();

        m_grPt.add(gP);

        if (m_bFraming)
        {
            if (m_framing.m_nP < m_grPt.m_nT)
                ++m_framing.m_nP;
            else
                m_framing.m_iPfrom = m_grPt.m_iT;
            m_framing.m_tStamp = tStamp;
        }

        m_tStamp = tStamp;
    }

    void _PointCloud::frameStart(void)
    {
        std::lock_guard<std::mutex> lock(m_mtxPt);
        m_framing.start(m_grPt.m_iT);
        m_bFraming = true;
    }

    void _PointCloud::frameStop(void)
    {
        std::scoped_lock lock(m_mtxPt, m_mtxFrame);
        IF_(!m_bFraming);
        
        m_framing.stop(m_grPt.m_iT, m_grPt.m_nT, m_framing.m_tStamp);
        std::swap(m_framing, m_framed);
        m_bFraming = false;
    }

    int _PointCloud::getLastFrame(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t& tStamp)
    {
        NULL__(pvP, -1);

        // Keep the completed-frame indices and ring storage stable while copying.
        std::scoped_lock lock(m_mtxPt, m_mtxFrame);
        return copyLastFrameLocked(pvP, pvC, tStamp);
    }

    int _PointCloud::getLastFrameIfNew(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t &tStamp, uint64_t afterStamp)
    {
        NULL__(pvP, -1);
        std::scoped_lock lock(m_mtxPt, m_mtxFrame);
        if (m_framed.m_tStamp <= afterStamp)
        {
            pvP->clear();
            if (pvC) pvC->clear();
            tStamp = m_framed.m_tStamp;
            return 0;
        }
        
        return copyLastFrameLocked(pvP, pvC, tStamp);
    }

    int _PointCloud::copyLastFrameLocked(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t &tStamp)
    {
        pvP->clear();
        pvP->reserve(m_framed.m_nP);
        if (pvC)
        {
            pvC->clear();
            pvC->reserve(m_framed.m_nP);
        }
        tStamp = m_framed.m_tStamp;

        int iP = m_framed.m_iPfrom;
        for (int nP = 0; nP < m_framed.m_nP; ++nP)
        {
            const GEOMETRY_POINT *point = m_grPt.get(iP);
            if (!point) break;
            pvP->push_back(point->m_vP);
            if (pvC) pvC->push_back(point->m_vC);
            if (++iP >= m_grPt.m_nT) iP = 0;
        }
        
        return int(pvP->size());
    }

    void _PointCloud::setFrame(const vector<Vector3f> &points, const vector<Vector3f> &colors, uint64_t stamp)
    {
        std::scoped_lock lock(m_mtxPt, m_mtxFrame);
        m_grPt.m_iT = 0;
        m_framing.clear();
        m_framed.start(m_grPt.m_iT);
        
        // Like add(), an oversized frame retains only the newest nP points.
        const size_t count = std::min(points.size(), size_t(m_grPt.m_nT));
        // get() walks backwards from the newest point and stops at timestamp 0.
        // A single invalid slot before this span replaces clearing the entire
        // capacity; unused slots are never visited. A full span overwrites all.

        if (count < size_t(m_grPt.m_nT))
            m_grPt.m_pT[m_grPt.m_nT - 1].m_tStamp = 0;
        // SLAM maps are already in world coordinates. Avoid an Eigen transform
        // per point for the common identity pose, without dropping small poses.
        
        const bool identityPose = m_mPosef.matrix().isIdentity(0.0f);
        const size_t first = points.size() - count;
        for (size_t destination = 0; destination < count; ++destination)
        {
            const size_t i = first + destination;
            GEOMETRY_POINT &point = m_grPt.m_pT[destination];
            if (identityPose) point.m_vP = points[i];
            else point.m_vP = m_mPosef * points[i];
            point.m_vC = i < colors.size() ? colors[i] : Vector3f::Ones();
            point.m_tStamp = stamp;
        }
        
        m_grPt.m_iT = count == size_t(m_grPt.m_nT) ? 0 : int(count);
        m_framed.m_nP = int(count);
        m_framed.stop(m_grPt.m_iT, m_grPt.m_nT, stamp);
        m_tStamp = stamp;
        m_bFraming = false;
    }

    int _PointCloud::copy(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pIn, GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire)
    {
        NULL__(pIn, -1);
        NULL__(pOut, -1);

        int nP = 0;
        int nPin = pIn->nT();
        // The newest entry precedes the next-write index, including index 0.
        int iP = pIn->iDec(pIn->m_iT);

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
