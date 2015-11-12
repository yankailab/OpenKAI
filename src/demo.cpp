#include "demo.h"


int main(int argc, char* argv[])
{
	// Initialize Google's logging library.
	google::InitGoogleLogging(argv[0]);

	printEnvironment();



	//Testing mavlink control
	return main_2(argc, argv);












	LOG(INFO)<<"Using config file: "<<argv[1];
	CHECK_FATAL(g_file.open(argv[1]));

	string config = g_file.getContent();
	CHECK_FATAL(g_Json.parse(config.c_str()));

	g_config.setJSON(&g_Json);

	//Connect to Mavlink
	g_pVehicle = new VehicleInterface();
	CHECK_ERROR(g_Json.getVal("serialPort", &g_serialPort));
	g_pVehicle->setSerialName(g_serialPort);
	if (g_pVehicle->open())
	{
		printf("Serial port opened¥n");
	}

	//Init CamStream
	g_pCamFront = new CamStream();
	CHECK_FATAL(g_config.setCamStream(g_pCamFront));
	g_pCamFront->init();
	g_pCamFront->openWindow();

#ifdef OBJECT_DETECT
	g_pCamFront->m_bGray = true;
#endif
#ifdef MARKER_COPTER
	g_pCamFront->m_bHSV = true;
	g_pCamFront->m_bMarkerDetect = true;
//	g_pCamFront->m_bDenseFlow = true;
#endif

#ifdef OBJECT_DETECT
	//Init Object Detector
	g_pOD = new ObjectDetector();
	g_pOD->init(&g_Json);
	g_pOD->m_pCamStream = g_pCamFront;

	//Init Fast Detector
	g_pFD = new FastDetector();
	g_pFD->init(&g_Json);
	g_pFD->m_pCamStream = g_pCamFront;

#endif

	//Init Autopilot
	g_pAP = new AutoPilot();
	CHECK_FATAL(g_config.setAutoPilot(g_pAP));
	g_pAP->init();
	g_pAP->setCamStream(g_pCamFront, CAM_FRONT);
	g_pAP->setVehicleInterface(g_pVehicle);
#ifdef OBJECT_DETECT
	g_pAP->m_pOD = g_pOD;
	g_pAP->m_pFD = g_pFD;
#endif
#ifdef MARKER_COPTER
	g_pMD = g_pAP->m_pCamStream[CAM_FRONT].m_pCam->m_pMarkerDetect;
#endif

	//Start threads
//	g_pVehicle->start();
	g_pCamFront->start();
#ifdef MARKER_COPTER
	g_pAP->start();
#endif
#ifdef OBJECT_DETECT
	g_pOD->start();
	g_pFD->start();
#endif

	//UI thread
	int i;
	g_bRun = true;
	g_bTracking = false;
	setMouseCallback(g_pCamFront->m_camName, onMouse, NULL);

	while (g_bRun)
	{

		if (!g_pCamFront->m_pMonitor->m_mat.empty())
		{
#ifdef OBJECT_DETECT
			Mat imL, imR, imD;

			g_pCamFront->m_pFrameL->m_pNext->download(imL);
			if (!imL.empty())
			{
				OBJECT* pObj;
				for (i = 0; i < g_pOD->m_numObj; i++)
				{
					pObj = &g_pOD->m_pObjects[i];
					if (pObj->m_name[0].empty())
						continue;

					rectangle(imL, pObj->m_boundBox.tl(),
							pObj->m_boundBox.br(), Scalar(0, 255, 0), 2, 5, 0);

					putText(imL, pObj->m_name[0],
							pObj->m_boundBox.tl(), FONT_HERSHEY_SIMPLEX, 0.6,
							Scalar(255, 0, 0), 2);
				}

				FAST_OBJECT* pFastObj;
				for (i = 0; i < g_pFD->m_numHuman; i++)
				{
					pFastObj = &g_pFD->m_pHuman[i];

					rectangle(imL, pFastObj->m_boundBox.tl(),
							pFastObj->m_boundBox.br(), Scalar(0, 0, 255), 2, 5, 0);
				}


				//			g_pCamFront->m_pMonitor->show();

				imshow("Left", imL);
				if(g_pOD->m_saliencyMap.rows != 0)
				{
					imshow( "Saliency Map", g_pOD->m_saliencyMap );
				}

				if(g_pOD->m_binMap.rows != 0)
				{
					imshow( "Binary Map", g_pOD->m_binMap );
				}
			}



			if (g_pCamFront->m_bStereoCam)
			{
				g_pCamFront->m_pFrameR->m_pNext->download(imR);
				if (!imR.empty())
				{
					imshow("Right", imR);
				}
				imD = Mat(imD.size(), CV_8U);
				g_pCamFront->m_pDepth->m_pNext->download(imD);
				if (!imD.empty())
				{
					imshow("Stereo", imD);
				}
			}



#endif

#ifdef MARKER_COPTER
			displayInfo();
			imshow(g_pCamFront->m_camName, g_displayMat);
#endif
		}

		//Handle key input
		g_key = waitKey(30);
		handleKey(g_key);
	}

#ifdef OBJECT_DETECT
	g_pOD->stop();
#endif
	g_pAP->stop();
	g_pCamFront->stop();
	g_pVehicle->stop();

#ifdef OBJECT_DETECT
	g_pOD->complete();
#endif
	g_pAP->complete();
	g_pCamFront->complete();
	g_pVehicle->complete();

	g_pVehicle->close();

	delete g_pVehicle;
	delete g_pAP;
	delete g_pCamFront;

#ifdef OBJECT_DETECT
	delete g_pOD;
#endif

	return 0;

}





























/****************************************************************************
 *
 *   Copyright (c) 2014 MAVlink Development Team. All rights reserved.
 *   Author: Trent Lukaczyk, <aerialhedgehog@gmail.com>
 *           Jaycee Lock,    <jaycee.lock@gmail.com>
 *           Lorenz Meier,   <lm@inf.ethz.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file mavlink_control.cpp
 *
 * @brief An example offboard control process via mavlink
 *
 * This process connects an external MAVLink UART device to send an receive data
 *
 * @author Trent Lukaczyk, <aerialhedgehog@gmail.com>
 * @author Jaycee Lock,    <jaycee.lock@gmail.com>
 * @author Lorenz Meier,   <lm@inf.ethz.ch>
 *
 */


// ------------------------------------------------------------------------------
//   TOP
// ------------------------------------------------------------------------------

int top(int argc, char **argv)
{

	// --------------------------------------------------------------------------
	//   PARSE THE COMMANDS
	// --------------------------------------------------------------------------

	// Default input arguments
//	char *uart_name = (char*) "/dev/ttyUSB0";
	char *uart_name = (char*) "/dev/cu.usbmodem1";
	int baudrate = 115200;//57600;

	// do the parse, will throw an int if it fails
	parse_commandline(argc, argv, uart_name, baudrate);

	// --------------------------------------------------------------------------
	//   PORT and THREAD STARTUP
	// --------------------------------------------------------------------------

	/*
	 * Instantiate a serial port object
	 *
	 * This object handles the opening and closing of the offboard computer's
	 * serial port over which it will communicate to an autopilot.  It has
	 * methods to read and write a mavlink_message_t object.  To help with read
	 * and write in the context of pthreading, it gaurds port operations with a
	 * pthread mutex lock.
	 *
	 */
	Serial_Port serial_port(uart_name, baudrate);

	/*
	 * Instantiate an autopilot interface object
	 *
	 * This starts two threads for read and write over MAVlink. The read thread
	 * listens for any MAVlink message and pushes it to the current_messages
	 * attribute.  The write thread at the moment only streams a position target
	 * in the local NED frame (mavlink_set_position_target_local_ned_t), which
	 * is changed by using the method update_setpoint().  Sending these messages
	 * are only half the requirement to get response from the autopilot, a signal
	 * to enter "offboard_control" mode is sent by using the enable_offboard_control()
	 * method.  Signal the exit of this mode with disable_offboard_control().  It's
	 * important that one way or another this program signals offboard mode exit,
	 * otherwise the vehicle will go into failsafe.
	 *
	 */
	Autopilot_Interface autopilot_interface(&serial_port);

	/*
	 * Setup interrupt signal handler
	 *
	 * Responds to early exits signaled with Ctrl-C.  The handler will command
	 * to exit offboard mode if required, and close threads and the port.
	 * The handler in this example needs references to the above objects.
	 *
	 */
	serial_port_quit = &serial_port;
	autopilot_interface_quit = &autopilot_interface;
	signal(SIGINT, quit_handler);

	/*
	 * Start the port and autopilot_interface
	 * This is where the port is opened, and read and write threads are started.
	 */
	serial_port.start();
	autopilot_interface.start();

	// --------------------------------------------------------------------------
	//   RUN COMMANDS
	// --------------------------------------------------------------------------

	/*
	 * Now we can implement the algorithm we want on top of the autopilot interface
	 */
	commands(autopilot_interface);

	// --------------------------------------------------------------------------
	//   THREAD and PORT SHUTDOWN
	// --------------------------------------------------------------------------

	/*
	 * Now that we are done we can stop the threads and close the port
	 */
	autopilot_interface.stop();
	serial_port.stop();

	// --------------------------------------------------------------------------
	//   DONE
	// --------------------------------------------------------------------------

	// woot!
	return 0;

}

// ------------------------------------------------------------------------------
//   COMMANDS
// ------------------------------------------------------------------------------

void commands(Autopilot_Interface &api)
{

	// --------------------------------------------------------------------------
	//   START OFFBOARD MODE
	// --------------------------------------------------------------------------

	api.enable_offboard_control();
	usleep(100); // give some time to let it sink in

	// now the autopilot is accepting setpoint commands

	// --------------------------------------------------------------------------
	//   SEND OFFBOARD COMMANDS
	// --------------------------------------------------------------------------
	printf("SEND OFFBOARD COMMANDS\n");

	// initialize command data strtuctures
	mavlink_set_position_target_local_ned_t sp;
	mavlink_set_position_target_local_ned_t ip = api.initial_position;

	// autopilot_interface.h provides some helper functions to build the command

	// Example 1 - Set Velocity
//	set_velocity( -1.0       , // [m/s]
//				  -1.0       , // [m/s]
//				   0.0       , // [m/s]
//				   sp        );

	// Example 2 - Set Position
	set_position(ip.x - 5.0, // [m]
	ip.y - 5.0, // [m]
	ip.z, // [m]
			sp);

	// Example 1.2 - Append Yaw Command
	set_yaw(ip.yaw, // [rad]
			sp);

	// SEND THE COMMAND
	api.update_setpoint(sp);
	// NOW pixhawk will try to move

	// Wait for 8 seconds, check position
	for (int i = 0; i < 8; i++)
	{
		mavlink_local_position_ned_t pos =
				api.current_messages.local_position_ned;
		printf("%i CURRENT POSITION XYZ = [ % .4f , % .4f , % .4f ] \n", i,
				pos.x, pos.y, pos.z);
		sleep(1);
	}

	printf("\n");

	// --------------------------------------------------------------------------
	//   STOP OFFBOARD MODE
	// --------------------------------------------------------------------------

	api.disable_offboard_control();

	// now pixhawk isn't listening to setpoint commands

	// --------------------------------------------------------------------------
	//   GET A MESSAGE
	// --------------------------------------------------------------------------
	printf("READ SOME MESSAGES \n");

	// copy current messages
	Mavlink_Messages messages = api.current_messages;

	// local position in ned frame
	mavlink_local_position_ned_t pos = messages.local_position_ned;
	printf(
			"Got message LOCAL_POSITION_NED (spec: https://pixhawk.ethz.ch/mavlink/#LOCAL_POSITION_NED)\n");
	printf("    pos  (NED):  %f %f %f (m)\n", pos.x, pos.y, pos.z);

	// hires imu
	mavlink_highres_imu_t imu = messages.highres_imu;
	printf(
			"Got message HIGHRES_IMU (spec: https://pixhawk.ethz.ch/mavlink/#HIGHRES_IMU)\n");
	printf("    ap time:     %lu \n", imu.time_usec);
	printf("    acc  (NED):  % f % f % f (m/s^2)\n", imu.xacc, imu.yacc,
			imu.zacc);
	printf("    gyro (NED):  % f % f % f (rad/s)\n", imu.xgyro, imu.ygyro,
			imu.zgyro);
	printf("    mag  (NED):  % f % f % f (Ga)\n", imu.xmag, imu.ymag, imu.zmag);
	printf("    baro:        %f (mBar) \n", imu.abs_pressure);
	printf("    altitude:    %f (m) \n", imu.pressure_alt);
	printf("    temperature: %f C \n", imu.temperature);

	printf("\n");

	// --------------------------------------------------------------------------
	//   END OF COMMANDS
	// --------------------------------------------------------------------------

	return;

}

// ------------------------------------------------------------------------------
//   Parse Command Line
// ------------------------------------------------------------------------------
// throws EXIT_FAILURE if could not open the port
void parse_commandline(int argc, char **argv, char *&uart_name, int &baudrate)
{

	// string for command line usage
	const char *commandline_usage =
			"usage: mavlink_serial -d <devicename> -b <baudrate>";

	// Read input arguments
	for (int i = 1; i < argc; i++)
	{ // argv[0] is "mavlink"

		// Help
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printf("%s\n", commandline_usage);
			throw EXIT_FAILURE;
		}

		// UART device ID
		if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--device") == 0)
		{
			if (argc > i + 1)
			{
				uart_name = argv[i + 1];

			}
			else
			{
				printf("%s\n", commandline_usage);
				throw EXIT_FAILURE;
			}
		}

		// Baud rate
		if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--baud") == 0)
		{
			if (argc > i + 1)
			{
				baudrate = atoi(argv[i + 1]);

			}
			else
			{
				printf("%s\n", commandline_usage);
				throw EXIT_FAILURE;
			}
		}

	}
	// end: for each input argument

	// Done!
	return;
}

// ------------------------------------------------------------------------------
//   Quit Signal Handler
// ------------------------------------------------------------------------------
// this function is called when you press Ctrl-C
void quit_handler(int sig)
{
	printf("\n");
	printf("TERMINATING AT USER REQUEST\n");
	printf("\n");

	// autopilot interface
	try
	{
		autopilot_interface_quit->handle_quit(sig);
	} catch (int error)
	{
	}

	// serial port
	try
	{
		serial_port_quit->handle_quit(sig);
	} catch (int error)
	{
	}

	// end program here
	exit(0);

}

// ------------------------------------------------------------------------------
//   Main
// ------------------------------------------------------------------------------
int main_2(int argc, char **argv)
{
	// This program uses throw, wrap one big try/catch here
	try
	{
		int result = top(argc, argv);
		return result;
	}

	catch (int error)
	{
		fprintf(stderr, "mavlink_control threw exception %i \n", error);
		return error;
	}

}



