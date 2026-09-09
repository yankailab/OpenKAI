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
        atomicFrom();

        m_grPt.clear();

        atomicTo();
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
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            updatePointCloud();
        }
    }

    void _PointCloud::updatePointCloud(void)
    {
        IF_(!check());
    }

    void _PointCloud::add(const Vector3d &vP, const Vector3f &vC, uint64_t tStamp)
    {
        add(e2v((Vector3f)vP.cast<float>()),
            e2v((Vector3f)vC.cast<float>()),
            tStamp);
    }

    void _PointCloud::add(const vFloat3 &vP, const vFloat3 &vC, uint64_t tStamp)
    {
        add(vP, vFloat4(vC.x, vC.y, vC.z, 1), tStamp);
    }

    void _PointCloud::add(const vFloat3 &vP, const vFloat4 &vC, uint64_t tStamp)
    {
        GEOMETRY_POINT gP;
        gP.m_vP = vP;
        gP.m_vC = vC;
        gP.m_tStamp = tStamp;

        m_grPt.add(gP);
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
            GEOMETRY_POINT* pGp = pIn->get(iP);
            if(!pGp)
                break;
            if(bExpired(pGp->m_tStamp, tExpire))
                break;

            pOut->add(*pGp);
            nP++;

            iP = pIn->iDec(iP);
        }

        return nP;
    }

    int _PointCloud::get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire)
    {
        return copy(getRingBuf(), pOut, tExpire);
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

/*
    bool _PointCloud::saveFile(const string &fName)
    {
        IF_F(fName.empty());

        PointCloud pc;
        this->copyTo(&pc);

        io::WritePointCloudOption par;
        par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
        par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

        return io::WritePointCloudToPLY(fName.c_str(), pc, par);
    }

    void _PointCloud::writeSharedMem(void)
    {
        NULL_(m_pSM);
        IF_(!m_pSM->bOpen());
        IF_(!m_pSM->bWriter());

        int nPw = small<int>(m_nP, m_pSM->nB() / sizeof(GEOMETRY_POINT));

        memcpy(m_pSM->p(), m_pP, nPw * sizeof(GEOMETRY_POINT));
    }

    void _PointCloud::readSharedMem(void)
    {
        NULL_(m_pSM);
        IF_(!m_pSM->bOpen());
        IF_(m_pSM->bWriter());

        //		memcpy(m_pP, m_pSM->p(), m_nP * sizeof(GEOMETRY_POINT));
        GEOMETRY_POINT *pSM = (GEOMETRY_POINT *)m_pSM->p();
        for (int i = 0; i < m_nP; i++)
        {
            GEOMETRY_POINT p = pSM[i];
            Vector3d eV = m_A * v2e(p.m_vP).cast<double>();
            p.m_vP = e2v((Vector3f)eV.cast<float>());

            m_pP[m_iP] = p;
            m_iP = iRing(m_iP, m_nP);
        }
    }

*/
