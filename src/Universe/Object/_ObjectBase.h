/*
 * _ObjectBase.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Object__ObjectBase_H_
#define OpenKAI_src_Universe_Object__ObjectBase_H_

#include "../_ReferenceFrame.h"

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

	class _ObjectBase : public _ModuleBase
	{
	public:
		_ObjectBase();
		virtual ~_ObjectBase();

		// type
		void setType(OBJ_TYPE type);
		OBJ_TYPE getType(void);

		// dimension
		void setDim(const Vector3f &vD);

		// classification
		void addClass(int16_t iClass, int8_t prob = 100);
		int getTopClass(void);
		bool bClass(int16_t iClass);
		void clearClass(void);

	protected:
		OBJ_TYPE m_type = obj_unknown;

		Vector3f m_vDim = Vector3f::Zero();	// w,h,d

		vector<OBJ_CLASS> m_vClass;

		uint64_t m_tStamp = 0;

	};

}
#endif
