/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_SurfaceBase.h"

namespace kai
{

	_SurfaceBase::_SurfaceBase()
	{
	}

	_SurfaceBase::~_SurfaceBase()
	{
	}

	bool _SurfaceBase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;


		// draw
		jKv(j, "bDrawText", m_bDrawText);
		jKv(j, "bDrawPos", m_bDrawPos);
		jKv(j, "bDrawBB", m_bDrawBB);

		// buffer
		jKv(j, "nBuf", m_nBuf);

		return true;
	}

	bool _SurfaceBase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["bDrawText"] = m_bDrawText;
		j["bDrawPos"] = m_bDrawPos;
		j["bDrawBB"] = m_bDrawBB;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	void _SurfaceBase::clear(void)
	{
	}

	bool _SurfaceBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _SurfaceBase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}

	_ObjectBase *_SurfaceBase::add(const _ObjectBase &o)
	{
	}

	_ObjectBase *_SurfaceBase::get(int i)
	{
	}

	void _SurfaceBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

//		((_Console *)pConsole)->addMsg("nObj=" + i2str(m_sO.get()->size()), 1);
	}

	void _SurfaceBase::draw(void *pMat)
	{
#ifdef USE_OPENCV
		NULL_(pMat);
		this->_ModuleBase::draw(pMat);
		IF_(!check());

		Mat *pM = static_cast<Mat *>(pMat);
		IF_(pM->empty());

		_ObjectBase *pO;
		Scalar oCol;
		Scalar bCol = Scalar(100, 100, 100);
		int col;
		int colStep = 20;
		int i = 0;

		while ((pO = get(i++)) != NULL)
		{
			if (pO->getType() == obj_tag)
			{
				Vector3f vP = pO->getPos();
				Point pCenter = Point(vP.x() * pM->cols, vP.y() * pM->rows);
				int r = pO->getDim().w(); // * pM->cols;

				circle(*pM, pCenter, r, Scalar(255, 255, 0), 2);

				putText(*pM, "iTag=" + i2str(pO->getTopClass()) + ", angle=" + i2str(pO->getAttitude().x()),
						pCenter,
						FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 0);

				double rad = -pO->getAttitude().x() * DEG_2_RAD;
				Point pD = Point(r * sin(rad), r * cos(rad));
				line(*pM, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
			}
			else
			{
				int iClass = pO->getTopClass();

				col = colStep * iClass;
				oCol = Scalar((col + 85) % 255, (col + 170) % 255, col) + bCol;

				// bb
				Rect r = bb2Rect<Vector4f>(pO->getBB2D(pM->cols, pM->rows));
				rectangle(*pM, r, oCol, 1);

				// position
				if (m_bDrawPos)
				{
					putText(*pM, f2str(pO->getPos().z()),
							Point(r.x + 15, r.y + 25),
							FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
				}

				// text
				if (m_bDrawText)
				{
					string oName = string(pO->getText());
					if (oName.length() > 0)
					{
						putText(*pM, oName,
								Point(r.x + 15, r.y + 50),
								FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
					}
				}

				// BB
				if (m_bDrawBB)
				{
					string strBB = "(" + i2str(r.x) + ", " + i2str(r.y) + ", " + i2str(r.width) + "," + i2str(r.height) + ")";
					putText(*pM, strBB,
							Point(r.x + 15, r.y + 50),
							FONT_HERSHEY_SIMPLEX, 0.6, oCol, 1);
				}
			}
		}

		// roi
		Rect roi = bb2Rect(bbScale(m_vRoi, pM->cols, pM->rows));
		rectangle(*pM, roi, Scalar(0, 255, 255), 1);
#endif
	}

}
