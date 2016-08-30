/*
 * _AprilTags.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef DETECTOR_APRILTAGS_H_
#define DETECTOR_APRILTAGS_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Stream/_Stream.h"
#include "../Base/_ThreadBase.h"
#include "DetectorBase.h"
#include "../include/apriltags-cpp/TagDetector.h"

#define NUM_TAGS 128

namespace kai
{

class _AprilTags : public DetectorBase, public _ThreadBase
{
public:
	_AprilTags();
	virtual ~_AprilTags();

	bool init(JSON* pJson, string name);
	bool start(void);
	bool draw(Frame* pFrame, iVector4* pTextPos);

	bool getTags(fVector3* pCenter);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_AprilTags*) This)->update();
		return NULL;
	}


public:
	int		 m_numTag;
	iVector3 m_pTag[NUM_TAGS];

	_Stream*	m_pCamStream;
	Frame*		m_pFrame;

	TagDetector*		m_pTagDetector;
	TagDetectionArray	m_tagArray;
	string				m_tagFamily;

};

} /* namespace kai */

#endif /* SRC_MarkerDetector_H_ */
