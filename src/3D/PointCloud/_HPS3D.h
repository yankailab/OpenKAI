/*
 * _HPS3D.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_HPS3D_H_
#define OpenKAI_src_3D_PointCloud_HPS3D_H_

#include "../../Base/common.h"
#include "_PCbase.h"
#include "../../Dependency/HPS3D.h"

namespace kai
{

class _HPS3D: public _PCbase
{
public:
	_HPS3D();
	virtual ~_HPS3D();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

private:
	void updateRS(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_HPS3D *) This)->update();
		return NULL;
	}

	static void* cbRecv(HPS3D_HandleTypeDef *handle,AsyncIObserver_t *event)
	{
		switch(event->RetPacketType)
		{
			case SIMPLE_ROI_PACKET:
				printf("observer id = %d, distance average:%d\n",event->ObserverID,event->MeasureData.simple_roi_data[0].distance_average);
				break;
			case FULL_ROI_PACKET:
				printf("observer id = %d, distance average:%d\n",event->ObserverID,event->MeasureData.full_roi_data[0].distance_average);
				break;
			case FULL_DEPTH_PACKET:
				printf("observer id = %d, distance average:%d\n",event->ObserverID,event->MeasureData.full_depth_data->distance_average);
				//printf("observer id = %d,  point_cloud_data[0]:(%f,%f,%f)\n",event->ObserverID,event->MeasureData.point_cloud_data->point_data[0].x,
				//event->MeasureData.point_cloud_data->point_data[0].y,event->MeasureData.point_cloud_data->point_data[0].z);
				break;
			case SIMPLE_DEPTH_PACKET:
				printf("observer id = %d, distance average:%d\n",event->ObserverID,event->MeasureData.simple_depth_data->distance_average);
				break;
			case OBSTACLE_PACKET:
				printf("observer id = %d\n",event->ObserverID);
				printf(" Obstacle ID：%d\n",event->MeasureData.Obstacle_data->Id);
				printf(" LeftPoint:(%f,%f,%f)\n",event->MeasureData.Obstacle_data->LeftPoint.x,event->MeasureData.Obstacle_data->LeftPoint.y,event->MeasureData.Obstacle_data->LeftPoint.z);
				printf(" RightPoint:(%f,%f,%f)\n",event->MeasureData.Obstacle_data->RightPoint.x,event->MeasureData.Obstacle_data->RightPoint.y,event->MeasureData.Obstacle_data->RightPoint.z);
				printf(" UpperPoint:(%f,%f,%f)\n",event->MeasureData.Obstacle_data->UpperPoint.x,event->MeasureData.Obstacle_data->UpperPoint.y,event->MeasureData.Obstacle_data->UpperPoint.z);
				printf(" UnderPoint:(%f,%f,%f)\n",event->MeasureData.Obstacle_data->UnderPoint.x,event->MeasureData.Obstacle_data->UnderPoint.y,event->MeasureData.Obstacle_data->UnderPoint.z);
				printf(" MinPoint:(%f,%f,%f)\n",event->MeasureData.Obstacle_data->MinPoint.x,event->MeasureData.Obstacle_data->MinPoint.y,event->MeasureData.Obstacle_data->MinPoint.z);
				printf(" DistanceAverage:%d\n\n",event->MeasureData.Obstacle_data->DistanceAverage);
				break;
			case NULL_PACKET:
				printf(" packet is null\n");
				break;
			default:
				printf(" system error\n");
				break;
		}

		return NULL;
	}


public:
	HPS3D_HandleTypeDef handle[DEV_NUM];
	AsyncIObserver_t My_Observer[DEV_NUM];
	ObstacleConfigTypedef ObstacleConf;
	uint8_t connect_number = 0;

};

}
#endif
