

/******************************************************************************
* File Name          : Timer_1234.c 
* Author             : T Rocacher
* Version            : _
* dépendance				 : Clock_F103
* Date release			 : 08/11/2024
* Description        : Driver gérant TIM1 à TIM4
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
  Chacun d'entre eux dispose de 4 voies de sorties numérotées de 1 à 4
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
 * @Brief  Le prescaler est calculé le plus petit possible pour une précision la plus 
						fine possible). Le Timer est lancé en mode UP et non down.
 * @note : Lancer avant la fonction d'initialisétion du système d'horloge. 
 * @param : TIM_TypeDef *Timer, TIM1 à TIM4
 * @param : float Duree_us , durée en float exprimée en µs
 * @ret : float , durée véritable, au plus proche de la valeur souhaitée
 * @EXAMPLE Timer_1234_Init(TIM2, 1000.0 );   
						// Lance le timer avec une périodicité de 1.0 ms. 
**/
float Timer_1234_Init(TIM_TypeDef *Timer, float Duree_us );

/**
 * @name   Active_IT_Debordement_Timer(..)
 * @Brief  La fonction initialise le périphérique et le NVIC de manière à
						générer une interruption à chaque débordement du timer précisé.
 * @note : La fonction passée en paramètre, forcément de type 
					void Toto(void) est lancée sous interruption, Prio fixe la priorité
 * @param : TIM_TypeDef *Timer, TIM1 à TIM4
 * @param : char Prio de 0 à 15
 * @param :  void (*IT_function) (void), la fonction callback
 * @ret : float , durée véritable, au plus proche de la valeur souhaitée
 * @EXAMPLE Timer_1234_Init(TIM2, 1000.0 );  // Lance le timer avec une périodicité de 1.0 ms. 
						
**/
void Active_IT_Debordement_Timer( TIM_TypeDef *Timer, char Prio, void (*IT_function) (void));


/**
 * @name   Active_IT_Compare_Timer(..)
 * @Brief  La fonction initialise le périphérique et le NVIC de manière à
						générer une interruption lors de la comparaison vraie avec CCRx,
						x correspondant au channel du timer souhaité
 * @note : _
 * @param : TIM_TypeDef *Timer, TIM1 à TIM4
 * @param : char Voie de 1 à 4 : le canal choisi
 * @param : char Prio de 0 à 15
 * @param :  void (*IT_function) (void), la fonction callback
 * @ret : _
 * @EXAMPLE _ 
**/
void Active_IT_Compare_Timer( TIM_TypeDef *Timer, char Voie, char Prio, void (*IT_function) (void));



/*=======================================================================================
 Timers en PWM
=======================================================================================*/

/**
 * @name   Active_IT_Compare_Timer(..)
 * @Brief  Cette fonction initialise la voie spécifiée du timer spécifié en PWM.
					 La fréquence souhaitée est passée en paramètre.
					 La fonction renvoie un entier qui correspond à la résolution de la PWM 
					 pour pouvoir ensuite régler les rapports cycliques (Resolution = ARR+1)
					 3 Timer "general Purpose", TIM2, TIM3 et TIM4 + TIM1
					 Chacun d'entre eux dispose de 4 voies de sorties numérotées de 1 à 4
					 Mapping des IO, voir début du .h.
 * @note : dans les fonctions d'init PWM, ajout de ARPE = 1,  ce qui permet de 
					 synchroniser les actions de chargement CCR donc modif rcy.
 * @param : TIM_TypeDef *Timer, TIM1 à TIM4
 * @param : char Voie de 1 à 4 : le canal choisi
 * @param : float Frequence_PWM_Khz
 * @ret : int : Résolution effective de la PWM réglée
 * @EXAMPLE Reso = PWM_Init(TIM2, 2, 20.0);
					La fréquence de PWM vaut 20kHz. La PWM est disponible sur la voie 2 du Timer 2
					Renvoie la valeur 3600, ce qui veut dire que PWM_Valeur(TIM2,2) = 3600 
					donne 100% de rapport cyclique
**/
vu16 PWM_Init(TIM_TypeDef *Timer, char Voie, float Frequence_PWM_Khz);


/**
 * @name   PWM_Complementaire_Timer1(..)
 * @Brief  petite fonction qui configure le timer supposé déjà en PWM
					de manière à sortir également la PWM complémentaire sur l'un des 
					3 channels ou c'est possible : 1, 2 et 3.
					Ce sont donc les channel CH1N, CH2N et CH3N qui s'activent en opposition
					de phase avec leur homoloque CH1, CH2, CH3.
 * @note : les dead time ne sont pas gérés, donc à priori à 0.
 * @param : char Voie de 1 à 4 : le canal choisi
 * @ret : _
 * @EXAMPLE _
**/
void PWM_Complementaire_Timer1(char Voie);


/*
Permet un accès direct au compteur du Timer spécifié sans avoir à connaître 
les registres du STM32
*/
#define CNT(Timer) Timer->CNT

/*
Permet un accès direct à l'autoreload du Timer (modulo du timer) spécifié sans 
avoir à connaître les registres du STM32
*/
#define ARR(Timer) Timer->ARR

/*
Permet de mettre à 0 le flag d'overflow UIF
*/
#define Clear_Flag_Overflow(Timer) 	Timer->SR = ((Timer->SR)&~0x01)

/*
Permet de définir le sens de comptage
*/
#define CptUp(Timer) Timer->CR1=Timer->CR1&~(1<<4)
#define CptDown(Timer) Timer->CR1=Timer->CR1|(1<<4)

/*
remet à 0 le compteur
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
Par défaut, après l'init de PWM, la polarité est positive. Si besoin est de l'inverser, 
il suffit d'écrire cette ligne, par exemple :
PWM_Polarite_Neg(TIM2,3) aura pour effet d'inverser la polarité de la PWM voie 3 du TIM2
*/
#define PWM_Polarite_Neg(Timer,Voie) Timer->CCER = (Timer->CCER)|(0x01<<((Voie-1)*4+1))


/*
Permet de fixer  la durée à l'état haut de la PWM, dont indirectement son rapport 
cyclique. La grandeur doit être comprise entre 0 et ARR. Ex:
Reso = PWM_Init (TIM3,2,25.0);
PWM_Valeur(TIM3,2) = Reso /4; // arrondi à gerer
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
 Timers en mode incrémental
=======================================================================================*/

/*  3 Timer "general Purpose", TIM2, TIM3 et TIM4
  Chacun d'entre eux dispose de 4 voies de sorties numérotées de 1 à 4
  Mapping des IO:
  	TM2_CH1_ETR 	- PA0 	TM3_CH1 - PA6		TIM4_CH1 - PB6	
    TM2_CH2 			- PA1		TM3_CH2 - PA7		TIM4_CH2 - PB7

NB : Dans ce mode, le timerx utilise les 2 entrées 1 et 2
*/


/**
 * @name   Timer_Inc_Init(..)
 * @Brief  Fonction de configuration du timer en question en mode codeur incrémental
					 Compte les fronts montant et descendant. Selon la configuration désirée
 * @note : la résolution est de ½ période du channel 1, ou  2, ou encore d' ¼ de période 
				   en comptant sur les deux voies.	
 * @note : C'est au user de configurer des canaux  en entrée !! Le cnt est lancé.
 * @param : TIM_TypeDef *Timer, TIM1 à TIM4
 * @param : char Resolution , voir les défintions pour faire le choix
 * @ret : 0 si pas d'erreur, 1 sinon
 * @EXAMPLE Timer_1234_Init(TIM2, 1000.0 );   
						// Lance le timer avec une périodicité de 1.0 ms. 
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
 * @Brief  Le Timer considéré est configuré en capture. La fonction permet de 
					 mesurer une largeur d'impulsion. Elle est associée à la fonction 
					 vu16 Lire_Duree_Pulse(TIM_TypeDef *Timer,int Voie).
					 La grandeur renvoyée par cette dernière fonction est proportionnelle à 
					 la durée de l'impulsion.
 * @note : Largeur_Pulse_Max_us est la durée la plus grande mesurable en µs
					 Resolution  est la valeur maximale souhaitée pour la grandeur de lecture
					On a donc la correspondance :
Durée impulsion :  -------->  Grandeur renvoyée par la fonction de lecture
0 µs ---------------------->  0
Largeur_Pulse_Max_us ------>  Resolution
 * @note : La fonction Capture_Init n'est pas forcément capable de faire la correspondance
					 exacte. C'est pourquoi elle renvoie la Resolution exacte, afin que l'utilisateur 
					 puisse s'adapter. D'autre part, si la configuration voulue va au delà des limites 
					 physiques, la valeur de résolution renvoyée est la mini/maxi possible.
 * @note : La mesure repose sur une interruption déclenchée sur front montant et sur front 
						descendant. Le sens du front est inversé à chaque interruption. 
 * @note : C'est au user de configurer l'IO correspondant  à la voie du timer,  en entrée 
 * @param : TIM_TypeDef *Timer, TIM1 à TIM4
 * @param : char voie, de 1 à 4
 * @param : char Resolution , voir les défintions pour faire le choix
 * @param : float Largeur_Pulse_Max_us : la largeur en µs max qu'on veut mesurer
 * @param : vu16 Resolution : la résolution qu'on veut voir associée à la durée max
 * @param : char Prio : la priorité associée à l'IT qui opère la mesure (de 0 à 15)
 * @ret : vu16, la résolution effective.
 * @EXAMPLE Val=Capture_Init(TIM2, 2, 300.0, 1000, 1); // prio = 0
						Renvoie la valeur 1028.
						On a donc :
						0 µs ---------------------->  0
						300µs ------>  1028 (et non 1000)
				(PSC calculé vaut 0x14 (20). Avec Fck = 72MHz, le timer avance à 72/(20+1) = 3 428kHz.
				Ainsi, pour avoir 300µs il faut compter 300µs*3 428kHz= 1028
**/
vu16 Capture_Init(TIM_TypeDef *Timer, char Voie, float Largeur_Pulse_Max_us, vu16 Resolution, char Prio);


/**
 * @name   Lire_Duree_Pulse(..)
 * @Brief  Le rôle de la fonction est de retourner le comptage opéré pendant l'impulsion. 
					 Le nombre évolue de 0 à Largeur_Pulse_Max_us (spécifié lors de l'appel à 
					 Capture_Init)
 * @param : TIM_TypeDef *Timer, TIM1 à TIM4
 * @param : int voie, de 1 à 4
 * @ret : nombre proportionnel à la durée mesurée.
**/
vu16 Lire_Duree_Pulse(TIM_TypeDef *Timer,int Voie);


#endif
