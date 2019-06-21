/*
 * constant.h
 *
 *  Created on: Nov 21, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base_constant_H_
#define OpenKAI_src_Base_constant_H_

#define DEFAULT_FPS 30

#define PI 3.141592653589793
#define OneOvPI 0.31830988618
#define DEG_RAD 0.0174533
#define RAD_DEG 57.29578
#define USEC_1SEC 1e6
#define OV_USEC_1SEC 1e-6
#define USEC_10SEC 1e7
#define NSEC_1SEC 1000000000
#define DEG_AROUND 360.0
#define RAD_AROUND 6.283188

#define CR '\x0d'
#define LF '\x0a'

#define CONSOLE_COL_TITLE 1
#define CONSOLE_COL_NAME 2
#define CONSOLE_COL_FPS 3
#define CONSOLE_COL_MSG 4
#define CONSOLE_COL_ERROR 5
#define COL_TITLE attrset(COLOR_PAIR(CONSOLE_COL_TITLE)|A_BOLD)
#define COL_NAME attrset(COLOR_PAIR(CONSOLE_COL_NAME)|A_BOLD)
#define COL_FPS attrset(COLOR_PAIR(CONSOLE_COL_FPS))
#define COL_MSG attrset(COLOR_PAIR(CONSOLE_COL_MSG))
#define COL_ERROR attrset(COLOR_PAIR(CONSOLE_COL_ERROR)|A_BOLD)
#define CONSOLE_X_NAME 0
#define CONSOLE_X_FPS 20
#define CONSOLE_X_MSG 30

#endif
