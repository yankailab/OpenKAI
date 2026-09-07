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
        vFloat3 m_vC; // color
        uint64_t m_tStamp;

        void clear(void)
        {
            m_vP = 0;
            m_vC = 0;
            m_tStamp = 0; // time stamp, 0: invalid, >= 1 valid
        }
    };

    struct GEOMETRY_LINE
    {
        vFloat3 m_vPa;     // line from
        vFloat3 m_vPb;     // line to
        vFloat3 m_vC;      // color
        uint64_t m_tStamp; // time stamp, 0: invalid, >= 1 valid

        void clear(void)
        {
            m_vPa = 0;
            m_vPb = 0;
            m_vC = 0;
            m_tStamp = 0;
        }
    };

    template <typename T>
    struct GEOMETRY_RINGBUF
    {
        T *m_pP = nullptr;
        int m_nP = 0;
        int m_iP = 0;

        bool alloc(int nP)
        {
            IF_F(nP <= 0);

            m_pP = new T[nP];
            NULL_F(m_pP);

            m_nP = nP;
            return true;
        }

        void release(void)
        {
            DEL(m_pP);

            m_nP = 0;
            m_iP = 0;
        }

        void clear(void)
        {
            NULL_(m_pP);
            IF_(m_nP <= 0);

            m_iP = 0;
            for (int i = 0; i < m_nP; i++)
                m_pP[i].clear();
        }

        void add(const T &p)
        {
            NULL_(m_pP);
            IF_(m_nP <= m_iP);

            m_pP[m_iP] = p;
            iInc();
        }

        void iInc(void)
        {
            if (++m_iP >= m_nP)
                m_iP = 0;
        }

        T *get(int i)
        {
            NULL_N(m_pP);
            IF_N(m_nP <= i);

            return &m_pP[i];
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

        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pGrPout, uint64_t dTexpire = 0);
        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pGrLOut, uint64_t dTexpire = 0);

    protected:
        GEOMETRY_TYPE m_type;
    };

}
#endif
