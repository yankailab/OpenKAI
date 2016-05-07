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

bool DeepSpace::create(DeepSpaceCoordinate* pCoordinate)
{


	m_lastError = 0;
	return true;
}

long DeepSpace::latlng2DSC(double lat, double lng, double Z)
{

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



/*
public class GridIndex
{
	//DEGREE IS THE DEFAULT UNIT

	public static final long MAX_SLIDES = 2;//10;
//	public static final double MAX_SCALES = 2;//5;

	public static final double MAX_ZOOM = 100;//100;//!!!!
	public static final double MIN_DELTA_DEGREE = 0.00001;
	public static final double MAX_DEGREE_BIAS = 10;
	public static final long ZOOM_DIFF = 10;

//	public static final double MAX_RADAR_RADIUS = 10;
	public static final double MIN_DELTA_HEIGHT = 20;
	public static final double MAX_HEIGHT = 500;
	public static final long MAX_HEIGHT_SLIDES = 2;

	public static final double EARTH_EQUATOR_METER = 40000000;

	public long INDEX_PER_ZOOM;
	public long INDEX_PER_SLIDE_LONG;
	public long INDEX_PER_SLIDE_LAT;
	public long INDEX_PER_SLIDE_HEIGHT;
	public long INDEX_PER_LONGITUDE;
	public long INDEX_PER_LATITUDE;
	public long INDEX_PER_HEIGHT;
	public double LATITUDE_BIAS;
	public double LONGITUDE_BIAS;
	public double ACTUAL_MIN_DEGREE;

	public double MAX_RADAR_RADIUS_METER;
	public double MAX_RADAR_RADIUS_DEGREE;
	public double METER_PER_DEGREE;

	//Adjustor needed for longitude!!!!

	double m_fZoom;
	long   m_iZoomLev;
	double m_fDegree;
	double m_fSlideLng;
	double m_fLng;
	double m_fLngLeft;
	long   m_iSlideIdxLng;
	double m_fSlidePosL;
	double m_fLng2SlideL;
	double m_fLng2SlideR;
	long   m_iLngIdx;
	double m_fSlideLat;
	double m_fLat;
	double m_fLatLeft;
	long   m_iSlideIdxLat;
	double m_fSlidePosB;
	double m_fLat2SlideB;
	double m_fLat2SlideT;
	long   m_iLatIdx;
	long   m_iHeightIdx;
	long   m_iSlideIdxHeight;
	long   m_globalIdx;


	public GridIndex()
	{
		//treat height independently to accelerate the speed searching in plane surface.
		METER_PER_DEGREE = EARTH_EQUATOR_METER/360.0;
		MAX_RADAR_RADIUS_DEGREE = MAX_ZOOM * ZOOM_DIFF * MIN_DELTA_DEGREE;
		MAX_RADAR_RADIUS_METER = MAX_RADAR_RADIUS_DEGREE * METER_PER_DEGREE;

		LATITUDE_BIAS = 90 + MAX_RADAR_RADIUS_DEGREE + MAX_DEGREE_BIAS;
		LONGITUDE_BIAS = 180 + MAX_RADAR_RADIUS_DEGREE + MAX_DEGREE_BIAS;
		ACTUAL_MIN_DEGREE = MIN_DELTA_DEGREE * ZOOM_DIFF;

		INDEX_PER_HEIGHT = 1;
		INDEX_PER_LONGITUDE = (long) (((MAX_HEIGHT + MIN_DELTA_HEIGHT)/MIN_DELTA_HEIGHT)*INDEX_PER_HEIGHT);
		INDEX_PER_LATITUDE = (long) ((((LONGITUDE_BIAS * 2) + ACTUAL_MIN_DEGREE) / ACTUAL_MIN_DEGREE) * INDEX_PER_LONGITUDE);	//to avoid negative value in slide, + MIN_DELTA_DEGREE
		INDEX_PER_SLIDE_HEIGHT = (long) ((((LATITUDE_BIAS * 2) + ACTUAL_MIN_DEGREE) / ACTUAL_MIN_DEGREE) * INDEX_PER_LATITUDE);
		INDEX_PER_SLIDE_LONG = INDEX_PER_SLIDE_HEIGHT * MAX_HEIGHT_SLIDES;
		INDEX_PER_SLIDE_LAT = INDEX_PER_SLIDE_LONG * MAX_SLIDES;
		INDEX_PER_ZOOM = INDEX_PER_SLIDE_LAT * MAX_SLIDES;

		System.out.println("Number test");
		System.out.println("INDEX_PER_LATITUDE:"+this.INDEX_PER_LATITUDE);
		System.out.println("INDEX_PER_SLIDE_LONG:"+this.INDEX_PER_SLIDE_LONG);
		System.out.println("INDEX_PER_SLIDE_LAT:"+this.INDEX_PER_SLIDE_LAT);
		System.out.println("INDEX_PER_ZOOM:"+this.INDEX_PER_ZOOM);
		System.out.println("MAX_BASE:"+this.INDEX_PER_ZOOM*(long)(GridIndex.MAX_ZOOM+1));
		System.out.println("LONG_MAX:"+Long.MAX_VALUE);

	}

	public long Pos2GridIdx(long iZoomLev, long slideLat, long slideLong, long slideHeight, double longitude, double latitude, double height)
	{
		//translate into block position
		longitude += LONGITUDE_BIAS;
		latitude += LATITUDE_BIAS;

		double dDegree = MIN_DELTA_DEGREE * (iZoomLev * ZOOM_DIFF);
		double dSlideLat = slideLat * dDegree * (1/(double)MAX_SLIDES);
		double dSlideLong = slideLong * dDegree * (1/(double)MAX_SLIDES);

		long ilong = (long)((longitude - dSlideLong) / dDegree);
		long ilat = (long)((latitude - dSlideLat) / dDegree);
		long iheight = 0;	//height, implement later

		return iZoomLev * INDEX_PER_ZOOM
					+ slideLat * INDEX_PER_SLIDE_LAT
					+ slideLong * INDEX_PER_SLIDE_LONG
					+ slideHeight * INDEX_PER_SLIDE_HEIGHT
					+ ilat * INDEX_PER_LATITUDE
					+ ilong * INDEX_PER_LONGITUDE
					+ iheight * INDEX_PER_HEIGHT;
	}

	public long FindGridIdx(double rRadar, double longitude, double latitude, double height)
	{
		//set limitation on rRadar//!!!!
		//rRadar is in meter now
		rRadar /= METER_PER_DEGREE;
		rRadar = Math.min(rRadar,MAX_RADAR_RADIUS_DEGREE);
		rRadar = Math.max(rRadar,MIN_DELTA_DEGREE);

		m_fZoom = Math.ceil(rRadar * 2 / MIN_DELTA_DEGREE);
		m_fZoom = Math.ceil(m_fZoom / ZOOM_DIFF) * ZOOM_DIFF;	//zoom_fidd: 10x,20x,30x...
		m_fZoom = Math.min(m_fZoom, MAX_ZOOM * ZOOM_DIFF);
		m_iZoomLev = (long)(m_fZoom / ZOOM_DIFF);

		//m_fDegree = MIN_DELTA_DEGREE * m_fZoom;
		m_fDegree = MIN_DELTA_DEGREE * (m_iZoomLev * ZOOM_DIFF);

		//Longitude
		m_fSlideLng = m_fDegree / MAX_SLIDES;
		m_fLng = longitude + LONGITUDE_BIAS;

		m_fLngLeft = m_fLng - rRadar;
		m_iSlideIdxLng = (long)(m_fLngLeft/m_fSlideLng);
		m_fSlidePosL = (m_fSlideLng * m_iSlideIdxLng);
		m_fLng2SlideL = m_fLngLeft - m_fSlidePosL;
		m_fLng2SlideR = (m_fLng + rRadar) - (m_fSlidePosL + m_fDegree);

		//											benefit			lose
		m_iSlideIdxLng += (long) Math.max( Math.signum(m_fLng2SlideR-(m_fSlideLng - m_fLng2SlideL)) , 0);
		m_iLngIdx = (m_iSlideIdxLng/MAX_SLIDES);
		m_iSlideIdxLng %= MAX_SLIDES;

		//Latitude
		m_fSlideLat = m_fDegree / MAX_SLIDES;
		m_fLat = latitude + LATITUDE_BIAS;

		m_fLatLeft = m_fLat - rRadar;
		m_iSlideIdxLat = (long)(m_fLatLeft/m_fSlideLat);
		m_fSlidePosB = (m_fSlideLat * m_iSlideIdxLat);
		m_fLat2SlideB = m_fLatLeft - m_fSlidePosB;
		m_fLat2SlideT = (m_fLat + rRadar) - (m_fSlidePosB + m_fDegree);

		m_iSlideIdxLat += (long) Math.max( Math.signum(m_fLat2SlideT-(m_fSlideLat - m_fLat2SlideB)) , 0);
		m_iLatIdx = (m_iSlideIdxLat/MAX_SLIDES);
		m_iSlideIdxLat %= MAX_SLIDES;

		//long tilong = (long)((m_fLng - m_iSlideIdxLng*m_fSlideLng) / m_fDegree);
		//long tilat = (long)((m_fLat - m_iSlideIdxLat*m_fSlideLat) / m_fDegree);
		//m_iLngIdx=tilong;
		//m_iLatIdx=tilat;

		//implement height later
		m_iHeightIdx = 0;
		m_iSlideIdxHeight = 0;

		m_globalIdx = (long)m_iZoomLev * INDEX_PER_ZOOM		//fZoom?
							+ m_iSlideIdxLat * INDEX_PER_SLIDE_LAT
							+ m_iSlideIdxLng * INDEX_PER_SLIDE_LONG
							+ m_iSlideIdxHeight * INDEX_PER_SLIDE_HEIGHT
							+ m_iLatIdx * INDEX_PER_LATITUDE
							+ m_iLngIdx * INDEX_PER_LONGITUDE
							+ m_iHeightIdx * INDEX_PER_HEIGHT;
		return m_globalIdx;
	}

	public Console FindBlockIdxConsole(double rRadar, double longitude, double latitude, double height)
	{
		FindGridIdx(rRadar, longitude, latitude, height);

		Console mconsole = new Console();
		mconsole.ddegree = m_fDegree;
		mconsole.slideLat = m_fSlideLat;
		mconsole.slideLng = m_fSlideLng;
		mconsole.slideIdxLat = m_iSlideIdxLat;
		mconsole.slideIdxLng = m_iSlideIdxLng;
		mconsole.slideIdxHeight = m_iSlideIdxHeight;
		mconsole.zoom =  Math.ceil(rRadar * 2 / MIN_DELTA_DEGREE);
		mconsole.zoomIdx = m_fZoom;
		mconsole.iLatIdx = m_iLatIdx;
		mconsole.iLngIdx = m_iLngIdx;
		mconsole.iHeightIdx = m_iHeightIdx;
		mconsole.latbias = LATITUDE_BIAS;
		mconsole.lngbias = LONGITUDE_BIAS;
		mconsole.globalIdx = m_globalIdx;

		return mconsole;
	}


}
 */
