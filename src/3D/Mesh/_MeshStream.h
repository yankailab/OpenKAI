/*
 * _MeshStream.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_Mesh__MeshStream_H_
#define OpenKAI_src_3D_Mesh__MeshStream_H_

#include "../_GeometryBase.h"

namespace kai
{
    class _MeshStream : public _GeometryBase
    {
    public:
        _MeshStream();
        virtual ~_MeshStream();

		virtual bool init(const json& j);
        virtual bool check(void);

        virtual void setAccept(bool b);
        virtual int nP(void);
        virtual int iP(void);

        virtual int addVertex(const Vector3d &vP, const uint64_t tStamp);
        virtual int addTriangle(const vInt3& vVertices, const uint64_t tStamp);

        virtual void startStream(void);
        virtual void stopStream(void);
        virtual void clear(void);

    protected:
        virtual void getStream(void *p);
        virtual void getNextFrame(void *p);
        virtual void getLattice(void *p);

    protected:
        GEOMETRY_POINT *m_pP;
        int m_nP;
        int m_iP;
        uint64_t m_tLastUpdate;
        bool m_bAccept;

    };

}
#endif
