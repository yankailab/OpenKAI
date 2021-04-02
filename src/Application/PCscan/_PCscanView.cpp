/*
 * _PCscanView.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCscanView.h"

namespace kai
{

	_PCscanView::_PCscanView()
	{
	}

	_PCscanView::~_PCscanView()
	{
	}

	bool _PCscanView::init(void *pKiss)
	{
		IF_F(!this->_PCviewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool _PCscanView::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTgui);
		IF_F(!m_pTgui->start(getUpdateGUI, this));

		return true;
	}

	int _PCscanView::check(void)
	{
		return this->_PCviewer::check();
	}

	void _PCscanView::update(void)
	{
		m_pT->sleepT(0);

		m_spWin->setDevice(m_device);

		while (m_nPread <= 0)
			readAllPC();

		m_spWin->SetGeometry(m_spPC);

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			readAllPC();
			m_spWin->UpdateGeometry(m_spPC);

			m_pT->autoFPSto();
		}
	}

	void _PCscanView::updateGUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<WindowPCscan>(*this->getName(), 2000, 1000);
		app.AddWindow(m_spWin);

		m_spWin->setCbResetPC(OnBtnResetPC, (void *)this);
		m_spWin->setCbResetPicker(OnBtnResetPicker, (void *)this);
		m_spWin->setCbSavePC(OnBtnSavePC, (void *)this);

		m_pT->wakeUp();
		app.Run();
		exit(0);
	}

	void _PCscanView::OnBtnResetPC(void *pPCV)
	{
		NULL_(pPCV);
		_PCscanView *pV = (_PCscanView *)pPCV;
	}

	void _PCscanView::OnBtnResetPicker(void *pPCV)
	{
		NULL_(pPCV);
		_PCscanView *pV = (_PCscanView *)pPCV;
	}

	void _PCscanView::OnBtnSavePC(void *pPCV)
	{
		NULL_(pPCV);
		_PCscanView *pV = (_PCscanView *)pPCV;

		io::WritePointCloudOption par;
		par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
		par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

		pthread_mutex_lock(&pV->m_mutexPC);
		PointCloud pc = *pV->m_sPC.prev();
		pthread_mutex_unlock(&pV->m_mutexPC);
		io::WritePointCloudToPLY("/home/kai/dev/testModel.ply", pc, par);
	}

}
#endif
