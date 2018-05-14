/*
 * constant.h
 *
 *  Created on: Nov 21, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_BASE_CONSTANT_H_
#define OPENKAI_SRC_BASE_CONSTANT_H_

#define N_INST 128
#define DEFAULT_FPS 30

#define PI 3.141592653589793
#define OneOvPI 0.31830988618
#define DEG_RAD 0.0174533
#define RAD_DEG 57.29578
#define USEC_1SEC 1000000
#define USEC_10SEC 10000000
#define NSEC_1SEC 1000000000
#define DEG_AROUND 360.0
#define RAD_AROUND 6.283188

#define CR '\x0d'
#define LF '\x0a'

#define CLI_COL_TITLE 1
#define CLI_COL_NAME 2
#define CLI_COL_FPS 3
#define CLI_COL_MSG 4
#define COL_TITLE attrset(COLOR_PAIR(CLI_COL_TITLE)|A_BOLD)
#define COL_NAME attrset(COLOR_PAIR(CLI_COL_NAME)|A_BOLD)
#define COL_FPS attrset(COLOR_PAIR(CLI_COL_FPS))
#define COL_MSG attrset(COLOR_PAIR(CLI_COL_MSG))
#define CLI_X_NAME 0
#define CLI_X_FPS 20
#define CLI_X_MSG 30

#endif
