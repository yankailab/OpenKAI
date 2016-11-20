/*
 * _AprilTags.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef DETECTOR_APRILTAGS_H_
#define DETECTOR_APRILTAGS_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Stream/_Stream.h"
#include "../Detector/DetectorBase.h"
#include "../include/apriltags-cpp/TagDetector.h"


#define NUM_TAGS 128
#define DEFAULT_TAG_FAMILY "Tag16h5"
#define NUM_PER_TAG 10

namespace kai
{

struct APRIL_TAG
{
	TagDetection m_tag;
	uint64_t	m_frameID;
	uint64_t	m_detInterval;
};


class _AprilTags : public DetectorBase, public _ThreadBase
{
public:
	_AprilTags();
	virtual ~_AprilTags();

	bool init(Config* pConfig);
	bool link(void);
	bool start(void);
	bool draw(Frame* pFrame, vInt4* pTextPos);

	int getTags(int tagID, APRIL_TAG* pTag);
	void reset(void);

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

	_Stream*	m_pStream;
	Frame*		m_pFrame;
	uint64_t	m_frameID;

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

} /* namespace kai */

#endif /* SRC_MarkerDetector_H_ */
