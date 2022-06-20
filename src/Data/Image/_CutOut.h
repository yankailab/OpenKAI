/*
 * _CutOut.h
 *
 *  Created on: Sept 16, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Image__CutOut_H_
#define OpenKAI_src_Data_Image__CutOut_H_

#include "../../Base/_ModuleBase.h"
#include "../../Utility/utilCV.h"

namespace kai
{

	struct CutOutDir
	{
		string m_dirRGBIn;
		string m_dirSegIn;
		string m_dirRGBOut;
		string m_dirSegOut;

		double m_w;
		double m_h;
		double m_dW;
		double m_dH;
		int m_nX;
		int m_nY;
		vector<vDouble4> m_vBB;

		void init(void)
		{
			m_dirRGBIn = "";
			m_dirSegIn = "";
			m_dirRGBOut = "";
			m_dirSegOut = "";

			m_w = 0.6;
			m_h = 0.6;
			m_dW = 0.1;
			m_dH = 0.1;

			m_vBB.clear();
		}

		void setup(void)
		{
			m_nX = ((1.0 - m_w) / m_dW) + 1;
			m_nY = ((1.0 - m_h) / m_dH) + 1;

			for (int i = 0; i < m_nY; i++)
			{
				for (int j = 0; j < m_nX; j++)
				{
					vDouble4 bb;
					bb.x = j * m_dW;
					bb.z = bb.x + m_w;
					bb.y = i * m_dH;
					bb.w = bb.y + m_h;
					m_vBB.push_back(bb);
				}
			}
		}
	};

	class _CutOut : public _ModuleBase
	{
	public:
		_CutOut();
		~_CutOut();

		bool init(void *pKiss);
		bool start(void);

	private:
		void process(CutOutDir *pD);
		int getFileIdx(string &file);
		string getFileWithOutIdx(string &file);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_CutOut *)This)->update();
			return NULL;
		}

	public:
		vector<CutOutDir> m_vDir;
		vector<string> m_vRGBIn;
		vector<string> m_vSegIn;

		string m_extImgIn;
		string m_extImgOut;

		double m_nMinPixel;
	};

}
#endif
