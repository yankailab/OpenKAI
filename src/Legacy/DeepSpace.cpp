/*
 * DeepSpace.cpp
 *
 *  Created on: May 7, 2016
 *      Author: Kai Yan
 */

#include "DeepSpace.h"

DeepSpace::DeepSpace()
{
	// TODO Auto-generated constructor stub
	m_lastError = 0;

}

DeepSpace::~DeepSpace()
{
	// TODO Auto-generated destructor stub
}

bool DeepSpace::create(DeepSpaceCoordinate* pCoord)
{
	if(pCoord == 0)
	{
		m_lastError = DS_ERR_NULLP;
		return false;
	}

	if(pCoord->m_minD <= 0.0 ||
			pCoord->m_nD <= 0.0 ||
			pCoord->m_kD <= 0.0 ||
			pCoord->m_kX <= 0.0 ||
			pCoord->m_kY <= 0.0)
	{
		m_lastError = DS_ERR_INVALID_COORD;
		return false;
	}

	double lenX = pCoord->m_lngTo - pCoord->m_lngFrom;
	double lenY = pCoord->m_latTo - pCoord->m_latFrom;

	pCoord->m_numBlkX = 1;
	pCoord->m_numBlkY = ((lenX / (pCoord->m_minD * pCoord->m_kX)) + 1) * pCoord->m_numBlkX;
	pCoord->m_numBlkZ = ((lenY / (pCoord->m_minD * pCoord->m_kY)) + 1) * pCoord->m_numBlkY;

	m_coord = *pCoord;

	m_lastError = 0;
	return true;
}

long DeepSpace::latlng2DSC(double lat, double lng, double Z)
{
	if(lat < m_coord.m_latFrom || lat > m_coord.m_latTo)
	{
		m_lastError = DS_ERR_INVALID_VALUE;
		return -1;
	}

	if(lng < m_coord.m_lngFrom || lat > m_coord.m_lngTo)
	{
		m_lastError = DS_ERR_INVALID_VALUE;
		return -1;
	}

	lat += m_coord.m_latOffset;
	lng += m_coord.m_lngoffset;

	//TODO


}

bool DeepSpace::DSC2latlng(long DSC, double* pLat, double* pLng)
{

	return true;
}

bool DeepSpace::estimatePosition(long* pDSC, int numDSC, double* pLat, double* pLng, double* pHeight, double* prob)
{

	return true;
}

long DeepSpace::getLastError(void)
{
	return m_lastError;
}


