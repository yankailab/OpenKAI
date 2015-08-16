#define APP_NAME "CVDrone"
#define CONFIG_WINDOW "Configuration"

#define SHOW_MARKER_FLOW 0
#define MF_WINDOW "Marker_Flow"
#define SHOW_FEATURE_FLOW 0
#define CF_WINDOW "Feature_Flow"
#define SHOW_OPTICAL_FLOW 0
#define OPF_WINDOW "Optical_Flow"

#define USE_MARKER_FLOW
//#define USE_OPTICAL_FLOW
//#define USE_FEATURE_FLOW

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
#define CAM_UPWARD_ID 1
#endif

#define Z_FAR_LIM 50
#define Z_NEAR_LIM 500

#define PID_UI_LIM 1000.0
#define PID_UI_LIM_HALF 500.0

#define PID_P_LIM 10.0
#define PID_I_LIM 1.0
#define PID_D_LIM 50.0
#define PID_Z_LIM 500.0
#define PID_YAW_P_LIM 200.0
#define DT_LIM 10.0

