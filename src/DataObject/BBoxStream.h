/*
 * BBoxStream.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DataStream_ObjStream_H_
#define OpenKAI_src_DataStream_ObjStream_H_

#include "DataObjBase.h"

namespace kai
{
	enum OBJ_TYPE
	{
		obj_unknown = 0,
		obj_bbox = 1,
		obj_tag = 2,
	};

	struct OBJ_CLASS
	{
		int16_t m_iClass = -1;
		int8_t m_prob = 0;
	};

	struct BBOX_OBJ
	{
		OBJ_TYPE m_type = obj_unknown;
		Vector3f m_vPos = Vector3f::Zero();
		Vector3f m_vDim = Vector3f::Zero(); // w,h,d
		vector<OBJ_CLASS> m_vClass;
		uint64_t m_tStamp = 0;

		void setType(OBJ_TYPE type)
		{
			m_type = type;
		}

		OBJ_TYPE getType(void)
		{
			return m_type;
		}

		void setDim(const Vector3f &vD)
		{
			m_vDim = vD;
		}

		void addClass(int16_t iClass, int8_t prob = 100)
		{
			for (OBJ_CLASS &objClass : m_vClass)
			{
				if (objClass.m_iClass == iClass)
				{
					objClass.m_prob = prob;
					return;
				}
			}

			m_vClass.emplace_back();
			OBJ_CLASS &objClass = m_vClass.back();
			objClass.m_iClass = iClass;
			objClass.m_prob = prob;
		}

		int getTopClass(void)
		{
			if (m_vClass.empty())
				return -1;

			const OBJ_CLASS *pTop = &m_vClass.front();
			for (const OBJ_CLASS &objClass : m_vClass)
			{
				if (objClass.m_prob > pTop->m_prob)
					pTop = &objClass;
			}

			return pTop->m_iClass;
		}

		bool bClass(int16_t iClass)
		{
			for (const OBJ_CLASS &objClass : m_vClass)
			{
				if (objClass.m_iClass == iClass)
					return true;
			}

			return false;
		}

		void clear(void)
		{
			m_type = obj_unknown;
			m_vPos = Vector3f::Zero();
			m_vDim = Vector3f::Zero();
			m_vClass.clear();
			m_tStamp = 0;
		}
	};

	class BBoxStream : public DataObjBase
	{
	public:
		BBoxStream();
		virtual ~BBoxStream();

		void set(const vector<BBOX_OBJ>& vSrc, uint64_t tStamp = 0);
		uint64_t get(vector<BBOX_OBJ>& vDest);

	protected:
		vector<BBOX_OBJ> m_vObj;
		int m_nMaxObj;
		Vector3f m_vContainerSize;

		std::shared_mutex m_sMutex;
	};

}
#endif
