#ifndef OpenKAI_src_3D_Grid__OctreeBase_H_
#define OpenKAI_src_3D_Grid__OctreeBase_H_

#include "../_GeometryBase.h"

#define N_OCT 8

//#define octreeCidx(x, y, z) ((uint8_t)((((int8_t)x >> 7) << 2) | (((int8_t)y >> 7) << 1) | ((int8_t)z >> 7)))

namespace kai
{
	union UUID128
	{
		uint8_t m_uint8[16];
		uint16_t m_uint16[8];
		uint32_t m_uint32[4];
		uint64_t m_uint64[2];
	};

	template <typename T>
	struct OCTREE_CELL
	{
//		UUID128 m_ID = {0, 0};	// for later expansion, ignore this at the moment
		T *m_pT = nullptr;
		OCTREE_CELL *m_pParent = nullptr;
		OCTREE_CELL *m_pChild[N_OCT] = {};
		// Child cells indexed by 3 bits: 4bX+2bY+bZ, bX,bY,bZ: 1: negative half / 1:positive half;

		bool addChild(uint8_t iC)
		{
			IF_F(iC < 0 || iC >= N_OCT);
			IF__(m_pChild[iC], true);

			OCTREE_CELL *pC = new OCTREE_CELL();
			pC->m_pParent = this;
			m_pChild[iC] = pC;

			return true;
		}

		OCTREE_CELL *getChild(uint8_t iC)
		{
			IF_N(iC < 0 || iC >= N_OCT);

			return m_pChild[iC];
		}

		// OCTREE_CELL *getChild(const UUID128& ID)
		// {
		// 	// Ignore this function at the moment, for later expansion
		// 	int iC;

		// 	return m_pChild[iC];
		// }

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
//			m_ID = {0, 0};
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

		// config
		// virtual bool loadConfig(json *pJ = nullptr, string fName = "");
		// virtual bool saveConfig(json &j, string fName = "");

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
