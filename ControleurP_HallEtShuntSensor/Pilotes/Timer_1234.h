

/******************************************************************************
* File Name          : Timer_1234.c 
* Author             : T Rocacher
* Version            : _
* d�pendance				 : Clock_F103
* Date release			 : 08/11/2024
* Description        : Driver g�rant TIM1 � TIM4
* Processeur				 : STM32F103, 
* Outil Developpement: KEIL 5.40.0.3
* Documentation			 : _
********************************************************************************/


#include "stm32f10x.h"


#ifndef _TIMER_1234_H__
#define _TIMER_1234_H__

/*=======================================================================================
 Rappel sur les ressources 


  3 Timers "general Purpose", TIM2, TIM3 et TIM4 + TIM1
  Chacun d'entre eux dispose de 4 voies de sorties num�rot�es de 1 � 4
  Mapping des IO:
  	TIM1_CH1 - PA08       TM2_CH1_ETR - PA0 TM3_CH1 - PA6		TIM4_CH1 - PB6	
    TIM1_CH2 - PA09       TM2_CH2 - PA1		  TM3_CH2 - PA7		TIM4_CH2 - PB7
 		TIM1_CH3 - PA10       TM2_CH3 - PA2			TM3_CH3 - PB0		TIM4_CH3 - PB8
 		TIM1_CH4 - PA11		    TM2_CH4 - PA3			TM3_CH4 - PB1		TIM4_CH4 - PB9
	  TIM1_CH1N - PB13
	  TIM1_CH2N - PB14
	  TIM1_CH3N - PB15
//=======================================================================================*/



/*=======================================================================================
		Fonctions basiques des timers
=======================================================================================*/

/**
 * @name   Timer_1234_Init(..)
 * @Brief  Le prescaler est calcul� le plus petit possible pour une pr�cision la plus 
						fine possible). Le Timer est lanc� en mode UP et non down.
 * @note : Lancer avant la fonction d'initialis�tion du syst�me d'horloge. 
 * @param : TIM_TypeDef *Timer, TIM1 � TIM4
 * @param : float Duree_us , dur�e en float exprim�e en �s
 * @ret : float , dur�e v�ritable, au plus proche de la valeur souhait�e
 * @EXAMPLE�Timer_1234_Init(TIM2, 1000.0 );   
						// Lance le timer avec une p�riodicit� de 1.0 ms. 
**/
float Timer_1234_Init(TIM_TypeDef *Timer, float Duree_us );

/**
 * @name   Active_IT_Debordement_Timer(..)
 * @Brief  La fonction initialise le p�riph�rique et le NVIC de mani�re �
						g�n�rer une interruption � chaque d�bordement du timer pr�cis�.
 * @note : La fonction pass�e en param�tre, forc�ment de type 
					void Toto(void) est lanc�e sous interruption, Prio fixe la priorit�
 * @param : TIM_TypeDef *Timer, TIM1 � TIM4
 * @param : char Prio de 0 � 15
 * @param :  void (*IT_function) (void), la fonction callback
 * @ret : float , dur�e v�ritable, au plus proche de la valeur souhait�e
 * @EXAMPLE�Timer_1234_Init(TIM2, 1000.0 );  // Lance le timer avec une p�riodicit� de 1.0 ms. 
						
**/
void Active_IT_Debordement_Timer( TIM_TypeDef *Timer, char Prio, void (*IT_function) (void));


/**
 * @name   Active_IT_Compare_Timer(..)
 * @Brief  La fonction initialise le p�riph�rique et le NVIC de mani�re �
						g�n�rer une interruption lors de la comparaison vraie avec CCRx,
						x correspondant au channel du timer souhait�
 * @note : _
 * @param : TIM_TypeDef *Timer, TIM1 � TIM4
 * @param : char Voie de 1 � 4 : le canal choisi
 * @param : char Prio de 0 � 15
 * @param :  void (*IT_function) (void), la fonction callback
 * @ret : _
 * @EXAMPLE�_ 
**/
void Active_IT_Compare_Timer( TIM_TypeDef *Timer, char Voie, char Prio, void (*IT_function) (void));



/*=======================================================================================
 Timers en PWM
=======================================================================================*/

/**
 * @name   Active_IT_Compare_Timer(..)
 * @Brief  Cette fonction initialise la voie sp�cifi�e du timer sp�cifi� en PWM.
					 La fr�quence souhait�e est pass�e en param�tre.
					 La fonction renvoie un entier qui correspond � la r�solution de la PWM 
					 pour pouvoir ensuite r�gler les rapports cycliques (Resolution = ARR+1)
					 3 Timer "general Purpose", TIM2, TIM3 et TIM4 + TIM1
					 Chacun d'entre eux dispose de 4 voies de sorties num�rot�es de 1 � 4
					 Mapping des IO, voir d�but du .h.
 * @note : dans les fonctions d'init PWM, ajout de ARPE = 1,  ce qui permet de 
					 synchroniser les actions de chargement CCR donc modif rcy.
 * @param : TIM_TypeDef *Timer, TIM1 � TIM4
 * @param : char Voie de 1 � 4 : le canal choisi
 * @param : float Frequence_PWM_Khz
 * @ret : int : R�solution effective de la PWM r�gl�e
 * @EXAMPLE�Reso = PWM_Init(TIM2, 2, 20.0);
					La fr�quence de PWM vaut 20kHz. La PWM est disponible sur la voie 2 du Timer 2
					Renvoie la valeur 3600, ce qui veut dire que PWM_Valeur(TIM2,2) = 3600 
					donne 100% de rapport cyclique
**/
vu16 PWM_Init(TIM_TypeDef *Timer, char Voie, float Frequence_PWM_Khz);


/**
 * @name   PWM_Complementaire_Timer1(..)
 * @Brief  petite fonction qui configure le timer suppos� d�j� en PWM
					de mani�re � sortir �galement la PWM compl�mentaire sur l'un des 
					3 channels ou c'est possible : 1, 2 et 3.
					Ce sont donc les channel CH1N, CH2N et CH3N qui s'activent en opposition
					de phase avec leur homoloque CH1, CH2, CH3.
 * @note : les dead time ne sont pas g�r�s, donc � priori � 0.
 * @param : char Voie de 1 � 4 : le canal choisi
 * @ret : _
 * @EXAMPLE�_
**/
void PWM_Complementaire_Timer1(char Voie);


/*
Permet un acc�s direct au compteur du Timer sp�cifi� sans avoir � conna�tre 
les registres du STM32
*/
#define CNT(Timer) Timer->CNT

/*
Permet un acc�s direct � l'autoreload du Timer (modulo du timer) sp�cifi� sans 
avoir � conna�tre les registres du STM32
*/
#define ARR(Timer) Timer->ARR

/*
Permet de mettre � 0 le flag d'overflow UIF
*/
#define Clear_Flag_Overflow(Timer) 	Timer->SR = ((Timer->SR)&~0x01)

/*
Permet de d�finir le sens de comptage
*/
#define CptUp(Timer) Timer->CR1=Timer->CR1&~(1<<4)
#define CptDown(Timer) Timer->CR1=Timer->CR1|(1<<4)

/*
remet � 0 le compteur
*/
#define Reset_Timer(Timer) Timer->CNT=0

/*
bloque le timer (CEN=0)
*/
#define Bloque_Timer(Timer) Timer->CR1=(Timer->CR1)&~(1<<0)

/*
Lance timer (CEN=1)
*/
#define Run_Timer(Timer) Timer->CR1=(Timer->CR1)|(1<<0)


/*
Par d�faut, apr�s l'init de PWM, la polarit� est positive. Si besoin est de l'inverser, 
il suffit d'�crire cette ligne, par exemple :
PWM_Polarite_Neg(TIM2,3) aura pour effet d'inverser la polarit� de la PWM voie 3 du TIM2
*/
#define PWM_Polarite_Neg(Timer,Voie) Timer->CCER = (Timer->CCER)|(0x01<<((Voie-1)*4+1))


/*
Permet de fixer  la dur�e � l'�tat haut de la PWM, dont indirectement son rapport 
cyclique. La grandeur doit �tre comprise entre 0 et ARR. Ex:
Reso = PWM_Init (TIM3,2,25.0);
PWM_Valeur(TIM3,2) = Reso /4; // arrondi � gerer
*/
#define PWM_Valeur(Timer,Voie) Timer->CCR##Voie


/* Permet de valider un canal PWM*/
#define PWM_ChannelEnable(Timer,Voie)  (Timer->CCER = (Timer->CCER)|(0x01<<((Voie-1)*4)))

/* Permet d' invalider un canal PWM*/
#define PWM_ChannelDisable(Timer,Voie) (Timer->CCER = (Timer->CCER)& ~(0x01<<((Voie-1)*4)))

/* Edge / center aligned */
#define PWM_Edge_Aligned(Timer) Timer->CR1=Timer->CR1&~(3<<5)
#define PWM_Center_Aligned(Timer) Timer->CR1=Timer->CR1|(3<<5)

/*=======================================================================================
 Timers en mode incr�mental
=======================================================================================*/

/*  3 Timer "general Purpose", TIM2, TIM3 et TIM4
  Chacun d'entre eux dispose de 4 voies de sorties num�rot�es de 1 � 4
  Mapping des IO:
  	TM2_CH1_ETR 	- PA0 	TM3_CH1 - PA6		TIM4_CH1 - PB6	
    TM2_CH2 			- PA1		TM3_CH2 - PA7		TIM4_CH2 - PB7

NB : Dans ce mode, le timerx utilise les 2 entr�es 1 et 2
*/


/**
 * @name   Timer_Inc_Init(..)
 * @Brief  Fonction de configuration du timer en question en mode codeur incr�mental
					 Compte les fronts montant et descendant. Selon la configuration d�sir�e
 * @note : la r�solution est de � p�riode du channel 1, ou  2, ou encore d' � de p�riode 
				   en comptant sur les deux voies.	
 * @note : C'est au user de configurer des canaux  en entr�e !! Le cnt est lanc�.
 * @param : TIM_TypeDef *Timer, TIM1 � TIM4
 * @param : char Resolution , voir les d�fintions pour faire le choix
 * @ret : 0 si pas d'erreur, 1 sinon
 * @EXAMPLE�Timer_1234_Init(TIM2, 1000.0 );   
						// Lance le timer avec une p�riodicit� de 1.0 ms. 
**/
char Timer_Inc_Init(TIM_TypeDef *Timer, char Resolution);

#define Reso_Demie_Per_Ch1 2
#define Reso_Demie_Per_Ch2 1
#define Reso_Quart_Per 3



/*=======================================================================================
Timers en mode capture
=======================================================================================*/

/**
 * @name   Capture_Init(..)
 * @Brief  Le Timer consid�r� est configur� en capture. La fonction permet de 
					 mesurer une largeur d'impulsion. Elle est associ�e � la fonction 
					 vu16 Lire_Duree_Pulse(TIM_TypeDef *Timer,int Voie).
					 La grandeur renvoy�e par cette derni�re fonction est proportionnelle � 
					 la dur�e de l'impulsion.
 * @note : Largeur_Pulse_Max_us est la dur�e la plus grande mesurable en �s
					 Resolution  est la valeur maximale souhait�e pour la grandeur de lecture
					On a donc la correspondance :
Dur�e impulsion :  -------->  Grandeur renvoy�e par la fonction de lecture
0 �s ---------------------->  0
Largeur_Pulse_Max_us ------>  Resolution
 * @note : La fonction Capture_Init n'est pas forc�ment capable de faire la correspondance
					 exacte. C'est pourquoi elle renvoie la Resolution exacte, afin que l'utilisateur 
					 puisse s'adapter. D'autre part, si la configuration voulue va au del� des limites 
					 physiques, la valeur de r�solution renvoy�e est la mini/maxi possible.
 * @note : La mesure repose sur une interruption d�clench�e sur front montant et sur front 
						descendant. Le sens du front est invers� � chaque interruption. 
 * @note : C'est au user de configurer l'IO correspondant  � la voie du timer,  en entr�e 
 * @param : TIM_TypeDef *Timer, TIM1 � TIM4
 * @param : char voie, de 1 � 4
 * @param : char Resolution , voir les d�fintions pour faire le choix
 * @param : float Largeur_Pulse_Max_us : la largeur en �s max qu'on veut mesurer
 * @param : vu16 Resolution : la r�solution qu'on veut voir associ�e � la dur�e max
 * @param : char Prio : la priorit� associ�e � l'IT qui op�re la mesure (de 0 � 15)
 * @ret : vu16, la r�solution effective.
 * @EXAMPLE�Val=Capture_Init(TIM2, 2, 300.0, 1000, 1); // prio = 0
						Renvoie la valeur 1028.
						On a donc :
						0 �s ---------------------->  0
						300�s ------>  1028 (et non 1000)
				(PSC calcul� vaut 0x14 (20). Avec Fck = 72MHz, le timer avance � 72/(20+1) = 3 428kHz.
				Ainsi, pour avoir 300�s il faut compter 300�s*3 428kHz= 1028
**/
vu16 Capture_Init(TIM_TypeDef *Timer, char Voie, float Largeur_Pulse_Max_us, vu16 Resolution, char Prio);


/**
 * @name   Lire_Duree_Pulse(..)
 * @Brief  Le r�le de la fonction est de retourner le comptage op�r� pendant l'impulsion. 
					 Le nombre �volue de 0 � Largeur_Pulse_Max_us (sp�cifi� lors de l'appel � 
					 Capture_Init)
 * @param : TIM_TypeDef *Timer, TIM1 � TIM4
 * @param : int voie, de 1 � 4
 * @ret : nombre proportionnel � la dur�e mesur�e.
**/
vu16 Lire_Duree_Pulse(TIM_TypeDef *Timer,int Voie);


#endif
