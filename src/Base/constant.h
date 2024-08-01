/*
 * constant.h
 *
 *  Created on: Nov 21, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base_constant_H_
#define OpenKAI_src_Base_constant_H_

#define DEFAULT_FPS 30
#define OK_PI 3.1415926535897932384626433832795
#define OneOvPI 0.31830988618
#define DEG_2_RAD 0.0174532925199433
#define RAD_2_DEG 57.29577951308233
#define SEC_2_USEC 1000000
#define USEC_2_SEC 0.000001
#define USEC_1SEC 1000000
#define USEC_10SEC 10000000
#define NSEC_1SEC 1000000000
#define DEG_AROUND 360.0
#define RAD_AROUND 6.283188

#define CR '\x0d'
#define LF '\x0a'

#define _Console_COL_TITLE 1
#define _Console_COL_NAME 2
#define _Console_COL_FPS 3
#define _Console_COL_MSG 4
#define _Console_COL_ERROR 5
#define _Console_X_NAME 0
#define _Console_X_FPS 18
#define _Console_X_MSG 30

#define OK_OK 0
#define OK_ERR_UNKNOWN 1024
#define OK_ERR_NULLPTR 1025
#define OK_ERR_UNIMPLEMENTED 1026
#define OK_ERR_DUPLICATE 1027
#define OK_ERR_INVALID_VALUE 1028
#define OK_ERR_NOT_FOUND 1029
#define OK_ERR_ALLOCATION 1030
#define OK_ERR_NOT_READY 1031




#define F_OPEN 0
#define F_READY 1
#define F_RESET 2
#define F_ERROR 3

#endif
