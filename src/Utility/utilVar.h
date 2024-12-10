#ifndef OpenKAI_src_Utility_utilVar_H_
#define OpenKAI_src_Utility_utilVar_H_

namespace kai
{
	template <typename T>
	struct VAR_WR
	{
		T m_vR; // value read out
		T m_vW; // value to be written

		void init(T w, T r)
		{
			m_vW = w;
			m_vR = r;
		}

		void init(void)
		{
			m_vW = 0;
			m_vR = 0;
		}

		void read(T v)
		{
			m_vR = v;
		}

		void write(T v)
		{
			m_vW = v;
		}

		T getRead(void)
		{
			return m_vR;
		}

		T getWrite(void)
		{
			return m_vW;
		}

		bool bW(void)
		{
			return (m_vR != m_vW);
		}
	};

}
#endif
