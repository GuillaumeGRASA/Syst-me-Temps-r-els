/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tcontroler;
RT_TASK tcapturer;
RT_TASK tdisplay;

RT_MUTEX mutexOSD;

/*
RT_SEM semConnecterRobot;
RT_SEM semConnecterCamera;
RT_SEM semConnecterCamera;*/

//Variable globale à déclarer : OSD

RT_QUEUE queueOSDBuffer;

int OSD_QUEUE_SIZE = 10;

int PRIORITY_TCONTROLER = 30;
int PRIORITY_TCAPTURE = 20;
int PRIORITY_TDISPLAY = 10;

