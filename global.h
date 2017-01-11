/* 
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tServeur;
extern RT_TASK tconnect;
extern RT_TASK tmove;
extern RT_TASK tenvoyer;
extern RT_TASK tcalibrationArene;
extern RT_TASK ttraitementImage;
extern RT_TASK treloadWatchdog;
extern RT_TASK tetatBatterie;


/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexCompteurRobot; // Mutex pour consulter la variable partagée compteurRobot
extern RT_MUTEX mutexTypeCalibration;
extern RT_MUTEX mutexAreneSauvegarde;
extern RT_MUTEX mutexCalibration;
extern RT_MUTEX mutexCalculPosition;
extern RT_MUTEX mutexCommCamera;

/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semCalibrationArene;
extern RT_SEM semWatchdog;
extern RT_SEM semConnecterCamera;

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int etatCommCamera;
extern int compteurRobot; // Variable pour gérer le compteur de déconnexion du robot
extern int typeCalibration; // Variable pour stocker la demande en relation avec l'arène
extern DArena* areneSauvegarde; //Variable pour stocker l'arene calibrée
extern int calibration; //Variable pour gerer la calibration
extern int calculPosition;

extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;
extern DCamera *camera;
extern DBattery *batterie;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TCALIBRATION;
extern int PRIORITY_TTRAITEMENTIMAGE;
extern int PRIORITY_TRELOADWATCHDOG;
extern int PRIORITY_TETATBATTERIE;

#endif	/* GLOBAL_H */

