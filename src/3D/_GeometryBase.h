/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud__GeometryBase_H_
#define OpenKAI_src_3D_PointCloud__GeometryBase_H_

#include "../Base/_ModuleBase.h"
#include "../IPC/SharedMem.h"
#include "../Utility/util.h"
#include "../Utility/utilTime.h"
#include "../UI/_Console.h"

using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;
using namespace Eigen;

namespace kai
{
    enum GEOMETRY_TYPE
    {
        geometry_unknown = -1,
        pc_stream = 0,
        pc_frame = 1,
        pc_grid = 2,
        mesh_stream = 4,
    };

    struct GEOMETRY_POINT
    {
        vFloat3 m_vP; //pos
        vFloat3 m_vC; //color
        uint64_t m_tStamp;

        void clear(void)
        {
            m_vP = 0;
            m_vC = 0;
            m_tStamp = 0;
        }
    };

    class _GeometryBase : public _ModuleBase
    {
    public:
        _GeometryBase();
        virtual ~_GeometryBase();

        virtual bool init(void *pKiss);
		virtual bool link(void);
        virtual int check(void);
        virtual void console(void *pConsole);

        virtual void setTranslation(const vDouble3 &vT);
        virtual void setRotation(const vDouble3 &vR);
        virtual void updateTranslationMatrix(void);
        virtual void setTranslationMatrix(const Matrix4d &mT);
        virtual Matrix4d getTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR);

        virtual GEOMETRY_TYPE getType(void);
		virtual bool initBuffer(void);
        virtual void clear(void);

        virtual void getGeometry(void* p, const uint64_t& tExpire = 0);
        virtual void getPCstream(void* p, const uint64_t& tExpire);
        virtual void getPCframe(void* p);
        virtual void getPCgrid(void* p);

        virtual void writeSharedMem(void);
        virtual void readSharedMem(void);

    protected:
        void mutexLock(void);
        void mutexUnlock(void);

    protected:
        GEOMETRY_TYPE m_type;

        vFloat3 m_vDefaultColor;

        // transform
        vDouble3 m_vT;
        vDouble3 m_vR;
    	Matrix4d m_mT;
        Eigen::Affine3d m_A;

        // buffer writing protection
		pthread_mutex_t m_mutex;

        // shared mem for data
        SharedMem* m_pSM;
    };

}
#endif
