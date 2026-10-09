/*
 * DataObjStream.cpp
 *
 *  Created on: Oct 9, 2026
 *      Author: yankai
 */

#include "DataObjStream.h"
#include "BBoxStream.h"
#include "BytePacketStream.h"
#include "CANframeStream.h"
#include "UGLIDcellStream.h"

namespace kai
{

	template <class T>
	DataObjStream<T>::DataObjStream() = default;

	template <class T>
	DataObjStream<T>::~DataObjStream() = default;

	template <class T>
	void DataObjStream<T>::add(const vector<T> &vSrc, uint64_t tStamp)
	{
		IF_(vSrc.empty());

		std::unique_lock lock(m_sMutex);
		IF_(m_vElement.empty());

		for (const T &element : vSrc)
		{
			m_vElement[m_iBset] = element;
			if (++m_iBset == m_vElement.size())
			{
				m_iBset = 0;
			}
		}

		updateTstamp(tStamp);
	}

	template <class T>
	uint64_t DataObjStream<T>::get(vector<T> &vDest, uint64_t tStampFrom)
	{
		std::shared_lock lock(m_sMutex);

		vDest.clear();
		const size_t nElement = m_vElement.size();
		vDest.reserve(nElement);

		for (size_t n = 0, iElement = m_iBset; n < nElement; ++n)
		{
			const T &element = m_vElement[iElement];
			if (element.m_tStamp > tStampFrom)
			{
				vDest.push_back(element);
			}

			if (++iElement == nElement)
			{
				iElement = 0;
			}
		}

		return getTstamp();
	}

	// Keep template definitions here; instantiate each supported element type.
	template class DataObjStream<BBOX_OBJ>;
	template class DataObjStream<BYTE_PACKET>;
	template class DataObjStream<CAN_FRAME>;
	template class DataObjStream<UGLID_CELL_T>;

}
