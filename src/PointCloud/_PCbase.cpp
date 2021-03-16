/*
 * _PCbase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCbase.h"

namespace kai
{

    _PCbase::_PCbase()
    {
        m_pPCB = NULL;
        m_iPr = 0;

        m_ring.init();

        m_bTransform = false;
        m_vT.init(0);
        m_vR.init(0);
        m_A = Eigen::Matrix4d::Identity();

        m_fParam = "";
    }

    _PCbase::~_PCbase()
    {
        m_ring.release();
    }

    bool _PCbase::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        int nP = 0;
        pK->v("nP", &nP);
        IF_F(!m_ring.setup(nP));

        string n;
        n = "";
        pK->v("_PCbase", &n);
        m_pPCB = (_PCbase *)(pK->getInst(n));

        //transform
        pK->v("bTransform", &m_bTransform);
        pK->v("vT", &m_vT);
        pK->v("vR", &m_vR);

        //read from external kiss file if there is one
        pK->v("fParam", &m_fParam);
        IF_T(m_fParam.empty());

        _File *pFile = new _File();
        IF_T(!pFile->open(&m_fParam));

        string fn;
        pFile->readAll(&fn);
        IF_T(fn.empty());

        Kiss *pKf = new Kiss();
        if (pKf->parse(&fn))
        {
            pK = pKf->child("transform");
            pK->v("vT", &m_vT);
            pK->v("vR", &m_vR);
        }

        delete pKf;
        pFile->close();

        return true;
    }

    int _PCbase::check(void)
    {
        return this->_ModuleBase::check();
    }

    void _PCbase::add(Eigen::Vector3d &vP, Eigen::Vector3d &vC, uint64_t tStamp)
    {
        Vector3d p = m_A * vP;
        m_ring.add(p, vC, tStamp);
    }

    void _PCbase::setTranslation(vDouble3 &vT, vDouble3 &vR)
    {
        m_vT = vT;
        m_vR = vR;

        Eigen::Matrix4d mT;
        Eigen::Vector3d eR(m_vR.x, m_vR.y, m_vR.z);
        mT.block(0, 0, 3, 3) = Geometry3D::GetRotationMatrixFromXYZ(eR);
        mT(0, 3) = m_vT.x;
        mT(1, 3) = m_vT.y;
        mT(2, 3) = m_vT.z;

        m_A = mT;
    }

    PC_RING *_PCbase::getRing(void)
    {
        return &m_ring;
    }

    void _PCbase::readSrc(void)
    {
        NULL_(m_pPCB);
        m_ring.readSrc(m_pPCB->getRing(), &m_iPr);
    }

    void _PCbase::draw(void)
    {
        this->_ModuleBase::draw();
    }

    void _PCbase::saveParam(void)
    {
        IF_(m_fParam.empty());

        picojson::object o;
        o.insert(make_pair("name", "transform"));

        picojson::array vT;
        vT.push_back(value(m_vT.x));
        vT.push_back(value(m_vT.y));
        vT.push_back(value(m_vT.z));
        o.insert(make_pair("vT", value(vT)));

        picojson::array vR;
        vR.push_back(value(m_vR.x));
        vR.push_back(value(m_vR.y));
        vR.push_back(value(m_vR.z));
        o.insert(make_pair("vR", value(vR)));

        string k = picojson::value(o).serialize();

        _File *pFile = new _File();
        IF_(!pFile->open(&m_fParam, ios::out));
        pFile->write((uint8_t *)k.c_str(), k.length());
        pFile->close();
    }

}
#endif
