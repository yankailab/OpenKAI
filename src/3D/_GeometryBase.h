/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D__GeometryBase_H_
#define OpenKAI_src_3D__GeometryBase_H_

#include "../Base/_ModuleBase.h"
#include "../Utility/util.h"
#include "../Utility/utilTime.h"
#include "../UI/_Console.h"

using namespace Eigen;

namespace kai
{
    enum GEOMETRY_TYPE
    {
        geometry_unknown = 0,
        geometry_pointCloud = 1,
        geometry_line = 2,
        geometry_mesh = 3,
        geometry_octree = 4,
    };

    struct GEOMETRY_POINT
    {
        vFloat3 m_vP; // pos
        vFloat4 m_vC{0, 0, 0, 1}; // color with alpha; RGB-only sources are opaque
        uint64_t m_tStamp;

        void clear(void)
        {
            m_vP = 0;
            m_vC.clear();
            m_tStamp = 0; // time stamp, 0: invalid, >= 1 valid
        }
    };

    struct GEOMETRY_LINE
    {
        vFloat3 m_vPa;     // line from
        vFloat3 m_vPb;     // line to
        vFloat4 m_vC{0, 0, 0, 1}; // color with alpha; RGB-only sources are opaque
        uint64_t m_tStamp; // time stamp, 0: invalid, >= 1 valid

        void clear(void)
        {
            m_vPa = 0;
            m_vPb = 0;
            m_vC.clear();
            m_tStamp = 0;
        }
    };

    template <typename T>
    struct GEOMETRY_RINGBUF
    {
        T *m_pT = nullptr;
        int m_nT = 0;
        int m_iT = 0;

        bool alloc(int nP)
        {
            IF_F(nP <= 0);

            m_pT = new T[nP];
            NULL_F(m_pT);

            m_nT = nP;
            return true;
        }

        void release(void)
        {
            delete[] m_pT;
            m_pT = nullptr;

            m_nT = 0;
            m_iT = 0;
        }

        void clear(void)
        {
            NULL_(m_pT);
            IF_(m_nT <= 0);

            m_iT = 0;
            for (int i = 0; i < m_nT; i++)
                m_pT[i].clear();
        }

        void add(const T &p)
        {
            NULL_(m_pT);
            IF_(m_nT <= m_iT);

            m_pT[m_iT] = p;
            iInc();
        }

        void iInc(void)
        {
            if (++m_iT >= m_nT)
                m_iT = 0;
        }

        int iT(void)
        {
            IF__(m_iT <= 0, 0);
            return m_iT - 1;
        }

        int nT(void)
        {
            return m_nT;
        }

        int iDec(int i)
        {
            if (--i < 0)
                i = m_nT - 1;

            return i;
        }

        T *get(int i)
        {
            NULL_N(m_pT);
            IF_N(m_nT <= i);

            return &m_pT[i];
        }
    };

    class _GeometryBase : public _ModuleBase
    {
    public:
        _GeometryBase();
        virtual ~_GeometryBase();

        virtual bool init(const json &j);
        virtual bool link(const json &j, ModuleMgr *pM);
        virtual bool check(void);
        virtual void console(void *pConsole);

        virtual bool loadConfig(json *pJ = nullptr, string fName = "");
        virtual bool saveConfig(json &j, string fName = "");

        virtual GEOMETRY_TYPE getType(void);
        virtual void clear(void);

        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);
        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire = 0);

    protected:
        GEOMETRY_TYPE m_type = geometry_unknown;
    };

}
#endif
