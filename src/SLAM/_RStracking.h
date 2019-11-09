/*
 * _RStracking.h
 *
 *  Created on: Oct 29, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RStracking_H_
#define OpenKAI_src_Vision_RStracking_H_

#include "../Base/common.h"
#include "_SlamBase.h"

#ifdef USE_REALSENSE
#include <librealsense2/rs.hpp>
#include <../common/res/t265.h>

namespace kai
{

class _RStracking: public _SlamBase
{
public:
	_RStracking();
	virtual ~_RStracking();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	bool open(void);
	void close(void);

public:
    // Calculates transformation matrix based on pose data from the device
    void calc_transform(rs2_pose& pose_data, float mat[16])
    {
        auto q = pose_data.rotation;
        auto t = pose_data.translation;
        // Set the matrix as column-major for convenient work with OpenGL and rotate by 180 degress (by negating 1st and 3rd columns)
        mat[0] = -(1 - 2 * q.y*q.y - 2 * q.z*q.z); mat[4] = 2 * q.x*q.y - 2 * q.z*q.w;     mat[8] = -(2 * q.x*q.z + 2 * q.y*q.w);      mat[12] = t.x;
        mat[1] = -(2 * q.x*q.y + 2 * q.z*q.w);     mat[5] = 1 - 2 * q.x*q.x - 2 * q.z*q.z; mat[9] = -(2 * q.y*q.z - 2 * q.x*q.w);      mat[13] = t.y;
        mat[2] = -(2 * q.x*q.z - 2 * q.y*q.w);     mat[6] = 2 * q.y*q.z + 2 * q.x*q.w;     mat[10] = -(1 - 2 * q.x*q.x - 2 * q.y*q.y); mat[14] = t.z;
        mat[3] = 0.0f;                             mat[7] = 0.0f;                          mat[11] = 0.0f;                             mat[15] = 1.0f;
    }

    // Updates minimum and maximum coordinates of the trajectory, used in order to scale the viewport accordingly
    void update_min_max(rs2_vector point)
    {
        max_coord.x = std::max(max_coord.x, point.x);
        max_coord.y = std::max(max_coord.y, point.y);
        max_coord.z = std::max(max_coord.z, point.z);
        min_coord.x = std::min(min_coord.x, point.x);
        min_coord.y = std::min(min_coord.y, point.y);
        min_coord.z = std::min(min_coord.z, point.z);
    }

    // Register new point in the trajectory
    void add_to_trajectory(tracked_point& p)
    {
        // If first element, add to trajectory and initialize minimum and maximum coordinates
        if (trajectory.size() == 0)
        {
            trajectory.push_back(p);
            max_coord = p.point;
            min_coord = p.point;
        }
        else
        {
            // Check if new element is far enough - more than 1 mm (to keep trajectory vector as small as possible)
            rs2_vector prev = trajectory.back().point;
            rs2_vector curr = p.point;
            if (sqrt(pow((curr.x - prev.x), 2) + pow((curr.y - prev.y), 2) + pow((curr.z - prev.z), 2)) < 0.001)
            {
                // If new point is too close to previous point and has higher confidence, replace the previous point with the new one
                if (p.confidence > trajectory.back().confidence)
                {
                    trajectory.back() = p;
                    update_min_max(p.point);
                }
            }
            else
            {
                // If new point is far enough, add it to trajectory
                trajectory.push_back(p);
                update_min_max(p.point);
            }
        }
    }

    // Draw trajectory with colors according to point confidence
    void draw_trajectory()
    {
        float3 colors[]{
            { 0.7f, 0.7f, 0.7f },
            { 1.0f, 0.0f, 0.0f },
            { 1.0f, 1.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
        };

        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        for (auto&& v : trajectory)
        {
            auto c = colors[v.confidence];
            glColor3f(c.x, c.y, c.z);
            glVertex3f(v.point.x, v.point.y, v.point.z);
        }
        glEnd();
        glLineWidth(0.5f);
    }

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RStracking *) This)->update();
		return NULL;
	}

public:
	bool	m_bOpen;
    std::vector<tracked_point> trajectory;
    rs2_vector max_coord;
    rs2_vector min_coord;

    rs2::pipeline m_rsPipe;

};

}
#endif
#endif
