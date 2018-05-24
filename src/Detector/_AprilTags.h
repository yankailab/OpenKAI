/*
 * _AprilTags.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__AprilTags_H_
#define OpenKAI_src_Detector__AprilTags_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"
#include "../include/apriltags-cpp/TagDetector.h"


#define NUM_TAGS 128
#define DEFAULT_TAG_FAMILY "Tag16h5"
#define NUM_PER_TAG 10

namespace kai
{

struct APRIL_TAG
{
	TagDetection m_tag;
	uint64_t	m_tStamp;
	uint64_t	m_detInterval;
};


class _AprilTags : public _ThreadBase
{
public:
	_AprilTags();
	virtual ~_AprilTags();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	int getTags(int tagID, APRIL_TAG* pTag);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AprilTags*) This)->update();
		return NULL;
	}


public:
	int		  m_numTags;
	APRIL_TAG m_pTag[NUM_TAGS][NUM_PER_TAG];

	_VisionBase* m_pVision;
	Frame		 m_BGR;
	uint64_t	 m_tStamp;

	TagDetector*		m_pTagDetector;
	TagDetectionArray	m_tagArray;
	string				m_tagFamily;
	double				m_tagErr;
	uint64_t			m_tagLifetime;
	int					m_tagDistThr;
	double				m_tagAliveFrom;
	double				m_tagAliveInterval;
	double				m_tagScaling;
	int					m_tagSizeLim;

};

}

#endif
