/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud__GeometryBase_H_
#define OpenKAI_src_3D_PointCloud__GeometryBase_H_

//#include "../Vision/ImgFilter/_Remap.h"
#include "../IO/_File.h"
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;
using namespace Eigen;

namespace kai
{
    enum PC_TYPE
    {
        geometry_unknown = -1,
        pc_stream = 0,
        pc_frame = 1,
        pc_lattice = 2,
        mesh_stream = 3,
    };

    enum PC_SHADE
    {
        pcShade_original = 0,
        pcShade_colOvrr = 1,
        pcShade_colPos = 2,
    };

    struct PC_POINT
    {
        Vector3d m_vP; //pos
        Vector3f m_vC; //color
        uint64_t m_tStamp;

        void init(void)
        {
            m_vP = Vector3d(0,0,0);
            m_vC = Vector3f(0,0,0);
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

    struct MESH_VERTEX
    {
        Vector3d m_vP; //pos
        Vector3f m_vC; //color
        uint64_t m_tStamp;

        void init(void)
        {
            m_vP = Vector3d(0,0,0);
            m_vC = Vector3f(0,0,0);
            m_tStamp = 0;
        }
    };

    struct MESH_TRIANGLE
    {
        Vector3d m_vP; //pos
        Vector3f m_vC; //color
        uint64_t m_tStamp;

        void init(void)
        {
            m_vP = Vector3d(0,0,0);
            m_vC = Vector3f(0,0,0);
            m_tStamp = 0;
        }
    };

    struct MESH_PIPIN_CTX
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


    class _GeometryBase : public _ModuleBase
    {
    public:
        _GeometryBase();
        virtual ~_GeometryBase();

        virtual bool init(void *pKiss);
        virtual int check(void);

        virtual PC_TYPE getType(void);
        virtual void setOffset(const vDouble3 &vT, const vDouble3 &vR);
        virtual void setTranslation(const vDouble3 &vT, const vDouble3 &vR);
        virtual void setTranslation(const Matrix4d &mT);
        virtual void setRGBoffset(const vDouble3 &vT, const vDouble3 &vR);
        virtual void readPC(void* pPC);
        virtual int nPread(void);
        virtual void clear(void);

    protected:
        virtual Matrix4d getTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR);
        virtual void getStream(void* p);
        virtual void getNextFrame(void* p);
        virtual void getLattice(void* p);
        virtual bool bRange(const Vector3d& vP);

        virtual bool getColor(const Vector3d &vP, Vector3f* pvC);

    protected:
        PC_TYPE m_type;
        int m_nPread;

        // axis index for Pos/Atti sensor
        vInt3 m_vAxisIdx;
        vFloat3 m_vAxisK;
        float m_unitK;

        // pos/atti sensor offset in pos/atti sensor coordinate
        vDouble3 m_vToffset;
        vDouble3 m_vRoffset;
    	Matrix4d m_mToffset;
        Eigen::Affine3d m_Aoffset;

        // dynamic transform
        vDouble3 m_vT;
        vDouble3 m_vR;
    	Matrix4d m_mT;
        Eigen::Affine3d m_A;

        // RGB offset in Lidar coordinate
        // _Remap* m_pR;
        // vDouble3 m_vToffsetRGB;
        // vDouble3 m_vRoffsetRGB;
    	// Matrix4d m_mToffsetRGB;
    	// Eigen::Affine3d m_AoffsetRGB;
        // vInt3 m_vAxisIdxRGB;
        // vFloat3 m_vAxisKrgb;

        // filter
        vDouble2 m_vRange;

        // pipeline input
        PC_PIPIN_CTX m_pInCtx;

    };

}
#endif
