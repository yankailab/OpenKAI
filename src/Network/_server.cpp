/*
 * _server.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_server.h"

namespace kai
{

_server::_server()
{
	_ThreadBase();


}

_server::~_server()
{
	// TODO Auto-generated destructor stub
}

bool _server::init(Kiss* pKiss)
{
//	CHECK_INFO(pJson->getVal("CAFFE_DIR", &caffeDir));
//	LOG(INFO)<<"Caffe Initialized";

//	double FPS = DEFAULT_FPS;
//	CHECK_ERROR(pJson->getVal("CLASSIFIER_FPS", &FPS));
//	this->setTargetFPS(FPS);

	return true;
}

bool _server::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _server::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_frameID = get_time_usec();


		this->autoFPSto();
	}

}


} /* namespace kai */
