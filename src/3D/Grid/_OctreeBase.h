#ifndef OpenKAI_src_3D_Grid__OctreeBase_H_
#define OpenKAI_src_3D_Grid__OctreeBase_H_

#include "../_GeometryBase.h"

#define N_OCT 8

namespace kai
{
	template <typename T>
	struct OCTREE_CELL
	{
		T *m_pT = nullptr;
		OCTREE_CELL *m_pParent = nullptr;
		OCTREE_CELL *m_pChild[N_OCT] = {};
		// Child cells indexed by 3 bits: 4bX+2bY+bZ, bX,bY,bZ: 1: negative half / 1:positive half;

		OCTREE_CELL *addChild(uint8_t iC)
		{
			IF_N(iC < 0 || iC >= N_OCT);

			OCTREE_CELL *pC = m_pChild[iC];
			IF__(pC, pC);

			pC = new OCTREE_CELL();
			pC->m_pParent = this;
			m_pChild[iC] = pC;

			return pC;
		}

		OCTREE_CELL *getChild(uint8_t iC)
		{
			IF_N(iC < 0 || iC >= N_OCT);

			return m_pChild[iC];
		}

		void deleteChild(uint8_t iC)
		{
			IF_(iC < 0 || iC >= N_OCT);

			OCTREE_CELL* pC = m_pChild[iC];
			NULL_(pC);

			pC->release();
			delete pC;
			m_pChild[iC] = nullptr;
		}

		int getLevel(void)
		{
			int L = 0;
			const OCTREE_CELL *pC = m_pParent;
			while (pC)
			{
				L++;
				pC = pC->m_pParent;
			}

			return L;
		}

		T *addT(void)
		{
			if (m_pT == nullptr)
				m_pT = new T();

			return m_pT;
		}

		T *getT(void)
		{
			return m_pT;
		}

		void release(void)
		{
			DEL(m_pT);
			m_pParent = nullptr;

			for (int i = 0; i < N_OCT; i++)
			{
				OCTREE_CELL *pC = m_pChild[i];
				IF_CONT(pC == nullptr);

				pC->release();
				delete pC;
				m_pChild[i] = nullptr;
			}
		}
	};

	class _OctreeBase : public _GeometryBase
	{
	public:
		_OctreeBase();
		virtual ~_OctreeBase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

	protected:
		virtual void updateOctree(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OctreeBase *)This)->update();
			return NULL;
		}

	protected:
	};

}
#endif
