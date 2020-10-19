#ifndef HPS3D_API_H_
#define HPS3D_API_H_

#ifdef __cplusplus
extern "C"  /*C++*/
{
#endif

#include <stdbool.h>
#include <string.h>


#ifdef _WIN32
	#include <windows.h>
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif

#define DLL_API _declspec(dllexport)

typedef signed char 	int8_t;
typedef unsigned char 	uint8_t;
typedef unsigned short 	uint16_t;
typedef short 			int16_t;
typedef unsigned int 	uint32_t;
typedef int 			int32_t;
typedef float 			float32_t;
typedef double 			float64_t;

/***********************************************************************************/
/*User can modify the parameter,odify parameter values will affect the occupation of the SDK memory. Refer to SDK manual(HPS3D_RM002) for detail instructions. 4 Support mult-devices connection  */
#define 	DEV_NUM 			 (10)							/*Device number supported*/
#define     DEV_NAME_SIZE		 (20)							/*Device name size*/
#define 	ROI_GROUP_NUMBER     (16)							/*Number of ROI group*/
#define 	ROI_NUM 			 (8)							/*Number of ROI*/
#define 	OBSTACLE_NUM 		 (20)							/*Number of obstacles supported*/
#define 	OBSERVER_NUM  		 (10)							/*Number of observers*/
/**********************************************************************************/
/*Parameters for internal use only, do not modify*/
/*	Camera resolution */
#define		RES_WIDTH			 (160)
#define		RES_HEIGHT			 (60)
#define		MAX_PIX_NUM 		 (9600)    /*RES_WIDTH * RES_HEIGHT*/

/*Indicates the invalid data and meaning of current measuring pixel*/
#define	 	LOW_AMPLITUDE   	(65300) 						/*Low amplitude*/
#define	  	SATURATION 			(65400)     					/*Saturation*/
#define	 	ADC_OVERFLOW  		(65500)   						/*ADC overflow*/
#define	 	INVALID_DATA 		(65530)    						/*Indalid data*/
/**********************************************************************************/

/*The result returned by the function*/
typedef enum
{
	RET_OK 		= 0x01,
	RET_ERROR 	= 0x02,
	RET_BUSY 	= 0x03,
	RET_CONNECT_FAILED,
	RET_CREAT_PTHREAD_ERR,  /*Thread creation failed*/
	RET_WRITE_ERR,
	RET_READ_ERR,
	RET_PACKET_HEAD_ERR,
	RET_PACKET_ERR,
	RET_BUFF_EMPTY,			/*Buffer is empty or unavailable*/
	RET_VER_MISMATCH,  		/*Camera firmware version does not match SDK version*/
}RET_StatusTypeDef;

/*Device version*/
typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t major;
	uint8_t minor;
	uint8_t rev;
}Version_t;

/*Run mode*/
typedef enum
{
	MinOfRunModeType = 0,
	RUN_IDLE = 0,			/*Standby or stop measuring*/
	RUN_SINGLE_SHOT,		/*Single measurement*/
	RUN_CONTINUOUS,			/*Continuous measurement*/
	NumberOfRunModeType
}RunModeTypeDef;

/*Set measurement data package type*/
typedef enum
{
	DEPTH_DATA_PACKET = 0x0,  /*Depth data packet  @see DepthDataTypeDef*/
	ROI_DATA_PACKET,		  /*ROI data packet  @see FullRoiDataTypeDef  @see SimpleRoiDataTypeDef*/
	OBSTACLE_PACKE			  /*Obstacle data type  @see ObstacleDataTypedef*/
}MeasurePacketTypeDef;


/*Output data packet type*/
typedef enum
{
	PACKET_FULL = 0,		/*Full data packet(includes depth data)*/
	PACKET_SIMPLE			/*Simple data packet(depth data not included*/
}OutPacketTypeDef;

/*ROI threshold alarm type*/
typedef enum
{
	ROI_ALARM_DISABLE = 0,  /* Disable ROI GPIO alam*/
	ROI_ALARM_GPIO       	/*ROI alam type is GPIO signal output*/
}ROIAlarmTypeDef;

/*Hysteresis configuration*/
typedef struct
{
	uint8_t threshold_id;  		    /*ROI threshold id*/
	uint32_t threshold_value; 		/*ROI threshold value*/
	uint32_t hysteresis; 			/*Hysteresis value*/
	bool enable;					/*Enable hysteresis*/
	bool positive;					/*true:Positive comparison, return True if the input value is larger than the threshold
									false:Reverse comparison, return False if the input value is smaller than the threshold*/
}HysteresisSingleConfTypeDef;

/*ROI reference value type*/
typedef enum
{
	ROI_REF_DIST_AVR = 1,
	ROI_REF_DIST_MIN,
	ROI_REF_DIST_MAX,
	ROI_REF_SAT_COUNT,
	ROI_REF_AMPLITUDE,
	ROI_REF_VAILD_AMPLITUDE,
	ROI_REF_THRESHOLD_PIX_NUM
}ROIReferenceTypeDef;

/*ROI configured structure*/
 typedef struct
{
	bool enable;
	uint8_t roi_id;
	uint16_t left_top_x;
	uint16_t left_top_y;
	uint16_t right_bottom_x;
	uint16_t right_bottom_y;
	HysteresisSingleConfTypeDef hysteresis_conf[3];
	ROIReferenceTypeDef ref_type[3];					/*ROI reference value type corresponds to hysteresis_conf*/
	ROIAlarmTypeDef alarm_type[3];						/*ROI threshold alarm type corresponds to hysteresis_conf*/
	uint16_t pixel_number_threshold[3];					/*Pixel number of over threshold corresponds to hysteresis_conf*/
}ROIConfTypeDef;

/*HDR Mode*/
typedef enum
{
	HDR_DISABLE = 0,				/*Not recommended for use*/
	AUTO_HDR,						/*Not recommended for use*/
	SUPER_HDR,
	SIMPLE_HDR
}HDRModeTypeDef;

/*HDR Configuration*/
typedef struct
{
	HDRModeTypeDef hdr_mode;
	float32_t qualtity_overexposed;				/*AUTO_HDR Overexposed amplitude, this mode is not recommended*/
	float32_t qualtity_overexposed_serious;		/*AUTO_HDR Serious overexposed amplitude, this mode is not recommended*/
	float32_t qualtity_weak;					/*AUTO_HDR Weak signal amplitude, this mode is not recommended*/
	float32_t qualtity_weak_serious;			/*AUTO_HDR Serious weak signal amplitude, this mode is not recommended*/
	uint32_t simple_hdr_max_integration;		/*SIMPLE_HDR Maximum integration time us*/
	uint32_t simple_hdr_min_integration;		/*SIMPLE_HDR Minimum integration time us*/
	uint8_t super_hdr_frame_number;				/*SUPER_HDR Synthetic frame numbe*/
	uint32_t super_hdr_max_integration;			/*SUPER_HDR Maximum integration time us*/
	uint32_t hdr_disable_integration_time;		/*HDR_DISABLE Manual integration time us this mode is not recommended*/
}HDRConf;

/*Smooth filter type*/
typedef enum
{
	SMOOTH_FILTER_DISABLE = 0,		/*Disalbe smooth filter*/
	SMOOTH_FILTER_AVERAGE = 1,		/*Average filter*/
	SMOOTH_FILTER_GAUSS				/*Gauss filter*/
}SmoothFilterTypeDef;

/*Smooth filter configuration*/
typedef struct
{
	SmoothFilterTypeDef type;		/*Smooth filter type*/
	uint32_t arg1;					/*Filter parameter*/
}SmoothFilterConfTypeDef;

/*uart conifg param*/
typedef enum
{
	UART_BAUDRATE_9600 = 9600,
	UART_BAUDRATE_19200 = 19200,
	UART_BAUDRATE_38400 = 38400,
	UART_BAUDRATE_57600 = 57600,
	UART_BAUDRATE_115200 = 115200,
	UART_BAUDRATE_230400 = 230400,
	UART_BAUDRATE_460800 = 460800,
	UART_BAUDRATE_1000000 = 1000000,
	UART_BAUDRATE_1500000 = 1500000,
	UART_BAUDRATE_2000000 = 2000000,
}UartBaudrateTypeDef;

typedef enum
{
	PARITY_NONE1,
	PARITY_ODD1,
	PARITY_EVEN1,
}UartParityTypeDef;

typedef enum
{
	UART_STOP_BITS0_5,
	UART_STOP_BITS1,
	UART_STOP_BITS1_5,
	UART_STOP_BITS2,
}UartStopBitTypeDef;

typedef struct
{
	UartBaudrateTypeDef baudrate;
	UartParityTypeDef parity;
	UartStopBitTypeDef stop_bits;
}UartConfTypeDef;

/*GPIO configuration definition*/
/*GPIO_OUT function*/
typedef enum
{
	GPOUT_FUNC_DISABLE = 0,				/*GPIO alam disalbe*/
	GPOUT_FUNC_ROI_THRESHOLD0_ALARM,	/*GPIO output threshold 0 alarm*/
	GPOUT_FUNC_ROI_THRESHOLD1_ALARM,	/*GPIO output threshold 1 alarm*/
	GPOUT_FUNC_ROI_THRESHOLD2_ALARM		/*GPIO output threshold 2 alarm*/
}GPOutFunctionTypeDef;

/*GPIO_IN function*/
typedef enum
{
	GPIN_FUNC_DISABLE = 0,			/*GPIO function disable*/
	GPIN_FUNC_CAPTURE   			/*Start measuring. Note:After the measurement is turned on, it is not controlled by the command and is controlled only by the IO input.*/
}GPInFunctionTypeDef;

/*GPIO Polarity*/
typedef enum
{
	GPIO_POLARITY_LOW = 0,
	GPIO_POLARITY_HIGH
}GPIOPolarityTypeDef;

/*GPIO Pin*/
typedef enum
{
    GPIN_1 = 1,
    GPIN_2 = 2,
    GPIN_3 = 3,
    GPOUT_1 = 10,
    GPOUT_2 = 11,
    GPOUT_3 = 12,
}GPIOTypeDef;

/*GPIO output configuration*/
typedef struct
{
	GPIOTypeDef gpio;
	GPIOPolarityTypeDef polarity;
	GPOutFunctionTypeDef function;
}GPIOOutConfTypeDef;

/*GPIO input configuration*/
typedef struct
{
	GPIOTypeDef gpio;
	GPIOPolarityTypeDef polarity;
	GPInFunctionTypeDef function;
}GPIOInConfTypeDef;


/*Depth filter type*/
typedef enum
{
	DISTANCE_FILTER_DISABLE = 0,
	DISTANCE_FILTER_SIMPLE_KALMAN			/*Simple Kalman filter*/
}DistanceFilterTypeDef;

typedef struct
{
	DistanceFilterTypeDef filter_type;		/*@see DistanceFilterTypeDef*/
	float32_t kalman_K; 					/*Scaling factor K. recommended value 0.5*/
	uint32_t kalman_threshold;				/*Noise threshold*/
	uint32_t num_check;						/*Threshold check frame number, recommended value 2*/
}DistanceFilterConfTypeDef;

/*Installation angle change parameters, for rotating coordinate system use*/
typedef struct
{
	bool enable;					/*Enable installation angle*/
	uint8_t angle_vertical;     	/*Vertical installation angle（°）:The angle between the main optical axis and the ground perpendicula*/
	uint16_t height;				/*Mounting height relative to the ground(mm)*/
}MountingAngleParamTypeDef;

/*Returned data packet type*/
typedef enum
{
	NULL_PACKET = 0x00,				/*Data returns empty means no measurement data is returned*/
	SIMPLE_ROI_PACKET = 0x01,		/*Simple ROI data packet(depth data not included @see SimpleRoiDataTypeDef*/
	FULL_ROI_PACKET,				/*Full ROI packet(includes depth data)@see FullRoiDataTypeDef*/
	FULL_DEPTH_PACKET,				/*Full depth data packet(includes depth data)@see DepthDataTypeDef*/
	SIMPLE_DEPTH_PACKET,			/*Simple depth data packet(depth data not included)@see DepthDataTypeDef*/
	KEEP_ALIVE_PACKET,				/*KEEP Alive*/
	OBSTACLE_PACKET,				/*Obstacle data packet @see ObstacleDataTypedef*/
    SYSTEM_ERROR					/*System error*/
}RetPacketTypedef;

/*ROI simple data packet*/
typedef struct
{
	uint8_t group_id;						/*group ID*/
	uint8_t id;								/*ROI ID*/
	uint16_t amplitude;						/*average amplitude*/
	uint16_t valid_amplitude;				/*average valid amplitude*/
	uint16_t distance_average;				/*average distance*/
	uint16_t distance_max;					/*maximum distance*/
	uint16_t distance_min;					/*minimum distance*/
	uint16_t dist_max_x;					/*Unavailable*/
	uint16_t dist_max_y;					/*Unavailable*/
	uint16_t dist_min_x;					/*Unavailable*/
	uint16_t dist_min_y;					/*Unavailable*/
	uint16_t saturation_count;				/*Number of saturated pixels*/
	uint8_t threshold_state;				/*Alarm indicator, indicates if the current measured value exceeds the threshold :bit0:zone0, bit1:zone1, bit2:zone2*/
	uint16_t out_of_threshold_pix_num[3];	/*[0]:pixel number exceed thresold0,[1]:...,[2]:...*/
	uint16_t frame_cnt;						/*Frame counter*/
}SimpleRoiDataTypeDef;

/*Full ROI data packet*/
typedef struct
{
	uint8_t roi_num;						/*ROI total number*/
	uint8_t group_id;						/*group ID*/
	uint8_t id;								/*ROI ID*/
	uint16_t left_top_x;					/*Upper left corner x coordinate*/
	uint16_t left_top_y;					/*Upper left corner y coordinate*/
	uint16_t right_bottom_x;				/*Right lower corner x coordinate*/
	uint16_t right_bottom_y;				/*Right lower corner y coordinate*/
	uint32_t pixel_number;					/*ROI pixel number*/
	uint16_t amplitude;						/*Average amplitude*/
	uint16_t valid_amplitude;				/*Average valid amplitute*/
	uint16_t distance_average;				/*Average distance*/
	uint16_t distance_max;					/*Unavailable*/
	uint16_t distance_min;					/*Minimum distance*/
	uint16_t saturation_count;				/*Number of saturated pixels*/
	uint16_t threshold_state;				/*if the current measured value exceeds the threshold:bit0:zone0, bit1:zone1, bit2:zone2*/
	uint16_t dist_max_x;					/*Unavailable*/
	uint16_t dist_max_y;					/*Unavailable*/
	uint16_t dist_min_x;					/*Unavailable*/
	uint16_t dist_min_y;					/*Unavailable*/
	uint32_t frame_cnt;						/*Frame counter*/
	uint16_t *distance;						/*Depth data, store in order*/
}FullRoiDataTypeDef;

/*Depth data*/
typedef struct
{
	uint16_t distance_average;				/*Average distance value of the whole field of view*/
	uint16_t amplitude_average;				/*Valid amplitude, invalid pixels not included*/
	uint16_t amplitude_average_whole;		/*Average amplitude value of the whole field of view*/
	uint16_t amplitude_low_count;			/*Number of low amplitute pixel*/
	uint16_t saturation_count;				/*Number of saturated pixels*/
	uint16_t distance_max;					/*Unavailable*/
	uint16_t distance_min;					/*Minimum estimated distance*/
	int16_t temperature;					/*Camera current reference temperature*/
	uint16_t frame_cnt;						/*Frame counter,can be used for frame loss detection*/
	uint16_t interference_num; 				/*Unavailable*/
	uint16_t *distance;						/*Depth data, store in order, not available when the output data type is @see PACKET_SIMPLE*/
}DepthDataTypeDef;


typedef struct
{
	float32_t x;
	float32_t y;
	float32_t z;
}PerPointCloudDataTypeDef;

/*Ordered point cloud data*/
typedef struct
{
	PerPointCloudDataTypeDef *point_data;
	uint16_t width;
	uint16_t height;
	uint32_t points;
}PointCloudDataTypeDef;

/*Obstacle configuration parameters*/
typedef struct
{
	bool enable;
	uint16_t frame_head; 			/*Data frame, frame header feature byte, such as 0XEB81*/
	uint8_t number;  				/*The number of obstacles that need to be extracted, for example 3*/
	uint16_t vaild_min_dist; 		/*The minimum distance of the effective range in mm, for example 60*/
	uint16_t vaild_max_dist; 		/*The maximum distance of the effective range in mm, for example 3500*/
	uint16_t invaild_value;  		/*Fixed parameter value of invalid area in mm eg 5000*/
	uint16_t frame_size;			/*Save the current buffer valid bytes*/
}ObstacleConfigTypedef;

/*Obstacle data*/
typedef struct
{
	uint8_t ObstacleNumber;								/*Obstacle total number*/
	uint8_t Id;											/*Obstacle ID*/
	uint32_t FrameCount;  								/*Framce count value*/
	uint16_t PixelNumber;
	uint16_t DistanceAverage;
	PerPointCloudDataTypeDef LeftPoint;
	PerPointCloudDataTypeDef RightPoint;
	PerPointCloudDataTypeDef UpperPoint;
	PerPointCloudDataTypeDef UnderPoint;
	PerPointCloudDataTypeDef MinPoint;
	PerPointCloudDataTypeDef *PixelBuffer;   /*Save all pixel information for obstacles*/
}ObstacleDataTypedef;

/*Obstacle avoidance parameter structure*/
typedef struct
{
	bool avoidance_enable;
	uint32_t avoidance_width;
	uint32_t avoidance_height;
	uint32_t avoidance_minDistance;
	uint32_t avoidance_mountHeight; /*Mount height*/
	uint32_t avoidance_pixelNumber; /*Pixcel threshold number*/
	bool avoidance_alarmStatues;    /*Alarm status*/
	GPIOOutConfTypeDef GpioOutConf; /*GPIO function*/
}AvoidanceTypeDef;

/*Data return*/
typedef struct
{
	SimpleRoiDataTypeDef *simple_roi_data;		/*Simple ROI data packet @see SimpleRoiDataTypeDef*/
	FullRoiDataTypeDef *full_roi_data;			/*Full ROI data packet @see FullRoiDataTypeDef*/
	DepthDataTypeDef *simple_depth_data;		/*Simple depth data packet  @see DepthDataTypeDef*/
	DepthDataTypeDef *full_depth_data;			/*Full ROI data packet @see DepthDataTypeDef*/
	PointCloudDataTypeDef *point_cloud_data;	/*Point cloud data packet @see PointCloudDataTypeDef*/
	ObstacleDataTypedef *Obstacle_data;			/*Obstacle data packet @see ObstacleDataTypedef*/
	uint8_t *Obstacle_frame_data_buff; 			/*Buffer used to store obstacle data packet*/
}MeasureDataTypeDef;


typedef enum
{
	SYNC = 0x01,  						/*Synchronous mode*/
	ASYNC = 0x02 						/*Asynchronous mode*/
}HPS3D_SynchronousTypedef;

/*Transport Type*/
typedef enum
{
	TRANSPORT_USB = 0,
	TRANSPORT_CAN,
	TRANSPORT_RS232,
	TRANSPORT_RS485,
	TRANSPORT_ETHERNET
}TransportTypeDef;

/*handle*/
typedef struct
{
	char *DeviceName; 					/*R/W Device name*/
	uint32_t DeviceFd; 					/*R   Not editable*/
	uint8_t DeviceAddr; 				/*R   Store the device address (also the frame ID) of the currently connected device*/
	uint8_t ConnectionNumber;			/*R   Not editable*/
	HPS3D_SynchronousTypedef SyncMode;  /*R   Synchronous or asynchronous mode*/
	RunModeTypeDef RunMode;   			/*R/W Run mode*/
	MeasureDataTypeDef MeasureData;     /*R   Synchronous measurement data, when measured in asynchronous mode, the measurement results are not saved here (can be operated by the observer)*/
	RetPacketTypedef RetPacketType;     /*R   Synchronous measurement returns the packet type. When the asynchronous mode is measured, the result of returning the packet type will not be saved here (can be operated by the observer)*/
	OutPacketTypeDef OutputPacketType; 	/*R   Output packet type, not editable*/
	bool ConnectStatus;  	 			/*R   Connect status, not editable*/
	uint8_t RoiNumber;					/*R   Save the number of ROI supported by the device*/
	uint8_t ThresholdNumber;			/*R   Save the number of threshold supported by the device ROI*/
	uint8_t ViewAngleHorizontal;  		/*R   Horizontal view of angle, not editable*/
	uint8_t ViewAngleVertical;			/*R   Vertical view of angle, no editable*/
	struct sockaddr_in ServerAddr;	    /*R/W Sever ip address and port number*/
	TransportTypeDef TransportType;     /*R   当Current transport type*/
}HPS3D_HandleTypeDef;

/*Optical parameters*/
typedef struct
{
	bool enable;						/*Optical parameters enable(Turn on and the measured depth data is the vertical distance)*/
	uint8_t viewing_angle_horiz;    	/*Horizontal view of angle*/
	uint8_t viewing_angle_vertical; 	/*Vertical view of angle*/
	uint8_t illum_angle_horiz;
	uint8_t illum_angle_vertical;
}OpticalParamConfTypeDef;

/*mult-devices mutual interference configuration, not editable*/
typedef struct
{
	bool enable;
	uint32_t integ_time;
	uint16_t amplitude_threshold;
	uint8_t capture_num;
	uint8_t number_check;
}InterferenceDetectConfTypeDef;


/*Details for observer mode, please refer to https://www.cnblogs.com/luohanguo/p/7825656.html
 * */
/*Observer subscription event*/
typedef enum
{
	ISubject_Event_DataRecvd = 1<<0,	/*Data receive event*/
	ISubject_Event_DevConnect = 1<<1,	/*Connect even*/
	ISubject_Event_DevDisconnect = 1<<2 /*Disconnect event*/
}AsyncISubjectEvent;

/*Observer subscription event structure parameters*/
typedef struct
{
	uint8_t ObserverID;
	bool NotifyEnable;
	AsyncISubjectEvent AsyncEvent; 		/*Observer subscription event*/
	MeasureDataTypeDef MeasureData; 	/*To store measure data*/
	RetPacketTypedef RetPacketType; 	/*Measure return packet type*/
}AsyncIObserver_t;

/*Point cloud data mirror*/
typedef enum
{
	MIRROR_DISABLE = 0X0,
	MIRROR_HORIZONTAL = 0X1,
	MIRROR_VERTICLA = 0X2,
	MIRROR_HORIZONTAL_VERTICLA = 0X3,
}PointCloudImageMirrorTypeDef;

/**************************************Function interface*************************************/

/***********************************1.Command function interface***********************************/
/**
 * @brief	Set measured mode
 * @param[in]
 * @note    Manually modify the handle->RunMode value before calling this function，
 * 			And this function works only after the device initialization is completed, @see HPS3D_ConfigInit
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetRunMode(HPS3D_HandleTypeDef *handle);

/**
 * @brief	Get device address
 * @param[out]  handle->DeviceAddr  Output device address
 * @note    Device address( also the frame ID)
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetDevAddr(HPS3D_HandleTypeDef *handle);

/**
 * @brief	Set device address
 * @param[in]	handle->DeviceAddr   Old device address
 * @param[in]	new_addr     		  New device address
 * @note	Multiple devices can be distinguished by modifying this parameter
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetDevAddr(HPS3D_HandleTypeDef *handle, uint8_t new_addr);

/**
 * @brief	Get device version information
 * @param[out]  version_t
 * @note	Get device firmware version information
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetDeviceVersion(HPS3D_HandleTypeDef *handle, Version_t *version_t);

/**
 * @brief	Set measurer data packet type
 * @param[in]	type
 * @note	This configuration needs to be done before @see HPS3D_ConfigInit is initialized. The default is full depth packet
 * @see
 * @code
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetMeasurePacketType(MeasurePacketTypeDef type);
/**
 * @brief	Get measure data packet type
 * @param
 * @note	Default is full depth data packet
 * @see		MeasurePacketTypeDef
 * @code
 * @retval	Return type enumeration value
 */
extern MeasurePacketTypeDef HPS3D_GetMeasurePacketType(void);


/**
 * @brief	Set measure data return packet type(simple or full packet)
 * @param[in] outPacketType
 * @note    Set output data packet as simple data packet or full data packet
 * @see		OutPacketTypeDef
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetPacketType(HPS3D_HandleTypeDef *handle,OutPacketTypeDef outPacketType);

/**
 * @brief	Get data packet type
 * @param[out]	handle->PacketType
 * @note
 * @see		OutPacketTypeDef
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetPacketType(HPS3D_HandleTypeDef *handle);


/**
 * @brief	Save conmmunication configuration
 * @note	Communication parameters will take effect permanently after using this function, please keep in mind the current configuration (use with caution)
 * @see
 * @code
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SaveTransportConf(HPS3D_HandleTypeDef *handle);

/**
 * @brief	Save to user configuration parameters
 * @param
 * @note	This function will make the current configuration permanent, and you can reset to the default configuration by clearing the configuration or reset factor setting
 * @see		HPS3D_ProfileClearCurrent HPS3D_ProfileRestoreFactory
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_ProfileSaveToCurrent(HPS3D_HandleTypeDef *handle);

/**
 * @brief	Clear user configuration parameters
 * @param
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_ProfileClearCurrent(HPS3D_HandleTypeDef *handle);

/**
 * @brief	Reset factory setting
 * @param
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_ProfileRestoreFactory(HPS3D_HandleTypeDef *handle);

/**
 * @brief	Get transport type
 * @param[out]	transport_type
 * @note
 * @see	TransportTypeDef
 * @retval	Return OK RET_OK
 * @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_GetTransportType(HPS3D_HandleTypeDef *handle, TransportTypeDef *transport_type);

/**
 * @brief	Select ROI group
 * @param[in]	group_id
 * @note
 * @retval	Return OK RET_OK
 * @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_SelectROIGroup(HPS3D_HandleTypeDef *handle, uint8_t group_id);

/**
 * @brief	Get current ROI group ID
 * @param[out]  group_id
 * @note
 * @retval  Return OK RET_OK
 * @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_GetROIGroupID(HPS3D_HandleTypeDef *handle, uint8_t *group_id);

/**
 * @brief	Set ROI alarm type
 * @param[in]
 * @see ROIAlarmTypeDef
 * @note
 * @retval	Return OK RET_OK @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_SetROIAlarmType(HPS3D_HandleTypeDef *handle, uint8_t roi_id, uint8_t threshold_id, ROIAlarmTypeDef roi_alarm_type);

/**
 * @brief	Set ROI reference type
 * @param[in]
 * @see ROIReferenceTypeDef
 * @note
 * @retval	Return OK RET_OK @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_SetROIReferenceType(HPS3D_HandleTypeDef *handle, uint8_t roi_id, uint8_t threshold_id, ROIReferenceTypeDef ref_type);

/**
 * @brief	Set ROI area
 * @param[in]	roi_conf
 * @see ROIConfTypeDef
 * @note
 * @retval	Return OK RET_OK @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_SetROIRegion(HPS3D_HandleTypeDef *handle, ROIConfTypeDef roi_conf);

/**
 * @brief	Set ROI enable
 * @param[in]	roi_id
 * @param[in]	en
 * @note
 * @retval	Return OK RET_OK @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_SetROIEnable(HPS3D_HandleTypeDef *handle, uint32_t roi_id, bool en);

/**
 * @brief	Set ROI threshold enable
 * @param[in]	roi_id
 * @param[in]	threshold_id
 * @param[in]	en
 * @note
 * @retval	Return OK RET_OK @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_SetROIThresholdEnable(HPS3D_HandleTypeDef *handle, uint32_t roi_id, uint32_t threshold_id, bool en);

/**
 * @brief	Set ROI threshold configuration
 * @param[in]	hysteresis_conf
 * @see HysteresisSingleConfTypeDef
 * @note
 * @retval	Return OK RET_OK @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_SetROIThresholdConf(HPS3D_HandleTypeDef *handle, uint32_t roi_id, uint32_t threshold_id, uint16_t pix_num_threshold, HysteresisSingleConfTypeDef hysteresis_conf);

/**
 * @brief	Get the number of ROIs and thresholds supported by the current device
 * @param[out]	roi_number
 * @param[out]	threshold_number
 * @note
 * @retval	Return OK RET_OK @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_GetNumberOfROI(HPS3D_HandleTypeDef *handle, uint8_t *roi_number, uint8_t *threshold_number);

/**
 * @brief	Get the specified ROI configuration
 * @param[out]	roi_conf
 * @see ROIConfTypeDef
 * @note
 * @retval	Return OK RET_OK @see RET_StatusTypeDef
 */
extern RET_StatusTypeDef HPS3D_GetROIConfById(HPS3D_HandleTypeDef *handle, uint8_t roi_id, ROIConfTypeDef *roi_conf);

/**
 * @brief	Set the configuration of the specified GPIO output port
 * @param[in]	gpio_out_conf
 * @see GPIOOutConfTypeDef
 * @note        gpio_out_conf：Can only configure IO output
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetGPIOOut(HPS3D_HandleTypeDef *handle, GPIOOutConfTypeDef gpio_out_conf);

/**
 * @brief	Get the configuration of the specified GPIO output port
 * @param[in]	gpio_out_conf
 * @see GPIOOutConfTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetGPIOOutConf(HPS3D_HandleTypeDef *handle, GPIOOutConfTypeDef *gpio_out_conf);

/**
 * @brief	Set the configuration of the specified GPIO input port
 * @param[in]	gpio_in_conf
 * @see GPIOInConfTypeDef
 * @note	gpio_in_conf：Can only configure IO input
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetGPIOIn(HPS3D_HandleTypeDef *handle, GPIOInConfTypeDef gpio_in_conf);

/**
 * @brief	Get the configuration of the specified GPIO input port
 * @param[out]	gpio_in_conf
 * @see GPIOInConfTypeDef
 * @note
 * @retval	 Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetGPIOInConf(HPS3D_HandleTypeDef *handle, GPIOInConfTypeDef *gpio_in_conf);

/**
 * @brief	Set HDR mode
 * @param[in]	hdr_mode
 * @see HDRModeTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetHDRMode(HPS3D_HandleTypeDef *handle, HDRModeTypeDef hdr_mode);

/**
 * @brief	Set HDR
 * @param[in] hdr_conf
 * @see 	HDRConf
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetHDRConfig(HPS3D_HandleTypeDef *handle, HDRConf hdr_conf);

/**
 * @brief	Get HDR configuration
 * @param[out] hdr_conf
 * @see		HDRConf
 * @note
 * @retval	 Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetHDRConfig(HPS3D_HandleTypeDef *handle, HDRConf *hdr_conf);

/**
 * @brief	Set the distance filter type
 * @param[in]	distance_filter_conf
 * @see	DistanceFilterTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetDistanceFilterType(HPS3D_HandleTypeDef *handle, DistanceFilterTypeDef distance_filter_conf);

/**
 * @brief	Configurate distance filter
 * @param[in]	distance_filter_conf
 * @see DistanceFilterConfTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetSimpleKalman(HPS3D_HandleTypeDef *handle, DistanceFilterConfTypeDef distance_filter_conf);

/**
 * @brief	Get distance filter configuration
 * @param[out]	distance_filter_conf
 * @see		DistanceFilterConfTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetDistanceFilterConf(HPS3D_HandleTypeDef *handle, DistanceFilterConfTypeDef *distance_filter_conf);

/**
 * @brief	Set the smooth filter
 * @param[in]	smooth_filter_conf
 * @see	SmoothFilterConfTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetSmoothFilter(HPS3D_HandleTypeDef *handle, SmoothFilterConfTypeDef smooth_filter_conf);

/**
 * @brief	Get the configuration of the smooth filter
 * @param[out]	smooth_filter_conf
 * @see	SmoothFilterConfTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetSmoothFilterConf(HPS3D_HandleTypeDef *handle, SmoothFilterConfTypeDef *smooth_filter_conf);

/**
 * @brief	Set optical parameter enable
 * @param[in]	en
 * @note	Optical parameter compensation is to convert the actual optical path into a horizontal distance
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetOpticalEnable(HPS3D_HandleTypeDef *handle, bool en);

/**
 * @brief	Get optical parameters
 * @param[out]	optical_param_conf
 * @see	OpticalParamConfTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetOpticalParamConf(HPS3D_HandleTypeDef *handle, OpticalParamConfTypeDef *optical_param_conf);

/**
* @brief	Set distance compensation
* @note
* @retval	Return OK RET_OK
*/
extern RET_StatusTypeDef HPS3D_SetDistanceOffset(HPS3D_HandleTypeDef *handle, int16_t offset);

/**
* @brief	Get distance compensation
* @param[out]	offset
* @note
* @retval	Return OK RET_OK
*/
extern RET_StatusTypeDef HPS3D_GetDistanceOffset(HPS3D_HandleTypeDef *handle, int16_t *offset);

/**
 * @brief	Set multi-devices multual interference detection enable
 * @param[in]	en
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetInterferenceDetectEn(HPS3D_HandleTypeDef *handle, bool en);


/**
 * @brief	Set the installation angle change enable
 * @param[in]	en
 * @note
 * @retval	Return OK RET_OK
 */
/*extern RET_StatusTypeDef HPS3D_SetMountingAngleEnable(HPS3D_HandleTypeDef *handle, bool en);*/ /*This interface is no longer used and is replaced by the following interface*/

/**
 * @brief	Set the installation angle transformation parameters
 * @param[in]	mounting_angle_param_conf
 * @see 		MountingAngleParamTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetMountingAngleParamConf(HPS3D_HandleTypeDef *handle, MountingAngleParamTypeDef mounting_angle_param_conf);

/**
 * @brief	Get the installation angle transformation parameters
 * @param[out]	mounting_angle_param_conf
 * @see 		MountingAngleParamTypeDef
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_GetMountingParamConf(HPS3D_HandleTypeDef *handle, MountingAngleParamTypeDef *mounting_angle_param_conf);



/*************************************2.Integrated function interface**********************************/

/**
 * @brief	Get the specified prefix file under the directory (automatically find the device))
 * @param[in]	dirPath Device file root directory
 * @param[in]   prefix  Device file name prefix
 * @param[out]  fileName To save the device found in the current current directory
 * @note		For example：n = HPS3D_GetDeviceList("/dev/","ttyACM",fileName);
 * @retval	Return the number of successful acquisitions 0 indicates failure
 */
extern uint32_t HPS3D_GetDeviceList(char *dirPath,char *prefix,char fileName[DEV_NUM][DEV_NAME_SIZE]);

/**
 * @brief	Device connect
 * @param[out]	handle->ConnectStatus
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_Connect(HPS3D_HandleTypeDef *handle);

/**
 * @brief	Disconnect
 * @param[out]	handle->ConnectStatus
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_DisConnect(HPS3D_HandleTypeDef *handle);


/**
 * @brief	Device initialization
 * @param[in]
 * @note	Create a thread and get the sensor initialization parameters to apply for memory space;
 * 			If need to use, @see HPS3D_AutoConnectAndInitConfigDevice
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_ConfigInit(HPS3D_HandleTypeDef *handle);


/**
   * @brief set serial port properties
   * @param[in] uartConfig
   * @note
   * @see UartConfTypeDef
   * @code
   * @retval successfully returns RET_OK
   */
RET_StatusTypeDef HPS3D_SetUartConfig(UartConfTypeDef uartConfig);

/**
 * @brief	Set ROI threshold
 * @param[in]	threshold_id
 * @param[in]	roi_conf
 * @see ROIConfTypeDef
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetThreshold(HPS3D_HandleTypeDef *handle, uint8_t threshold_id, ROIConfTypeDef roi_conf);


/**
 * @brief	Set ROI parameters
 * @param[in]   roi_conf
 * @see ROIConfTypeDef  GPIOOutConfTypeDef
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetSingleRoi(HPS3D_HandleTypeDef *handle, ROIConfTypeDef roi_conf, GPIOOutConfTypeDef gpio_out_conf);

/**
 * @brief	Add observer(registrate callback function notification event)
 * @param[in]	Observer_t
 * @see AsyncIObserver_t
 * @note 	Asynchronous mode (continuous measurement mode uses this mode)
 * @retval  Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_AddObserver(void * (*fun)(HPS3D_HandleTypeDef *,AsyncIObserver_t *),HPS3D_HandleTypeDef *handle,AsyncIObserver_t *Observer_t);

/**
 * @brief	 Remove observer (logout notification event)
 * @param[in]	Observer_t
 * @see		AsyncIObserver_t
 * @note	Asynchronous mode (continuous measurement mode uses this mode)
 * @retval Return OK RET_OK
 */
RET_StatusTypeDef HPS3D_RemoveObserver(AsyncIObserver_t *Observer_t);


/**
 * @brief	Device uninstallation and resource recycle
 * @param[in]
 * @note
 * @retval Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_RemoveDevice(HPS3D_HandleTypeDef *handle);

/**
 * @brief		Set debug enable
 * @param[in]	en
 * @note
 * @retval	 Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetDebugEnable(bool en);

/**
 * @brief	Get debug enable status
 * @param
 * @note
 * @retval	Return debug enable status value
 */
extern bool HPS3D_GetDebugEnable(void);

/**
 * @brief		Receive the address of the callback function
 * @param[in]	void *Call_Back     Receive callback function address   callback function is void *fun(uint8_t *str, uint16_t *str_len){...}
 * @param[out]	Return to callback function str和strlen
 * @note
 * @retval	 Return OK RET_OK
 */
RET_StatusTypeDef HPS3D_SetDebugFunc(void (*Call_Back)(char *str));

/**
 * @brief	Set point cloud data conversion enable
 * @param[in]	en
 * @note
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetPointCloudEn(bool en);

/**
 * @brief	Get point cloud data conversion enable status
 * @param
 * @note
 * @retval	true or false
 */
extern bool HPS3D_GetPointCloudEn(void);


/**
 * @brief		Single measurement
 * @param[out]	handle->RetPacketType	Return packet type
 * @param[out]	handle->MeasureData		Measure data
 * @note   This method is synchronous measurement, that is, the measured return value is obtained immediately after calling this function.
 * @see
 * @code
 * @retval	 Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SingleMeasurement(HPS3D_HandleTypeDef *handle);

/**
 * @brief		Initialize obstacle extraction parameters
 * @param[in]
 * @note
 * @see ObstacleConfigTypedef
 * @code
 *
 * @retval	 Return OK RET_OK
 */
extern ObstacleConfigTypedef HPS3D_GetObstacleConfigInit(void);

/**
 * @brief		Get obstacle extraction configuration parameters
 * @param
 * @note
 * @see ObstacleConfigTypedef
 * @code
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_ObstacleConfigInit(ObstacleConfigTypedef Conf);

/**
 * @brief		Set the threshold number of obstacle pixels
 * @param
 * @note  Set this threshold to filter small obstacles.
 * @see
 * @code
 * @retval none
 */
extern void HPS3D_SetObstaclePixelNumberThreshold(uint32_t pixel_num_thr);

/**
 * @brief		Get the threshold number of obstacle pixels
 * @param
 * @note
 * @see
 * @code
 * @retval  Return pixel threshold
 */
extern uint32_t HPS3D_GetObstaclePixelNumberThreshold(void);


/**
 * @brief	Set obstacle extraction threshold offset
 * @param
 * @note
 * @see
 * @code
 * @retval  none
 */
extern void HPS3D_SetThresholdOffset(int32_t thr_offset);

/**
 * @brief	Get obstacle extraction threshold offset
 * @param
 * @note
 * @see
 * @code
 * @retval  Return current threshold
 */
extern int32_t HPS3D_GetThresholdOffset(void);


/**
 * @brief	Get SDK version
 * @param
 * @note
 * @see
 * @code
 * @retval	 Version information
 */
extern Version_t HPS3D_GetSDKVersion(void);


/**
 * @brief		Parameters configuration on how to convert special measured output values to specified special value
 * param[in]	enable
 * param[in]    value
 * @note
 * @see
 * @code
 * @retval	 Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_ConfigSpecialMeasurementValue(bool enable,uint16_t value);

/**
 * @brief		Set edge noise filter enable
 * param[in]
 * @note
 * @see
 * @code
 * @retval	Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetEdgeDetectionEnable(bool en);

/**
 * @brief		Get edge noise filter enable
 * param[in]
 * @note
 * @see
 * @code
 *
 * @retval	 Return status value
 */
extern bool HPS3D_GetEdgeDetectionEnable(void);

/**
 * @brief		Set edge noise threshold
 * param[in]
 * @note
 * @see
 * @code
 *
 * @retval	 Return OK RET_OK
 */
extern RET_StatusTypeDef HPS3D_SetEdgeDetectionValue(int32_t threshold_value);

/**
 * @brief	Get edge noise threshold
 * param[in]
 * @note
 * @see
 * @code
 *
 * @retval	 Return current threshold value
 */
extern int32_t HPS3D_GetEdgeDetectionValue(void);

/**
  * @brief	save point cloud data to ply format file
  * @param[in]
  * @param[in] point_cloud_data
  * @note
  * @see PointCloudDataTypeDef
  * @code
  * @retval Return OK RET_OK
  */
extern RET_StatusTypeDef HPS3D_SavePlyFile(char *filename,PointCloudDataTypeDef point_cloud_data);

/**
  * @brief	Extract obstacle avoidance scheme (3IO version output alarm value by IO)
  * @param[in] MeasureData
  * @param[in] AvoidConf
  * @note
  * @see AvoidanceTypeDef
  * @code
  * @retval	Return OK RET_OK
  */
extern RET_StatusTypeDef HPS3D_ObstacleAvoidance(HPS3D_HandleTypeDef *handle,MeasureDataTypeDef *MeasureData,AvoidanceTypeDef *AvoidConf);

/**
  * @brief	Set point cloud mirror
  * @param[in] type Mirror direction (horizontal or vertical)
  * @note
  * @see PointCloudImageMirrorTypeDef
  * @code
  * @retval	Return OK RET_OK
  */
extern RET_StatusTypeDef HPS3D_SetPointCloudMirrorType(PointCloudImageMirrorTypeDef type);
/**
  * @brief	Get point cloud mirror
  * @param[in]
  * @note
  * @see PointCloudImageMirrorTypeDef
  * @code
  * @retval PointCloudImageMirrorTypeDef
  */
extern PointCloudImageMirrorTypeDef HPS3D_GetPointCloudMirrorType(void);

/**
  * @brief	Set server information
  * @param[in]
  * @note eg: char *serverIP = "192.168.0.10";
  *         serverPort = 12345;  This function needs to be called before connect
  * @see
  * @code
  * @retval	Return OK RET_OK
  */
extern RET_StatusTypeDef HPS3D_SetEthernetServerInfo(HPS3D_HandleTypeDef *handle,char *serverIP,uint16_t serverPort);

/**
  * @brief	Reset server IP（To edit device default IP. Edit here is not recommended
  * @param[in]  serverIP
  * @param[in]  serverPort
  * @param[in]  netmask
  * @param[in]  gateway
  * @note eg:serverIP[4] = {192,168,0,10};
  *         netMask[4] = {255,255,255,0};
  *         geteway[4] = {192,168,0,1};
  *         serverPort = 12345;
  * @see
  * @code
  * @retval	Return OK RET_OK
  */
RET_StatusTypeDef HPS3D_ConfigEthernet(HPS3D_HandleTypeDef *handle,uint8_t *serverIP,uint16_t serverPort,uint8_t *netMask,uint8_t *geteway);

/**
  * @brief	Multiple devices automatically connect and initialize (the number of connections is limited by DEV_NUM and can be modified)
  * @param[in]
  * @note
  * @see
  * @code
  * @retval	Return the number of successful connections
  */
extern uint8_t HPS3D_AutoConnectAndInitConfigDevice(HPS3D_HandleTypeDef *handle);

/**
  * @brief set multi-machine encoding
  * @param[in] CameraCode Multi-machine code
  * @note Multi-machine encoding only supports firmware version 1.7.62 and above, and the code serial number is 0-15.
  * @see
  * @code
  *
  * @retval Return OK RET_OK
  */
extern RET_StatusTypeDef HPS3D_SetMultiCameraCode(HPS3D_HandleTypeDef *handle, uint8_t CameraCode);

/**
  * @brief Get multi-machine code
  * @param[in] handle
  * @note
  * @see
  * @code
  *
  * @retval returns the current multi-machine code value
  */
extern uint8_t HPS3D_GetMultiCameraCode(HPS3D_HandleTypeDef *handle);


/**
  * @brief set heartbeat detection
  * @param[in] handle
  * @param[in] enable Heartbeat Detection Enable
  * @param[in] time_ms heartbeat detection time ms
  * @note After calling this interface, send a heartbeat packet HPS3D_SendKeepAlive before HPS3D_SetRunMode
  * @see
  * @code
  * @retval	Return OK RET_OK
  */
extern RET_StatusTypeDef HPS3D_SetKeepAliveConfig(HPS3D_HandleTypeDef *handle, bool enable, uint32_t time_ms);

/**
  * @brief sends a keep-alive command
  * @param[in] handle
  * @note This interface needs to be sent periodically within the set heartbeat detection time. After sending this command, it is only necessary to detect whether there is a heartbeat return packet within the set time.
  * KEEP_ALIVE_PACKET in @see RetPacketTypedef
  * @code
  * @retval	Return OK RET_OK
  */
extern RET_StatusTypeDef HPS3D_SendKeepAlive(HPS3D_HandleTypeDef *handle);

/**
  * @brief sets the number of splicing devices
  * @param[in] number of stitches
  * @note This interface is only used when multiple devices are spliced. For details, please contact the relevant technical staff.
  * @see
  * @code
  * @retval returns RET_OK successfully
  */
extern RET_StatusTypeDef HPS3D_SpliceSetDeviceNumber(uint8_t number);

/**
  * @brief Get the number of current splicing devices
  * @param[in]
  * @note This interface is only used when multiple devices are spliced. For details, please contact the relevant technical staff.
  * @see
  * @code
  * @retval number of stitches
  */
extern uint8_t HPS3D_SpliceGetDeviceNumber(void);

/**
  * @brief Loads the current device installation angle and other parameters (used for multiple devices in a unified coordinate system)
  * @param[in]
  * @note This interface is only used when multiple devices are spliced. For details, please contact the relevant technical staff.
  * @see
  * @code
  * @retval returns RET_OK successfully
  */
extern RET_StatusTypeDef HPS3D_SpliceLoadRotateConfig(const char* fileName);

/**
  * @brief Load parameters such as splicing device sensitive area
  * @param[in]
  * @note This interface is only used when multiple devices are spliced. For details, please contact the relevant technical staff.
  * @see
  * @code
  * @retval returns RET_OK successfully
  */
extern RET_StatusTypeDef HPS3D_SpliceLoadROIConfig(const char* fileName);

/**
  * @brief Point cloud data and sensitive area data processing functions when multiple devices are spliced
  * @param[in] SplicePointCloudBuffer: Complete point cloud data after unifying the device coordinate system
  * @param[in] point_cloud_data: point cloud data buffer
  * @param[in] FullRoiData: used to save the ROI information under the current group
  * @note This interface is only used when splicing multiple devices. For details, please contact the relevant technical personnel. This interface must judge the return value.
  * @see
  * @code
  * @retval returns RET_OK successfully
  */
extern RET_StatusTypeDef HPS3D_SpliceProcessFunc(PerPointCloudDataTypeDef *SplicePointCloudBuffer, uint8_t id, PerPointCloudDataTypeDef point_cloud_data[MAX_PIX_NUM], FullRoiDataTypeDef FullRoiData[ROI_NUM]);

/**
 * @brief sets the number of splicing devices
 * @param[in] group ID
 * @note Please refer to ROI_GROUP_NUMBER for the range of ID values.
 * @see ROI_GROUP_NUMBER
 * @code
 * @retval returns none
 */
extern RET_StatusTypeDef HPS3D_SpliceSetCurrentGroupID(uint8_t groupID);

/**
  * @brief Get the current number of splicing devices
  * @param
  * @note
  * @see
  * @code
  * @retval returns none
  */
extern uint8_t HPS3D_SpliceGetCurrentGroupID(void);


/**
  * @brief sets the maximum integration time
  * @param[in] max_integration_time
  * @note
  * @see
  * @code
  * @retval returns RET_OK successfully
  */
extern RET_StatusTypeDef HPS3D_SetMaxIntegTime(HPS3D_HandleTypeDef *handle, uint32_t max_integration_time);

/**
  * @brief Get the maximum integration time
  * @param[out] max_integration_time
  * @note
  * @see
  * @code
  * @retval returns RET_OK successfully
  */
extern RET_StatusTypeDef HPS3D_GetMaxIntegTime(HPS3D_HandleTypeDef *handle, uint32_t *max_integration_time);

/**
  * @brief sets the minimum signal amplitude
  * @param[in] minAmplitude
  * @note
  * @see
  * @code
  * @retval returns RET_OK successfully
  */
extern RET_StatusTypeDef HPS3D_SetMinAmpliude(HPS3D_HandleTypeDef *handle, uint32_t minAmplitude);

/**
  * @brief Get the minimum signal amplitude
  * @param[out] minAmplitude
  * @note
  * @see
  * @code
  * @retval returns RET_OK successfully
  */
extern RET_StatusTypeDef HPS3D_GetMinAmplitude(HPS3D_HandleTypeDef *handle, uint32_t *minAmplitude);

/**
 * @brief	Get Serial Number
 * @param[in]	handle   
 * @param[out]	info[64] 
 * @note
 * @see
 * @code
 *
 * @retval 成功返回 RET_OK
 */
 extern RET_StatusTypeDef HPS3D_GetSerialNumber(HPS3D_HandleTypeDef *handle, uint8_t *info);
 
#ifdef __cplusplus
}
#endif

#endif /* API_H_ */
