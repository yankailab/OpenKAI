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

	class _Object : public _ModuleBase
	{
	public:
		_Object();
		virtual ~_Object();

		//general
		virtual bool init(void *pKiss);
		virtual void clear(void);
		virtual bool start(void);
		virtual void update(void);

		//pos
		void setPos(vFloat3 &p);
		vFloat3 getPos(void);

		void setX(float x);
		void setY(float y);
		void setZ(float z);
		float getX(void);
		float getY(void);
		float getZ(void);

		void setPosScr(vFloat2 &p);
		vFloat2 getPosScr(void);
		void setDimScr(vFloat2 &d);
		vFloat2 getDimScr(void);

		//attitude
		void setAttitude(vFloat3 &a);
		vFloat3 getAttitude(void);

		void setRoll(float r);
		void setPitch(float p);
		void setYaw(float y);
		float getRoll(void);
		float getPitch(void);
		float getYaw(void);

		//dimension
		void setDim(vFloat4 &d);
		void setWidth(float w);
		void setHeight(float h);
		void setDepth(float d);
		void setRadius(float r);

		vFloat4 getDim(void);
		float getWidth(void);
		float getHeight(void);
		float getDepth(void);
		float getRadius(void);

		float area(void);
		float volume(void);

		void scale(float kx = 1.0, float ky = 1.0, float kz = 1.0);

		//convenient
		void setBB2D(vFloat4 bb);
		void setBB2D(float l, float t, float w, float h);
		vFloat4 getBB2D(void);
		vFloat4 getBB2Dscaled(float kx, float ky);

		//vertex
		void setVertices2D(vFloat2 *pV, int nV);
		vFloat2 *getVertex(int i);

		//classification
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
		vFloat3 m_vAtti; //attitude roll, pith, yaw
		vFloat4 m_vDim;	 //width, height, depth, radius

		vFloat2 m_vPosScr; //screen pos x, y
		vFloat2 m_vDimScr; //w,h

		//kinetics
		vFloat3 m_vSpeed;
		vFloat3 m_vAccel;

		//classification
		int m_topClass;	   //most probable class
		float m_topProb;   //prob for the topClass
		uint64_t m_mClass; //all candidate class mask
		string m_txt;

		//vertex
		vector<vFloat2> m_vVertex;

		//bitflag
		uint64_t m_mFlag;

		//Tracker
		void *m_pTracker;
	};

}
#endif
