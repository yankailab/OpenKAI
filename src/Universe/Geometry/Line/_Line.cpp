/*
 * _Line.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_Line.h"

namespace kai
{

    _Line::_Line()
    {
        m_type = geometry_line;
    }

    _Line::~_Line()
    {
        m_grLn.release();
    }

    bool _Line::loadConfig(void)
    {
        IF_F(!this->_GeometryBase::loadConfig());
        const json &j = *m_pJ;

        int nL = 1000;
        jKv(j, "nL", nL);
        IF_Le_F(nL <= 0, "Invalid nL: " + i2str(nL));
        IF_Le_F(!m_grLn.alloc(nL), "Alloc faild with nL: " + i2str(nL));

        clear();
        return true;
    }

    bool _Line::saveConfig(bool bExport)
    {
        IF_F(!_GeometryBase::saveConfig(false));

        json &j = *m_pJ;
        j["nL"] = m_grLn.m_nT;

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    void _Line::clear(void)
    {
		std::lock_guard<std::mutex> lock(m_mtxLn);
        m_grLn.clear();
    }

    bool _Line::start(void)
    {
        NULL_F(m_pT);
        return m_pT->startThread(getUpdate, this);
    }

    bool _Line::check(void)
    {
        return this->_GeometryBase::check();
    }

    void _Line::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPS();

            updateLine();
        }
    }

    void _Line::updateLine(void)
    {
        IF_(!check());
    }

    void _Line::add(const Vector3f &vPa, const Vector3f &vPb, const Vector3f &vC, uint64_t tStamp)
    {
        GEOMETRY_LINE gL;
        gL.m_vPa = m_mPosef * vPa;
        gL.m_vPb = m_mPosef * vPb;
        gL.m_vC = vC;
        gL.m_tStamp = tStamp;

        m_grLn.add(gL);
    }

    int _Line::copy(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pIn, GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire)
    {
        NULL__(pIn, 0);
        NULL__(pOut, 0);

        int nL = 0;
        int nLin = pIn->nT();
        int iL = pIn->iLastT();

        while (nL < nLin)
        {
            GEOMETRY_LINE* pGp = pIn->get(iL);
            if(!pGp)
                break;
            if(bExpired(pGp->m_tStamp, tExpire))
                break;

            pOut->add(*pGp);
            nL++;

            iL = pIn->iDec(iL);
        }

        return nL;
    }

    int _Line::get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire)
    {
        return copy(getRingBuf(), pOut, tExpire);
    }

    GEOMETRY_RINGBUF<GEOMETRY_LINE> *_Line::getRingBuf(void)
    {
        return &m_grLn;
    }

    void _Line::console(void *pConsole)
    {
        NULL_(pConsole);
        this->_GeometryBase::console(pConsole);
    }

    void _Line::console(const json &j, void *pJSONbase)
    {
        // _JSONbase *pJb = (_JSONbase *)pJSONbase;
        string cmd;
        IF_(!jKv(j, "cmd", cmd));

    }

}
