#include "fonctions.h"

int write_in_queue (RT_QUEUE * msgQueue, void *data, int size);

void envoyer (void *arg)
{
  DMessage *msg;
  int err;

  while (1)
    {
      rt_printf ("tenvoyer : Attente d'un message\n");
      if ((err = rt_queue_read (&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
	rt_printf ("tenvoyer : envoi d'un message au moniteur\n");
	serveur->send (serveur, msg);
	msg->free (msg);
      }
      else  {
	rt_printf ("Error msg queue write: %s\n", strerror (-err));
      }
    }
}


void connecter (void *arg)
{
  int status;
  DMessage *message;

  rt_printf ("tconnect : Debut de l'exécution de tconnect\n");

  while (1)
    {
      rt_printf ("tconnect : Attente du sémarphore semConnecterRobot\n");
      rt_sem_p (&semConnecterRobot, TM_INFINITE);
      rt_printf ("tconnect : Ouverture de la communication avec le robot\n");
      status = robot->open_device (robot);
      rt_mutex_acquire (&mutexEtat, TM_INFINITE);
      etatCommRobot = status;
      rt_mutex_release (&mutexEtat);

      if (status == STATUS_OK)
	{
	  status = robot->start_insecurely(robot);
	  if (status == STATUS_OK){
	    rt_printf("tconnect : Robot démarrer, On lance le Watchdog\n");
	    rt_sem_v(&semWatchdog);    
	  }
	}

      message = d_new_message ();
      message->put_state (message, status);

      rt_printf ("tconnecter : Envoi message\n");
      message->print (message, 100);

      if (write_in_queue (&queueMsgGUI, message, sizeof (DMessage)) < 0)
	{
	  message->free (message);
	}
    }
		      
}

void communiquer (void *arg)
{
  DMessage *msg;
  int var1 = 1;
  int num_msg = 0;

  rt_printf ("tserver : Début de l'exécution de serveur\n");
  serveur->open (serveur, "8000");
  rt_printf ("tserver : Connexion\n");

  rt_mutex_acquire (&mutexEtat, TM_INFINITE);
  etatCommMoniteur = 0;
  rt_mutex_release (&mutexEtat);

  while (var1 > 0)
    {
      rt_printf ("tserver : Attente d'un message\n");
      var1 = serveur->receive (serveur, msg);
      num_msg++;
      if (var1 > 0) {
	switch (msg->get_type (msg))
	  {
	  case MESSAGE_TYPE_ACTION:
	    rt_printf ("tserver : Le message %d reçu est une action\n",num_msg);
	    DAction *action = d_new_action ();
	    action->from_message (action, msg);
	    switch (action->get_order (action)) {
	    case ACTION_CONNECT_ROBOT:
	      rt_printf("tserver : Action connecter robot\n");
	      rt_sem_v(&semConnecterRobot);
	      rt_printf("tserver : Action connecter camera \n");
	      rt_sem_v(&semConnecterCamera);
	      break;
	    case ACTION_FIND_ARENA:
	    case ACTION_ARENA_FAILED:
	    case ACTION_ARENA_IS_FOUND:
	      rt_printf("tserver : Action detecter arene \n");
	      rt_mutex_acquire(&mutexTypeCalibration, TM_INFINITE);
	      typeCalibration = action->get_order (action);
	      rt_mutex_release (&mutexTypeCalibration);
	      rt_sem_v(&semCalibrationArene);
	      break;
	    case ACTION_COMPUTE_CONTINUOUSLY_POSITION:
	      rt_printf ("tsever : Action calcul position \n");
	      rt_mutex_acquire (&mutexCalculPosition, TM_INFINITE);
	      calculPosition = 1;
	      rt_mutex_release (&mutexCalculPosition);
	      break;
	    }
	    break;
	  case MESSAGE_TYPE_MOVEMENT:
	    rt_printf ("tserver : Le message reçu %d est un mouvement\n",num_msg);
	    rt_mutex_acquire (&mutexMove, TM_INFINITE);
	    move->from_message (move, msg);
	    move->print (move);
	    rt_mutex_release (&mutexMove);
	    break;
	  }
      }
    }
}

void
deplacer (void *arg)
{
  int status = 1;
  int gauche;
  int droite;
  int compteur;
  DMessage *message;
  
  rt_printf ("tmove : Debut de l'éxecution de periodique à 1s\n");
  rt_task_set_periodic (NULL, TM_NOW, 1000000000);

  while (1)
    {
      /* Attente de l'activation périodique */
      rt_task_wait_period (NULL);
      rt_printf ("tmove : Activation périodique\n");

      rt_mutex_acquire (&mutexEtat, TM_INFINITE);
      status = etatCommRobot;
      rt_mutex_release (&mutexEtat);

      rt_mutex_acquire (&mutexCompteurRobot, TM_INFINITE);
      compteur = compteurRobot;
      rt_mutex_release (&mutexCompteurRobot);

      if (status == STATUS_OK)
	{
	  rt_mutex_acquire (&mutexMove, TM_INFINITE);
	  switch (move->get_direction (move))
	    {
	    case DIRECTION_FORWARD:
	      gauche = MOTEUR_ARRIERE_LENT;
	      droite = MOTEUR_ARRIERE_LENT;
	      break;
	    case DIRECTION_LEFT:
	      gauche = MOTEUR_ARRIERE_LENT;
	      droite = MOTEUR_AVANT_LENT;
	      break;
	    case DIRECTION_RIGHT:
	      gauche = MOTEUR_AVANT_LENT;
	      droite = MOTEUR_ARRIERE_LENT;
	      break;
	    case DIRECTION_STOP:
	      gauche = MOTEUR_STOP;
	      droite = MOTEUR_STOP;
	      break;
	    case DIRECTION_STRAIGHT:
	      gauche = MOTEUR_AVANT_LENT;
	      droite = MOTEUR_AVANT_LENT;
	      break;
	    }
	  rt_mutex_release (&mutexMove);

	  status = robot->set_motors (robot, gauche, droite);

	  if (status != STATUS_OK)
	    {
	      if (compteur < 3)
		{
		  rt_mutex_acquire (&mutexCompteurRobot, TM_INFINITE);
		  compteurRobot = compteur + 1;
		  rt_mutex_release (&mutexCompteurRobot);
		}
	      else
		{
		  rt_mutex_acquire (&mutexEtat, TM_INFINITE);
		  etatCommRobot = status;
		  rt_mutex_release (&mutexEtat);

		  message = d_new_message ();
		  message->put_state (message, status);

		  rt_printf ("tperteConnexion : Envoi message\n"); 
		  if (write_in_queue
		      (&queueMsgGUI, message, sizeof (DMessage)) < 0)
		    {
		      message->free (message);
		    }
		  rt_mutex_acquire (&mutexCompteurRobot, TM_INFINITE);
		  compteurRobot = 0;
		  rt_mutex_release (&mutexCompteurRobot);
		}
	    }
	  else
	    {
	      rt_mutex_acquire (&mutexCompteurRobot, TM_INFINITE);
	      compteurRobot = 0;
	      rt_mutex_release (&mutexCompteurRobot);
	    }
	}
    }
}

void etatBatterie(void *arg){
  
  int status = 1; 
  DMessage *message;
  int *etatBat = malloc(sizeof(int));
    
  rt_printf("tBatterie : Debut de l'éxecution de periodique à 250ms\n");
  rt_task_set_periodic(NULL, TM_NOW, 250000000);
    
  while(1){
    /* Attente de l'activation périodique */
    rt_printf("Batterie : Activation périodique \n");
    rt_task_wait_period(NULL);
    
    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    status = etatCommRobot;
    rt_mutex_release(&mutexEtat);

    if (status == STATUS_OK) {
      status = d_robot_get_vbat(robot,etatBat);
      if(status == STATUS_OK){
	d_battery_set_level(batterie,*etatBat);
	
	rt_mutex_acquire(&mutexCompteurRobot, TM_INFINITE);
	compteurRobot=0;
	rt_mutex_release(&mutexCompteurRobot);
	
	message = d_new_message();
	d_message_put_battery_level(message,batterie);
	d_server_send(serveur,message);
      } else {
	rt_mutex_acquire(&mutexCompteurRobot, TM_INFINITE);
	if(compteurRobot<3){
	  compteurRobot++;
	} else if (compteurRobot == 3) {
	  rt_mutex_acquire(&mutexEtat, TM_INFINITE);
	  etatCommRobot = status;
	  rt_mutex_release(&mutexEtat);
	  message = d_new_message();
	  message->put_state(message, status);
	  rt_printf("tBatterie : Envoi message\n");
	  if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
	    message->free(message);
	  }
	  compteurRobot=0;
	}
	rt_mutex_release(&mutexCompteurRobot);
      }
    }
	
  }
}

void reloadWatchdog(void * arg){
  int status = 1; 
  DMessage *message;
    
  rt_printf("tWatchdog : Debut de l'éxecution de periodique à 1s\n");
  rt_task_set_periodic(NULL, TM_NOW, 1000000000);
    
  /*Attente du sémaphore*/
  rt_printf("tWatchdog : Attente du sémarphore semWathdog\n");
  rt_sem_p(&semWatchdog, TM_INFINITE);
  rt_printf("tWatchdog : Sempahore Watchdog prise\n");
  while(1){

    /* Attente de l'activation périotatCommCamera;dique */
    rt_task_wait_period(NULL);

    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
    status = etatCommRobot;
    rt_mutex_release(&mutexEtat);

    if (status == STATUS_OK) {
      status=d_robot_reload_wdt(robot);
      if(status == STATUS_OK){
	rt_mutex_acquire(&mutexCompteurRobot, TM_INFINITE);
	compteurRobot=0;
	rt_mutex_release(&mutexCompteurRobot);
      } else{
	rt_mutex_acquire(&mutexCompteurRobot, TM_INFINITE);
	if(compteurRobot<3){
	  compteurRobot++;
	}else if(compteurRobot == 3){
	  rt_mutex_acquire(&mutexEtat, TM_INFINITE);
	  etatCommRobot = status;
	  rt_mutex_release(&mutexEtat);

	  message = d_new_message();
	  message->put_state(message, status);

	  rt_printf("tWatchdog : Envoi message\n");
	  if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
	    message->free(message);
	  }
	  compteurRobot=0;
	}
	rt_mutex_release(&mutexCompteurRobot);
      }
    }
  }
}

void traitementImage(void *arg){
  
  int status;
  DMessage *message;
  DArena *arene ;
  DImage *frame;
  DPosition *pos;
  DJpegimage *jpegimage;

  rt_printf("ttraitementImage : Debut de l'éxecution periodique à 600ms\n");
  rt_task_set_periodic(NULL, TM_NOW, 600000000);
  
  /*Attente du sémaphore*/
  rt_printf("ttraitementImage : Attente du sémarphore semConnecterCamera\n");
  rt_sem_p(&semConnecterCamera, TM_INFINITE);
  rt_printf("ttraitementImage  : Ouverture de la communication avec la camera\n");
  d_camera_open(camera);

  while(1){
    /* Attente de l'activation périodique */
    rt_task_wait_period(NULL);
    rt_printf("ttraitementImage  : Activation périodique\n");
    
    frame = d_new_image();
    jpegimage = d_new_jpegimage();
    //if (status == STATUS_OK) {
    rt_mutex_acquire(&mutexCalibration, TM_INFINITE);
    status = calibration;
    rt_mutex_release(&mutexCalibration);
    if (status == 0) {
      rt_mutex_acquire(&mutexCalculPosition, TM_INFINITE);
      status = calculPosition;
      rt_mutex_release(&mutexCalculPosition);
      frame=d_new_image();
      d_camera_get_frame(camera, frame);
      if(status == 1) {
	/*Calcul position*/
	rt_printf("ttraitementImage : Calcul de la position \n");
	arene = d_new_arena();
	rt_mutex_acquire(&mutexAreneSauvegarde, TM_INFINITE);
	arene = areneSauvegarde;
	rt_mutex_release(&mutexAreneSauvegarde);
	
	if(arene == NULL) {
	  rt_printf("ttraitementImage  : Aucune arene sauvegarde, Veuillez calibrer \n");
	} else {
	  message = d_new_message();
	  pos = d_new_position();
	  pos = d_image_compute_robot_position(frame,NULL);
	  if(pos != NULL) { 
	    d_message_put_position(message,pos);
	    rt_printf("ttraitementImage  : Envoie de la position du robot \n");
	    if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
	      message->free(message);
	    }
	    d_imageshop_draw_position(frame,pos);
	  }
	}
      }
      d_jpegimage_compress(jpegimage,frame);
      message = d_new_message();
      d_message_put_jpeg_image(message,jpegimage);
      rt_printf("ttraitementImage  : Envoie de l'image \n");
      if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
	message->free(message);
      }
    }
  }
}


void calibrationArene (void *arg)
{

  DMessage *message;
  int calibrage;
  DArena *arene ;
  DImage *frame ;
  DJpegimage *jpeg ;
  rt_task_set_periodic (NULL, TM_NOW, 650000000);
  rt_printf ("tcalibrationArene : Debut de l'exécution de tcalibration\n");

  while (1)
    {
      rt_printf("tcalibrationArene : Attente du sémarphore semCalibrationArene\n");
      rt_sem_p (&semCalibrationArene, TM_INFINITE);
      rt_printf("tcalibrationArene : Debut de la calibration\n");

      rt_mutex_acquire (&mutexTypeCalibration, TM_INFINITE);
      calibrage = typeCalibration;
      rt_mutex_release (&mutexTypeCalibration);

      switch (calibrage)
	{
	  /* Detecter Arene */
	case 1:
	  rt_mutex_acquire (&mutexCalibration, TM_INFINITE);
	  calibration = 1;
	  rt_mutex_release (&mutexCalibration);
	  rt_task_wait_period (NULL); //Attente 650ms, le temps que traitement image ait fini dans le pire des cas
	  
	  frame = d_new_image();
	  d_camera_get_frame (camera, frame);
	  arene = d_new_arena();
	  arene = d_image_compute_arena_position (frame);
	  d_imageshop_draw_arena (frame, arene);
	  jpeg = d_new_jpegimage();
	  d_jpegimage_compress (jpeg, frame);
	  message = d_new_message();
	  d_message_put_jpeg_image (message, jpeg);
	  rt_printf ("tcalibrationArene : Envoi message\n");
	  message->print (message, 100);

	  if (write_in_queue (&queueMsgGUI, message, sizeof (DMessage)) < 0)
	    {
	      message->free (message);
	    }
	  break;
	  /* Action Arena Failed */
	case 2:
	  rt_mutex_acquire (&mutexCalibration, TM_INFINITE);
	  calibration = 0;
	  rt_mutex_release (&mutexCalibration);
	  break;
	  /* Action Arena Ok */
	case 3:
	  rt_mutex_acquire (&mutexAreneSauvegarde, TM_INFINITE);
	  areneSauvegarde = arene;
	  rt_mutex_release (&mutexAreneSauvegarde);
	  rt_mutex_acquire (&mutexCalibration, TM_INFINITE);
	  calibration = 0;
	  rt_mutex_release (&mutexCalibration);
	  break;
	}
    }
}

int write_in_queue (RT_QUEUE * msgQueue, void *data, int size)
{
  void *msg;
  int err;

  msg = rt_queue_alloc (msgQueue, size);
  memcpy (msg, &data, size);

  if ((err = rt_queue_send (msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0)
    {
      rt_printf ("Error msg queue send: %s\n", strerror (-err));
    }
  rt_queue_free (&queueMsgGUI, msg);

  return err;
}






