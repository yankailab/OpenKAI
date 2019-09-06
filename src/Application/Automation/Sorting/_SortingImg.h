/*
 * _SortingImg.h
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Automation__SortingImg_H_
#define OpenKAI_src_Automation__SortingImg_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Protocol/_OKlink.h"

namespace kai
{

struct SORTING_IMG
{
	int			m_id;
	uint64_t	m_tStamp;
	int			m_iClass;
	vFloat4		m_bb;
	Mat			m_mImg;
	vector<uchar> m_vBuf;

	void init(void)
	{
		m_id = -1;
		m_tStamp = getTimeUsec();
		m_iClass = -1;
		m_bb.init();
	}

	bool encodeImg(Mat m, vInt2& mSize, string& ext, vector<int>& vParam)
	{
		IF_F(m.empty());

		if(mSize.x != m.cols || mSize.y != m.rows)
			cv::resize(m, m_mImg, Size(mSize.x, mSize.y));
		else
			m.copyTo(m_mImg);

		imencode(ext, m_mImg, m_vBuf, vParam);

		return true;
	}

	bool decodeImg(Mat mBuf)
	{
		m_mImg = imdecode(mBuf, IMREAD_COLOR);
		IF_F(m_mImg.empty());

		return true;
	}
};

class _SortingImg: public _ThreadBase
{
public:
	_SortingImg(void);
	virtual ~_SortingImg();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	void updateImg(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SortingImg *) This)->update();
		return NULL;
	}

public:
	_DetectorBase*	m_pDet;
	_OKlink*		m_pOL;

	vInt2			m_imgSize;
	vector<int>		m_vParam;
};

}
#endif
