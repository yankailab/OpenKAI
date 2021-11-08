/*
 * PCstream.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PCstream_H_
#define OpenKAI_src_PointCloud__PCstream_H_
#ifdef USE_OPEN3D

#include "_PCbase.h"

namespace kai
{
    inline int iInc(int i, int n)
    {
        i++;
        if (i >= n)
            i = 0;

        return i;
    }

    class _PCstream : public _PCbase
    {
    public:
        _PCstream();
        virtual ~_PCstream();

        virtual bool init(void *pKiss);
        virtual int check(void);

        virtual void AcceptAdd(bool b);
        virtual void add(const Vector3d &vP, const Vector3f &vC, uint64_t tStamp = UINT64_MAX);
        virtual int nP(void);
        virtual int iP(void);

        virtual void clear(void);
        virtual void refreshCol(void);

        virtual void startStream(void);
        virtual void stopStream(void);

    protected:
        virtual void getStream(void *p);
        virtual void getNextFrame(void *p);
        virtual void getLattice(void *p);

    public:
        //ring buf
        PC_POINT *m_pP;
        int m_nP;
        int m_iP;
        uint64_t m_tLastUpdate;
        bool m_bAccept;

        //pipeline input src

    };

}
#endif
#endif
