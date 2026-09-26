/*
 * _PointCloud.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud__PointCloud_H_
#define OpenKAI_src_Universe_Geometry_PointCloud__PointCloud_H_

#include "../_GeometryBase.h"
#include "../../../Protocol/_JSONbase.h"
#include <mutex>

namespace kai
{
    struct PCL_FRAME
    {
        int m_iPfrom = 0;
        int m_iPto = 0;
        int m_nP = 0;
        uint64_t m_tStamp = 0;

        void clear(void)
        {
            m_iPfrom = 0;
            m_iPto = 0;
            m_nP = 0;
            m_tStamp = 0;
        }

        void start(int iP)
        {
            m_iPfrom = iP;
            m_iPto = 0;
            m_nP = 0;
            m_tStamp = 0;
        }

        void stop(int iP, int nPtot, uint64_t tStamp)
        {
            m_iPto = iP;
            m_tStamp = tStamp;

            // Equal indices can also describe a completely full ring.
            if (m_nP == nPtot)
                return;

            if(m_iPto >= m_iPfrom)
                m_nP = m_iPto - m_iPfrom;
            else
                m_nP = m_iPto + (nPtot - m_iPfrom);
        }
    };

    class _PointCloud : public _GeometryBase
    {
    public:
        _PointCloud();
        virtual ~_PointCloud();

        // BASE
        virtual bool loadConfig(void) override;
        bool saveConfig(bool bExport) override;
        virtual bool start(void);
        virtual bool check(void);
        virtual void console(void *pConsole);
        virtual void console(const json &j, void *pJSONbase);

        // _GeometryBase
        virtual void clear(void);
        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);

        // data io
        virtual void add(const Vector3f &vP, const Vector3f &vC, uint64_t tStamp = 1);

        // Frames refer to spans in m_grPt; overwritten completed frames expire.
        virtual void frameStart(void);
        virtual void frameStop(void);
        int getLastFrame(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t& tStamp) override;
        // Check freshness under the ring lock before copying any points.
        int getLastFrameIfNew(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t &tStamp, uint64_t afterStamp);
        // Atomically replace the ring and completed-frame indices (e.g. a SLAM map).
        void setFrame(const vector<Vector3f> &points, const vector<Vector3f> &colors, uint64_t stamp);

    protected:
        virtual int copy(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pIn, GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);
        virtual GEOMETRY_RINGBUF<GEOMETRY_POINT> *getRingBuf(void);

    private:
        int copyLastFrameLocked(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t &tStamp);
        void updatePointCloud(void);
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_PointCloud *)This)->update();
            return NULL;
        }

    protected:
        std::mutex m_mtxPt;
        GEOMETRY_RINGBUF<GEOMETRY_POINT> m_grPt;
        uint64_t m_tStamp = 0;  // last point added tStamp

        std::mutex m_mtxFrame;
        PCL_FRAME m_framing;
        PCL_FRAME m_framed;
        bool m_bFraming = false;
    };

}
#endif
