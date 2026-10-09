/*
 * DataObjStream.h
 *
 *  Created on: Oct 9, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_DataObject_DataObjStream_H_
#define OpenKAI_src_DataObject_DataObjStream_H_

#include "DataObjBase.h"

namespace kai
{

	template <class T>
	class DataObjStream : public DataObjBase
	{
	public:
		DataObjStream();
		virtual ~DataObjStream();

		// Producers own each element's m_tStamp; tStamp updates only this stream.
		void add(const vector<T> &vSrc, uint64_t tStamp = 0);

		// Copy retained elements in arrival order, strictly newer than tStampFrom.
		// The returned timestamp belongs to the stream, not the element cursor.
		uint64_t get(vector<T> &vDest, uint64_t tStampFrom = 0);

	protected:
		vector<T> m_vElement;
		size_t m_iBset = 0; // next element slot to write
		std::shared_mutex m_sMutex;
	};

}
#endif
