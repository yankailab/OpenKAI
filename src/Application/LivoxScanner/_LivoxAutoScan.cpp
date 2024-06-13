/*
 * _LivoxAutoScan.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_LivoxAutoScan.h"

namespace kai
{
	_LivoxAutoScan::_LivoxAutoScan()
	{
		m_nP = 0;
		m_nPmax = INT_MAX;
		m_rVoxel = 0.05;
		m_fProcess.clearAll();
		m_baseDir = "";
		m_dir = "";
		m_tWaitSec = 5;

		m_rB = 0.0;
		m_bScanning = false;
		m_nTake = 0;
	}

	_LivoxAutoScan::~_LivoxAutoScan()
	{
	}

	bool _LivoxAutoScan::init(void *pKiss)
	{
		IF_F(!this->_PCstream::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("rVoxel", &m_rVoxel);
		pK->v("nPmax", &m_nPmax);
		pK->v("baseDir", &m_baseDir);
		pK->v("tWaitSec", &m_tWaitSec);

		pK->v("vRangeH", &m_actH.m_vRange);
		pK->v("dH", &m_actH.m_dV);
		pK->v("vRangeV", &m_actV.m_vRange);
		pK->v("dV", &m_actV.m_dV);

		m_fProcess.set(lvScanner_reset);

		return true;
	}

	bool _LivoxAutoScan::link(void)
	{
		IF_F(!this->_PCstream::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		F_ERROR_F(pK->v("_ActuatorBaseH", &n));
		m_actH.m_pAct = (_ActuatorBase *)(pK->findModule(n));
		NULL_Fl(m_actH.m_pAct, n + ": not found");

		n = "";
		F_ERROR_F(pK->v("_ActuatorBaseV", &n));
		m_actV.m_pAct = (_ActuatorBase *)(pK->findModule(n));
		NULL_Fl(m_actV.m_pAct, n + ": not found");

		vector<string> vGB;
		pK->a("vGeometryBase", &vGB);
		for (string p : vGB)
		{
			_GeometryBase *pGB = (_GeometryBase *)(pK->findModule(p));
			IF_CONT(!pGB);

			m_vpGB.push_back(pGB);
		}

		return true;
	}

	bool _LivoxAutoScan::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		return true;
	}

	int _LivoxAutoScan::check(void)
	{
		//		IF__(m_vpGB.empty(), -1);

		return this->_PCstream::check();
	}

	void _LivoxAutoScan::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			if (m_fProcess.b(lvScanner_reset, true))
				scanReset();

			if (m_fProcess.b(lvScanner_start, true))
				scanStart();

			if (m_fProcess.b(lvScanner_stop, true))
				scanStop();

			if (m_fProcess.b(lvScanner_save, true))
				savePC();

			scanUpdate();

			m_pT->autoFPSto();
		}
	}

	void _LivoxAutoScan::center(void)
	{
		IF_(check() < 0);

		m_actH.m_pAct->setPtarget(0, 0);
		m_actV.m_pAct->setPtarget(0, 0);
	}

	void _LivoxAutoScan::scanReset(void)
	{
		IF_(check() < 0);

		center();

		_Livox *pPsrc = (_Livox *)m_vpGB[0];
		pPsrc->clear();

		m_nP = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud *pP = &m_vPC[i];
			pP->Clear();
		}
		m_vPC.clear();

		m_rB = (float)m_nP / (float)m_nPmax;
	}

	void _LivoxAutoScan::scanStart(void)
	{
		IF_(check() < 0);

		m_actH.reset();
		m_actV.reset();

		// Actuator reset pos
		// todo

		_Livox *pPsrc = (_Livox *)m_vpGB[0];
		//		pPsrc->startStream();

		m_bScanning = true;
	}

	void _LivoxAutoScan::scanStop(void)
	{
		IF_(check() < 0);

		_Livox *pPsrc = (_Livox *)m_vpGB[0];
		//		pPsrc->stopStream();

		m_bScanning = false;
	}

	void _LivoxAutoScan::scanUpdate(void)
	{
		IF_(check() < 0);
		IF_(!m_bScanning);

		m_actH.m_pAct->setPtarget(0, m_actH.m_v);
		m_actV.m_pAct->setPtarget(0, m_actV.m_v);
		while (!m_actH.m_pAct->bComplete())
			;
		while (!m_actV.m_pAct->bComplete())
			;

		// if(m_nP <= 0)
		// {
			sleep(m_tWaitSec);
		// }

		_Livox *pPsrc = (_Livox *)m_vpGB[0];
		vDouble3 vT;
		vT.clear();
		pPsrc->setOffset(m_vOffset, vT);
		vDouble3 vR;
		vR.x = m_actV.m_v * DEG_2_RAD;	// pitch
		vR.y = -m_actH.m_v * DEG_2_RAD; // yaw
		vR.z = 0.0;						// roll
		pPsrc->setTranslation(vT, vR);

		pPsrc->clear();
		sleep(m_tWaitSec); // TODO: change to point number

		PointCloud pc;
		pPsrc->getPC(&pc);
		int nPnew = pc.points_.size();
		if (nPnew > 0)
		{
			if (m_nP + nPnew >= m_nPmax)
			{
				scanStop();
				center();
			}

			m_vPC.push_back(pc);
			m_nP += nPnew;
			m_rB = (float)m_nP / (float)m_nPmax;
		}

		// update to next pos
		m_actV.update();
		if (m_actV.bComplete())
		{
			m_actV.reset();
			m_actH.update();
			if (m_actH.bComplete())
			{
				scanStop();
				center();
			}
		}
	}

	void _LivoxAutoScan::savePC(void)
	{
		IF_(check() < 0);
		IF_(m_nP <= 0);

		// Make new folder
		m_dir = m_baseDir + tFormat() + "/";
		string cmd = "mkdir " + m_dir;
		system(cmd.c_str());

		io::WritePointCloudOption par;
		par.write_ascii = io::WritePointCloudOption::IsAscii::Binary;
		par.compressed = io::WritePointCloudOption::Compressed::Uncompressed;

		PointCloud pcMerge;
		int nSave = 0;
		for (int i = 0; i < m_vPC.size(); i++)
		{
			PointCloud *pP = &m_vPC[i];
			nSave += (io::WritePointCloudToPLY(m_dir + i2str(i) + ".ply",
											   *pP,
											   par))
						 ? 1
						 : 0;

			pcMerge += *pP;
		}

		nSave += (io::WritePointCloudToPLY(m_dir + "_merged_raw.ply",
										   pcMerge,
										   par))
					 ? 1
					 : 0;

		PointCloud pcVD;
		pcVD = *pcMerge.VoxelDownSample(m_rVoxel);
		nSave += (io::WritePointCloudToPLY(m_dir + "_merged.ply",
										   pcVD,
										   par))
					 ? 1
					 : 0;
	}

	void _LivoxAutoScan::reset(void)
	{
		m_fProcess.set(lvScanner_reset);
	}

	void _LivoxAutoScan::startAuto(void)
	{
		m_fProcess.set(lvScanner_start);
	}

	void _LivoxAutoScan::stop(void)
	{
		m_fProcess.set(lvScanner_stop);
	}

	void _LivoxAutoScan::save(void)
	{
		m_fProcess.set(lvScanner_save);
	}

	void _LivoxAutoScan::setConfig(const LivoxAutoScanConfig &c)
	{
		m_actH.m_vRange = c.m_vRangeH;
		m_actH.m_dV = c.m_dH;
		m_actV.m_vRange = c.m_vRangeV;
		m_actV.m_dV = c.m_dV;
		m_vOffset = c.m_vOffset;
	}

	LivoxAutoScanConfig _LivoxAutoScan::getConfig(void)
	{
		LivoxAutoScanConfig c;
		c.m_vRangeH = m_actH.m_vRange;
		c.m_dH = m_actH.m_dV;
		c.m_vRangeV = m_actV.m_vRange;
		c.m_dV = m_actV.m_dV;
		c.m_vOffset = m_vOffset;

		return c;
	}

	bool _LivoxAutoScan::bScanning(void)
	{
		return m_bScanning;
	}

	float _LivoxAutoScan::getBufferCap(void)
	{
		return m_rB;
	}

}
