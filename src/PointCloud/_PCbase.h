/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PCbase_H_
#define OpenKAI_src_PointCloud__PCbase_H_
#ifdef USE_OPEN3D

#include "../Base/_ModuleBase.h"
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;
using namespace Eigen;

namespace kai
{
    enum PC_TYPE{
        pc_unknown = -1,
        pc_stream = 0,
        pc_frame = 1,
        pc_lattice = 2,
    };

    struct PC_POINT
    {
        Vector3d m_vP; //pos
        Vector3d m_vC; //color
        uint64_t m_tStamp;

        void init(void)
        {
            m_vP = Vector3d(0.0, 0.0, 0.0);
            m_vC = Vector3d(0.0, 0.0, 0.0);
            m_tStamp = 0;
        }
    };

    struct PC_PIPIN_CTX
    {
        void *m_pPCB;

        //stream 2 stream
        int m_iPr;

        //stream 2 frame
        uint64_t m_dT;

        void init(void)
        {
            m_pPCB = NULL;
            m_dT = UINT64_MAX;
            m_iPr = 0;
        }
    };

    class _PCbase : public _ModuleBase
    {
    public:
        _PCbase();
        virtual ~_PCbase();

        virtual bool init(void *pKiss);
        virtual int check(void);
        virtual void draw(void);

        virtual PC_TYPE getType(void);
        virtual void setTranslation(const vDouble3 &vT, const vDouble3 &vR);
        virtual void setTranslation(const Matrix4d &mT);
        virtual void readPC(void* pPC);

        virtual Matrix4d getTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR);
        virtual int nPread(void);

    protected:
        virtual void getStream(void* p);
        virtual void getFrame(void* p);
        virtual void getLattice(void* p);

    protected:
        PC_TYPE m_type;
        int m_nPread;

        //axis index for sensors
        vInt3 m_vAxisIdx;
        vFloat3 m_vAxisK;
        float m_unitK;

        //static offset
        vDouble3 m_vToffset; //translation
        vDouble3 m_vRoffset; //rotation
    	Matrix4d m_mToffset;
        Eigen::Affine3d m_Aoffset;

        //dynamic transform
        vDouble3 m_vT; //translation
        vDouble3 m_vR; //rotation
    	Matrix4d m_mT;
        Eigen::Affine3d m_A;

        //pipeline input
        PC_PIPIN_CTX m_pInCtx;
    };

}
#endif
#endif
