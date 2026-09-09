/*
 * _PointCloud.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud__PointCloud_H_
#define OpenKAI_src_3D_PointCloud__PointCloud_H_

#include "../_GeometryBase.h"
#include "../../Protocol/_JSONbase.h"

namespace kai
{
    class _PointCloud : public _GeometryBase
    {
    public:
        _PointCloud();
        virtual ~_PointCloud();

        // BASE
        virtual bool init(const json &j);
        virtual bool start(void);
        virtual bool check(void);
        virtual void console(void *pConsole);
        virtual void console(const json &j, void *pJSONbase);

        // _GeometryBase
        virtual void clear(void);
        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);

        // data io
        virtual void add(const Vector3d &vP, const Vector3f &vC, uint64_t tStamp = 1);
        virtual void add(const vFloat3 &vP, const vFloat3 &vC, uint64_t tStamp = 1);
        virtual void add(const vFloat3 &vP, const vFloat4 &vC, uint64_t tStamp = 1);

    protected:
        virtual int copy(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pIn, GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);
        virtual GEOMETRY_RINGBUF<GEOMETRY_POINT>* getRingBuf(void);

    private:
        void updatePointCloud(void);
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_PointCloud *)This)->update();
            return NULL;
        }

    protected:
        GEOMETRY_RINGBUF<GEOMETRY_POINT> m_grPt;
    };

}
#endif
