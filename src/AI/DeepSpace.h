/*
 * DeepSpace.h
 *
 *  Created on: May 7, 2016
 *      Author: Kai Yan
 */

#ifndef DeepSpace_H_
#define DeepSpace_H_

struct DeepSpaceCoordinate
{
	double m_minZ;	//minimum edge length of block in meter
	double m_nZ;	//number of Z direction layers
	double m_kZ;	//scaling factor for each Z layer
	double m_kX;	//factor for offset between each two adjacent blocks in Longitude direction
	double m_kY;	//factor for offset between each two adjacent blocks in Latitude direction

	double m_latFrom;
	double m_latTo;
	double m_latOffset;

	double m_lngFrom;
	double m_lngTo;
	double m_lngoffset;

	long m_blkPerZ;

};


class DeepSpace
{
public:
	DeepSpace();
	virtual ~DeepSpace();

	bool create(DeepSpaceCoordinate* pCoordinate);
	long latlng2DSC(double lat, double lng, double Z);
	bool DSC2latlng(long DSC, double* pLat, double* pLng);
	bool estimatePosition(long* pDSC, int numDSC, double* pLat, double* pLng, double* pHeight, double* prob);
	long getLastError(void);

private:
	DeepSpaceCoordinate m_coordinate;

	long m_lastError;

};

#endif /* DeepSpace_H_ */
