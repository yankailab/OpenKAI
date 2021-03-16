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
#include "../IO/_File.h"

using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;
using namespace Eigen;

namespace kai
{

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

    struct PC_RING
    {
        PC_POINT *m_pP;
        int m_nP;
        int m_iP;
        uint64_t m_tLastUpdate;

        void init(void)
        {
            m_pP = NULL;
            m_nP = 0;
            m_iP = 0;
            m_tLastUpdate = 0;
        }

        bool setup(int nP)
        {
            IF_F(nP <= 0);

            m_pP = new PC_POINT[m_nP];
            NULL_F(m_pP);
            m_iP = 0;
            m_tLastUpdate = 0;

            for (int i = 0; i < m_nP; i++)
                m_pP[i].init();
        }

        void release(void)
        {
            m_nP = 0;
            DEL(m_pP);
        }

        void add(Vector3d &vP, Vector3d &vC, uint64_t tStamp = UINT64_MAX)
        {
            NULL_(m_pP);

            PC_POINT *pP = &m_pP[m_iP];
            pP->m_vP = vP;
            pP->m_vC = vC;
            pP->m_tStamp = tStamp;

            m_iP = iRing(m_iP, m_nP);
        }

        void get(vector<Vector3d> *pvP, vector<Vector3d> *pvC, uint64_t tFrom = 0)
        {
            NULL_(pvP);
            NULL_(pvC);

            for (int i = 0; i < m_nP; i++)
            {
                PC_POINT *pP = &m_pP[i];
                IF_CONT(pP->m_tStamp < tFrom);

                pvP->push_back(pP->m_vP);
                pvC->push_back(pP->m_vC);
            }
        }

        bool get(Vector3d *peP, Vector3d *peC, int *pI)
        {
            NULL_F(peP);
            NULL_F(peC);
            NULL_F(pI);
            IF_F(*pI < 0);
            IF_F(*pI >= m_nP);

            PC_POINT *pP = &m_pP[*pI];
            *peP = pP->m_vP;
            *peC = pP->m_vC;

            *pI = iRing(*pI, m_nP);
            return true;
        }

        void readSrc(PC_RING *pRB, int *pPr, uint64_t tFrom = 0)
        {
            NULL_(pRB);
            NULL_(m_pP);
            IF_(*pPr >= pRB->m_nP);

            PC_POINT *pP = pRB->m_pP;

            while (*pPr != pRB->m_iP)
            {
                PC_POINT p = pP[*pPr];

                if (p.m_tStamp >= tFrom)
                {
                    m_pP[m_iP] = p;
                    m_iP = iRing(m_iP, m_nP);
                }

                *pPr = iRing(*pPr, pRB->m_nP);
            }
        }

        void readSrc(PC_RING *pRB, uint64_t tFrom = 0)
        {
            NULL_(pRB);
            NULL_(m_pP);

            PC_POINT *pP = pRB->m_pP;

            for (int i = 0; i < pRB->m_nP; i++)
            {
                PC_POINT p = pP[i];
                IF_CONT(p.m_tStamp < tFrom);

                m_pP[m_iP] = p;
                m_iP = iRing(m_iP, m_nP);
            }
        }

        inline int iRing(int i, int n)
        {
            i++;
            if (i >= n)
                i = 0;

            return i;
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

        virtual void add(Eigen::Vector3d &vP, Eigen::Vector3d &vC, uint64_t &tStamp);
        virtual void setTranslation(vDouble3 &vT, vDouble3 &vR);

        virtual PC_RING *getRing(void);
        virtual void readSrc(void);

        virtual void saveParam(void);

    protected:
        //pipeline input src
        _PCbase *m_pPCB;
        int m_iPr;

        //ring buf
        PC_RING m_ring;

        //dynamics
        bool m_bTransform;
        vDouble3 m_vT; //translation
        vDouble3 m_vR; //rotation
        Eigen::Affine3d m_A;

        //param save
        string m_fParam;
    };

}
#endif
#endif
