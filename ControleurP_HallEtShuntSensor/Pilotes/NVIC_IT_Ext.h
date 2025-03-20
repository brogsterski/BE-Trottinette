

/******************************************************************************
* File Name          : NVIC_IT_Ext.c 
* Author             : T Rocacher
* Version            : _
* dépendance				 : Module GPIO.c
* Date release			 : 08/11/2024
* Description        : Driver pour les IT GPIO
* Processeur				 : STM32F103, 
* Outil Developpement: KEIL 5.40.0.3
* Documentation			 : _
********************************************************************************/

#include "stm32f10x.h"
#ifndef _NVIC_IT_EXT_H__
#define _NVIC_IT_EXT_H__

/**
 * @name    NVIC_Enable_IT(..)
 * @Brief  Permet de valider une ligne d'interruption, hors coeur, de 0 à 63. Le numéro 
		est passé en paramètre.
 * @note : _	
 * @param : char NumeroIT (0 à 63, canal NVIC)
 * @ret : _
 * @EXAMPLE : NVIC_Enable_IT(28); // valide le TIMZ au niveu NVIC
**/
void NVIC_Enable_IT(char NumeroIT);


/**
 * @name   NVIC_Disable_IT(..)
 * @Brief  Permet d' invalider une ligne d'interruption, hors coeur, de 0 à 63. Le numéro 
		est passé en paramètre.
 * @note : _	
 * @param : char NumeroIT (0 à 63, canal NVIC)
 * @ret : _
 * @EXAMPLE : NVIC_Disable_IT(28); // invalide le TIMZ au niveu NVIC
**/
void NVIC_Disable_IT(char NumeroIT);

/**
 * @name   NVIC_Prio_IT(..)
 * @Brief  Fixe la priorité de la ligne d'interruption spécifiée.
 * @note : _	
 * @param : char NumeroIT (0 à 63, canal NVIC)
 * @param : Prio (0 le plus prioritaire à 15 le moins prioritaire)
 * @ret : _
 * @EXAMPLE : NVIC_Prio_IT(28,2); // Priorité 2 pour le TIM2
**/
void NVIC_Prio_IT(char NumeroIT, char Prio);



/**
 * @name   NVIC_Ext_IT (..)
 * @Brief  place la broche du port indiquée en entree IT sur front up/down/les deux
					La fonction est "clé en main" pour configurer l'IT sur une broche externe 
					voie 0 à 4, port A, B ou C), flag remis à 0 automatiquement lors de l'appel
					au callback.
 * @note :  Pour les broches, de 5 à 15, le STM32 n'utilise que 2 vecteurs d'IT.
						C'est pourquoi, contrairement aux broches 0 à 4, c'est au user de chercher
						le flag levé, et de le remettre à 0.
						Ce flag, c'est le bit de même rang que la broche, du registre EXTI->PR

						Exemple, remise à 0 du flag correspondant à la broche 10:
						Clear_Flag_IT_Ext_5_15(10)
 * @param : GPIO_TypeDef * Port (GPIOA à GPIOE)
 * @param : char Broche : la pin considérée
 * @param : char Front : montant, descendant ou les deux voir définitions...
 * @param : char Techno : voir les valeurs possibles dans GPIO.h (entrée seulement !)
 * @param : Prio (0 le plus prioritaire à 15 le moins prioritaire)
 * @param :  void (*IT_function) (void) ; le callback associé
 * @ret : Retourne 1 si erreur, 0 sinon
 * @EXAMPLE : NVIC_Ext_IT (GPIOA, 2, RISING_EDGE, INPUT_FLOATING,8,Cligno);
 La broche GPIOA.2 provoque au front montant une IT qui lance la fct Cligno(), avec
 une priorité de 8. L'entrée est flottante.
 
**/
char NVIC_Ext_IT (GPIO_TypeDef * Port, char Broche, char Front, char Techno, char Prio, void (*IT_function) (void));
/* define des fronts */
#define FALLING_EGDE 0
#define RISING_EDGE 1
#define FALLING_RISING_EDGE 2



#define Clear_Flag_IT_Ext_5_15(Voie) EXTI->PR=(EXTI->PR)|(0x01<<Voie)


#endif
