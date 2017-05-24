/*
 * _DNNodometryTrain.cpp
 *
 *  Created on: May 17, 2017
 *      Author: yankai
 */

#include "_DNNGen_odometry.h"

namespace kai
{

_DNNGen_odometry::_DNNGen_odometry()
{
	m_pIN = NULL;
	m_pZED = NULL;
	m_attiRad.init();
	m_tNow = 0;
}

_DNNGen_odometry::~_DNNGen_odometry()
{

}

bool _DNNGen_odometry::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string outDir = "";
	int nGen = 10000;

	F_INFO(pK->v("outDir", &outDir));
	F_INFO(pK->v("nGen", &nGen));

	random_device rand;
	mt19937 mt(rand());
	uniform_int_distribution<> rand224(0, 223);
	uniform_int_distribution<> rand30(1, 30);
	uniform_int_distribution<> rand256(0, 255);

	string outFile = outDir + "train.txt";
	ofstream ofs;
	ofs.open(outFile.c_str(), ios::out);

	for (int i = 0; i < nGen; i++)
	{
		Mat img;
		int x, y, d, b, g, r;
		stringstream ss;

		x = rand224(mt);
		y = rand224(mt);
		d = rand30(mt);
		b = rand256(mt);
		g = rand256(mt);
		r = rand256(mt);

		img = Mat(Size(224, 224), CV_8UC3, Scalar::all(255));
		circle(img, Point(x, y), d, Scalar(b, g, r), -1);

		ss << setfill('0') << setw(6) << right << i;
		ofs << ss.str() << ".png" << "\t" << x << "\t" << y << "\t" << d << "\t" << b << "\t" << g << "\t" << r << endl;

		imwrite(outDir + ss.str() + ".png", img);

		LOG_I("Generated: "<<i);

//		waitKey(30);
	}

	ofs.close();
	exit(0);

	return true;
}

bool _DNNGen_odometry::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_ERROR_F(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _DNNGen_odometry::start(void)
{
	m_bThreadON = false;//true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _DNNGen_odometry::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();
		m_tNow = get_time_usec();

		this->autoFPSto();
	}
}

bool _DNNGen_odometry::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
