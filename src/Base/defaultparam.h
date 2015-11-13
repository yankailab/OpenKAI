

//Optical Flow Default PID
#ifdef USE_OPTICAL_FLOW
#define ROLL_P -0.3
#define ROLL_I 0.0
#define ROLL_IMAX 0.0
#define ROLL_D 0.20

#define PITCH_P 0.1
#define PITCH_I 0.0
#define PITCH_IMAX 0.0
#define PITCH_D 0.15

#define ALT_P 2.0
#define ALT_I 0.0
#define ALT_IMAX 0.0
#define ALT_D 0.25

#define YAW_P 200.0
#define YAW_I 0.0
#define YAW_IMAX 0.0
#define YAW_D 0.0

#define DELAY_TIME 6.8

#define CAM_WIDTH 300
#define CAM_HEIGHT 300

#define CAM_EXTERNAL_ID 1
#define CAM_UPWARD_ID 2
#endif

//Marker Locking PID
/*
#ifdef USE_MARKER_FLOW
#define ROLL_P 1.0
#define ROLL_I 0.25
#define ROLL_IMAX 30.0
#define ROLL_D 8.0

#define PITCH_P 1.5
#define PITCH_I 0.1
#define PITCH_IMAX 15.0
#define PITCH_D 6.0

#define ALT_P 2.0
#define ALT_I 0.0
#define ALT_IMAX 0.0
#define ALT_D 0.25

#define YAW_P 200.0
#define YAW_I 0.0
#define YAW_IMAX 0.0
#define YAW_D 0.0

#define DELAY_TIME 5.0

#define CAM_WIDTH 1280
#define CAM_HEIGHT 800

#define CAM_EXTERNAL_ID 0
#define CAM_UPWARD_ID 2

#endif
*/
