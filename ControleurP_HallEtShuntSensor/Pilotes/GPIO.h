

/******************************************************************************
* File Name          : GPIO.c 
* Author             : T Rocacher
* Version            : _
* d�pendance				 : _
* Date release			 : 08/11/2024
* Description        : Driver GPIO 
* Processeur				 : STM32F103, 
* Outil Developpement: KEIL 5.40.0.3
* Documentation			 : _
********************************************************************************/

#include "stm32f10x.h"
#ifndef _GPIO_H__
#define _GPIO_H__


/*=======================================================================================
 Rappel sur les ressources 
 Broche : de 0 � 15
 GPIOA, GPIOB, GPIOC (STM103)
 GPIOD et GPIOE pour d'autres uC
	
=======================================================================================*/


/**
 * @name   GPIO_Configure(...)
 * @Brief  La fonction initialise n'importe quelle broche de port (entr�e, sortie, techno....)
 * @note : _
 * @param : GPIO_TypeDef * Port (GPIOA � GPIOE)
 * @param : int Broche (0 � 15)
 * @param : int Sens ( cf #define)
 * @param : int Techno (cf #define)
 * @ret : 0 si pas de soucis, 1 si pb.
 * @EXAMPLE�Port_IO_Init(GPIOB, 8, OUTPUT, OUTPUT_PPULL);
						Place la pin 8 du port B en sortie Push-pull
**/
char GPIO_Configure(GPIO_TypeDef * Port, int Broche, int Sens, int Techno);


/* D�finition pour l'argument Sens*/
#define INPUT 				'i'
#define OUTPUT 				'o'

/* D�finition pour l'argument Techno*/
/* Techno pour pin en entr�e (INPUT)*/
#define ANALOG 				0
#define INPUT_FLOATING 		1
#define INPUT_PULL_DOWN_UP 	2

/* Techno pour pin en sortie (OUTPUT) */
#define OUTPUT_PPULL 		0
#define OUTPUT_OPDRAIN 		1
#define ALT_PPULL 			2
#define ALT_OPDRAIN 		3


/*______________________________________________________________________________

		Macros
______________________________________________________________________________*/
#define GPIO_Set(GPIO,Broche) GPIO->BSRR=(0x01<<Broche)
/* exemple : Port_IO_Set(GPIOB,8); */

#define GPIO_Clear(GPIO,Broche) GPIO->BRR=(0x01<<Broche)
/* exemple : GPIO_Clear(GPIOB,8); */

#define GPIO_Toggle(GPIO,Broche) GPIO->ODR=(GPIO->ODR)^(0x01<<Broche)

#define GPIO_Write(GPIO,Broche,Val) GPIO->BRR=(0x01<<Broche); \
                                    GPIO->ODR|=(Val<<Broche)
/* exemple :  Val=1;
							GPIO_Write(GPIOB,8,Val); */



/**
* @name   Macro : GPIO_Read(GPIO,Broche)
 * @Brief  Lit la Pin demand�e, 
						renvoie 0 si la pin est � 0, 1<<Broche sinon
 * @note : _
 * @param : GPIO = GPIOA � GPIOE
 * @param : Broche = 0 � 15
 * @ret : 0 ou 1<< Broche
 * @EXAMPLE�Port_IO_Init(GPIOB, 8, OUTPUT, OUTPUT_PPULL);
						Place la pin 8 du port B en sortie Push-pull
**/
#define GPIO_Read(GPIO,Broche) ((GPIO->IDR)&(0x01<<Broche))


/**
* @name   Macro : GPIO_Write_Multi_bits(GPIO,Masque32Bits)
 * @Brief  Met � 1 et � 0 un ensemble de bits, simultan�ment
 * @note : _
 * @param : GPIO = GPIOA � GPIOE
 * @param : Masque32Bits, indique sur les 16 bits de poids forts
						ceux devant �tre mis � 0, sur les 16 bits de poids faible
						ceux devant �tre mis � 0
 * @ret :  _
 * @EXAMPLE� on veut mettre � 1 le bit 15 et mettre le bit 6 � 0, la valeur du masque sera :
						Masque32Bits = b 0000 0000 0010 0000   1000 0000 0000 0000 = 0x0020 8000
**/
#define GPIO_Write_Multi_bits(GPIO,Masque32Bits) (GPIO->BSRR)=Masque32Bits


//___________________________FIN ___________________________________________________
// suite pour r�tro compatibilit�...

/* Retro-compatibilit� avec les noms de fonction de Thierry */
#define Port_IO_Init	GPIO_Configure
#define Port_IO_Set 	GPIO_Set 
#define Port_IO_Clr 	GPIO_Clear
#define Port_IO_Toggle 	GPIO_Toggle
#define Port_IO_Lire 	GPIO_Read

#endif /* _GPIO_H__ */
