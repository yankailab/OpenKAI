/*
 * _Object.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__Object_H_
#define OpenKAI_src_Universe__Object_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"

#ifdef USE_OPENCV
#include "../Utility/utilCV.h"
#endif

namespace kai
{
	enum OBJ_TYPE
	{
		obj_unknown = 0,
		obj_bbox = 1,
		obj_tag = 2,
	};

	class _Object : public _ModuleBase
	{
	public:
		_Object();
		virtual ~_Object();

		//general
		virtual bool init(const json& j);
		virtual void clear(void);
		virtual bool start(void);
		virtual void update(void);

		//pos
		void setPos(const vFloat3 &vP);
		void setPos(float x, float y, float z);
		vFloat3 getPos(void);

		//dimension
		void setDim(const vFloat4 &vD);
		void setDim(float w, float h, float d, float r);
		vFloat4 getDim(void);
		float getDimArea(void);
		float getDimVolume(void);

		//attitude
		void setAttitude(const vFloat3 &vA);
		void setAttitude(float r, float p, float y);
		vFloat3 getAttitude(void);

		//convenient
		void setBB2D(const vFloat4& vBB, float kX = 1.0, float kY = 1.0);
		void setRect(float l, float t, float w, float h, float kX = 1.0, float kY = 1.0);
		vFloat4 getBB2D(float kX = 1.0, float kY = 1.0);

		//vertex
		void setVertices2D(vFloat2 *pV, int nV, float kX = 1.0, float kY = 1.0);
		vFloat2 *getVertex(int i);

		//classification
		void setType(OBJ_TYPE type);
		OBJ_TYPE getType(void);
		void resetClass(void);
		void addClassIdx(int iClass);
		void setClassMask(uint64_t mClass);
		void setTopClass(int iClass, float prob = 1.0);
		int getTopClass(void);
		float getTopClassProb(void);
		bool bClass(int iClass);
		bool bClassMask(uint64_t mClass);

		void setText(string &txt);
		string getText(void);

		//kinetics
		virtual void updateKinetics(void);

		//time stamp
		virtual void setTstamp(uint64_t t);
		virtual uint64_t getTstamp(void);

	private:
		static void *getUpdate(void *This)
		{
			((_Object *)This)->update();
			return NULL;
		}

	protected:
		//general
		uint64_t m_tStamp;

		//pos and dim
		vFloat3 m_vPos;	 //world pos x, y, z
		vFloat4 m_vDim;	 //width, height, depth, radius
		vFloat3 m_vAtti; //attitude roll, pith, yaw

		//vertex
		vector<vFloat2> m_vVertices;

		//kinetics
		vFloat3 m_vSpeed;
		vFloat3 m_vAccel;

		//classification
		OBJ_TYPE m_type;
		int m_topClass;	   //most probable class
		float m_topProb;   //prob for the topClass
		uint64_t m_mClass; //all candidate class mask
		string m_txt;

		//Tracker
		void *m_pTracker;
	};

}
#endif
