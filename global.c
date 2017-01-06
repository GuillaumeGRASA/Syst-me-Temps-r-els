/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tServeur;
RT_TASK tconnect;
RT_TASK tmove;
RT_TASK tenvoyer;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexCompteurRobot; // Mutex pour consulter la variable partagée compteurRobot
RT_MUTEX mutexTypeCalibration;
RT_MUTEX mutexAreneSauvegarde;
RT_MUTEX mutexCalibration;
RT_MUTEX mutexCalculPosition;
RT_MUTEX mutexCommCamera;


RT_SEM semConnecterRobot;
RT_SEM semCalibrationArene;
RT_SEM semWatchdog;
RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommCamera = 1;
int etatCommRobot = 1;
int compteurRobot = 0;
int typeCalibration=1;
DArena areneSauvegarde=NULL;
int calibration=0;
int calculPosition = 0;

DRobot *robot;
DMovement *move;
DServer *serveur;
Dcamera *camera;


int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TETATBATTERIE = 15;
int PRIORITY_TRELOADWATCHDOG = 13;
int PRIORITY_TTRAITEMENTIMAGE = 40;
int PRIORITY_TCALIBRATION = 30 ;
