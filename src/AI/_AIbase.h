/*
 * _AIbase.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef AI__AIBASE_H_
#define AI__AIBASE_H_

#include "../Base/common.h"

#include "../Base/_ThreadBase.h"
#include "../Stream/_StreamBase.h"

namespace kai
{

struct OBJECT
{
	vDouble4 m_fBBox;
	vInt4 m_bbox;
	vInt2 m_camSize;
	double m_dist;
	double m_prob;
	int m_iClass;
	string m_name;
	vector<Point> m_contour;
	int64_t m_frameID;
	uint8_t m_safety;

	void init(void)
	{
		m_fBBox.init();
		m_bbox.init();
		m_camSize.init();
		m_dist = -1.0;
		m_prob = 0;
		m_iClass = 0;
		m_name = "";
		m_frameID = 0;
		m_safety = -1;
	}
};

class _AIbase: public _ThreadBase
{
public:
	_AIbase();
	virtual ~_AIbase();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	virtual bool start(void);
	virtual bool draw(void);

	int size(void);
	bool add(OBJECT* pNewObj);
	OBJECT* get(int i, int64_t frameID);
	OBJECT* getByClass(int iClass);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AIbase*) This)->update();
		return NULL;
	}

public:
	_StreamBase* m_pStream;

	string m_fileModel;
	string m_fileTrained;
	string m_fileMean;
	string m_fileLabel;

	OBJECT* m_pObj;
	int m_nObj;
	int m_iObj;
	int64_t m_objLifetime;

	double m_sizeName;
	double m_sizeDist;
	Scalar m_colName;
	Scalar m_colDist;
	Scalar m_colObs;

	double m_contourBlend;
	bool m_bDrawContour;



};

}

#endif
