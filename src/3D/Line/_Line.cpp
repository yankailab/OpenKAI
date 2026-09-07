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

    bool _Line::init(const json &j)
    {
        IF_F(!this->_GeometryBase::init(j));

        int nL = 1000;
        jKv(j, "nL", nL);
        IF_Le_F(nL <= 0, "Invalid nL: " + i2str(nL));
        IF_Le_F(!m_grLn.alloc(nL), "Alloc faild with nL: " + i2str(nL));

        clear();
        return true;
    }

    void _Line::clear(void)
    {
        atomicFrom();

        m_grLn.clear();

        atomicTo();
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
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            updateLine();
        }
    }

    void _Line::updateLine(void)
    {
        IF_(!check());
    }

    void _Line::add(const Vector3d &vPa, const Vector3d &vPb, const Vector3f &vC, uint8_t width, uint64_t tStamp)
    {
        add(e2v((Vector3f)vPa.cast<float>()),
            e2v((Vector3f)vPb.cast<float>()),
            e2v((Vector3f)vC.cast<float>()),
            width,
            tStamp);
    }

    void _Line::add(const vFloat3 &vPa, const vFloat3 &vPb, const vFloat3 &vC, uint8_t width, uint64_t tStamp)
    {
        GEOMETRY_LINE gL;
        gL.m_vPa = vPa;
        gL.m_vPb = vPb;
        gL.m_vC = vC;
        gL.m_width = width;
        gL.m_tStamp = tStamp;

        m_grLn.add(gL);
    }

    int _Line::add(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pGrLin, uint64_t dTexpire)
    {
        NULL__(pGrLin, 0);

        int nL = 0;
        int i = 0;
        GEOMETRY_LINE *pGl = nullptr;

        atomicFrom();

        uint64_t tNow = getApproxTbootUs();
        while (pGl = pGrLin->get(i++))
        {
            IF_CONT(pGl->m_tStamp == 0);
            IF_CONT(dTexpire > 0 && bExpired(pGl->m_tStamp, dTexpire, tNow));

            m_grLn.add(*pGl);
            nL++;
        }

        atomicTo();

        return nL;
    }

    int _Line::get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pGrLout, uint64_t dTexpire)
    {
        NULL__(pGrLout, 0);

        int nL = 0;
        int i = 0;
        GEOMETRY_LINE *pGl = nullptr;

        atomicFrom();

        uint64_t tNow = getApproxTbootUs();
        while (pGl = m_grLn.get(i++))
        {
            IF_CONT(pGl->m_tStamp == 0);
            IF_CONT(dTexpire > 0 && bExpired(pGl->m_tStamp, dTexpire, tNow));

            pGrLout->add(*pGl);
            nL++;
        }

        atomicTo();
        return nL;
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
        _JSONbase *pJb = (_JSONbase *)pJSONbase;
        string cmd;
        IF_(!jKv(j, "cmd", cmd));

    }

}
