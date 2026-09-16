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
		void setPos(const Vector3f &vP);
		void setPos(float x, float y, float z);
		Vector3f getPos(void);

		//dimension
		void setDim(const Vector4f &vD);
		void setDim(float w, float h, float d, float r);
		Vector4f getDim(void);
		float getDimArea(void);
		float getDimVolume(void);

		//attitude
		void setAttitude(const Vector3f &vA);
		void setAttitude(float r, float p, float y);
		Vector3f getAttitude(void);

		//convenient
		void setBB2D(const Vector4f& vBB, float kX = 1.0, float kY = 1.0);
		void setRect(float l, float t, float w, float h, float kX = 1.0, float kY = 1.0);
		Vector4f getBB2D(float kX = 1.0, float kY = 1.0);

		//vertex
		void setVertices2D(Vector2f *pV, int nV, float kX = 1.0, float kY = 1.0);
		Vector2f *getVertex(int i);

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
		Vector3f m_vPos = Vector3f::Zero();	 //world pos x, y, z
		Vector4f m_vDim = Vector4f::Zero();	 //width, height, depth, radius
		Vector3f m_vAtti = Vector3f::Zero(); //attitude roll, pith, yaw

		//vertex
		vector<Vector2f> m_vVertices;

		//kinetics
		Vector3f m_vSpeed = Vector3f::Zero();
		Vector3f m_vAccel = Vector3f::Zero();

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
