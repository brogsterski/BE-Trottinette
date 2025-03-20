

/******************************************************************************
* File Name          : ADC_DMA.c 
* Author             : T Rocacher
* Version            : _
* dépendance				 : Module Timer_1234 et Clock_F103
* Date release			 : 08/11/2024
* Description        : Driver ADC avec implémentation DMA
* Processeur				 : STM32F103, 
* Outil Developpement: KEIL 5.40.0.3
* Documentation			 : _
********************************************************************************/


#include "stm32f10x.h"



#ifndef _ADC_DMA_H__
#define _ADC_DMA_H__

/*=======================================================================================
 Rappel sur les ressources 

  - 2ADC 16 canaux, 12 bits
  - Les broches :
		- ADC_In0  PA0
		- ADC_In1	PA1
		- ADC_In2	PA2
		- ADC_In3	PA3
		- ADC_In4	PA4
		- ADC_In5 	PA5
		- ADC_In6	PA6
		- ADC_In7	PA7
		- ADC_In8	PB0
		- ADC_In9	PB1
		- ADC_In10	PC0
		- ADC_In11	PC1
		- ADC_In12	PC2
		- ADC_In13	PC3
		- ADC_In14	PC4
		- ADC_In15	PC5

 	-  ADCck Maxi = 14MHz
	
=======================================================================================*/

/**
 * @name   Init_TimingADC_ActiveADC(...) 
 * @Brief  La fonction active le priphérique ADC (1 ou 2) avec un temps d'acquisition donné
  (Le temps d'acquisition (Duree_Ech_us) est le temps de fermeture de l'interrupteur de 
	l'échantillonneur bloqueur)
	La fonction renvoie la durée réelle de la durée de conversion TOTALE (Tacq+Tconv).
 * @note : Fonction de configuration incontournable (sauf si DMA multiple channel continu)
	
 * @param : ADC_TypeDef * ADC , ADC1 ou ADC2
 * @param : float Duree_Ech_us (en us)
 * @ret : _
 * @EXAMPLE Duree=Init_TimingADC_ActiveADC( ADC1, 1.5 );
**/
float Init_TimingADC_ActiveADC( ADC_TypeDef * ADC, float Duree_Ech_us );


/*=======================================================================================
			Fonctions de configuration Single 
=======================================================================================*/

/**
 * @name   Single_Channel_ADC() 
 * @Brief  La fonction spécifie le canal à traiter. Elle ne lance pas
 * @note : La macro Start_ADC_Conversion(ADC) démarre la conversion 
					sur le channel spécifié.  
	
 * @param : ADC_TypeDef * ADC , ADC1 ou ADC2
 * @param : char Voie_ADC (0 à 15) 
 * @ret : _
 * @EXAMPLE Single_Channel_ADC(ADC1,1);
						Start_ADC_Conversion(ADC1);
**/
void Single_Channel_ADC(ADC_TypeDef * ADC, char Voie_ADC);



/*=======================================================================================
    Fonctions de configuration Manuel / periodique
=======================================================================================*/

/**
 * @name   Init_Conversion_On_Trig_Timer(...) 
 * @Brief  Lance la conversion grâce à un timer
 * @note :  Le timer est lancé (en PWM 50% interne au uC)	
 * @param : ADC_TypeDef * ADC , ADC1 ou ADC2
 * @param : char Sources (cf #define) 
 * @param : float Freq_KHz , fréquence de déclenchement en kHz
 * @ret : _
 * @EXAMPLE Init_Conversion_On_Trig_Timer(ADC1,TIM1_CC1,20.0);

**/
void Init_Conversion_On_Trig_Timer(ADC_TypeDef * ADC , char Source, float Freq_KHz);
#define TIM1_CC1 0
#define TIM1_CC2 1
#define TIM1_CC3 2
#define TIM2_CC2 3
#define TIM4_CC4 5


/*=======================================================================================
 Fonctions de configuration en interruption
=======================================================================================*/
/**
 * @name   Init_IT_ADC_EOC
 * @Brief  Configure l'ADC de manière à rentrer en interruption lors de la fin de conversion.
 * @note : Le nom de la fonction est passé en paramètre ainsi que la
	         priorité d'interruption associée.
 * @param : ADC_TypeDef * ADC , ADC1 ou ADC2
 * @param : char Prio, la priorité d'IT 
 * @param : void (*IT_function) (void) , le nom du callback
 * @ret : _
 * @EXAMPLE Init_IT_ADC_EOC(ADC1, 1, IT_ADC);

**/
void Init_IT_ADC_EOC(ADC_TypeDef * ADC, char Prio, void (*IT_function) (void));


/**
 * @name   Init_IT_End_Of_DMA1
 * @Brief  Configure la DMA channel 1 de manière à rentrer en interruption lors 
           de la fin de transfert DMA
 * @note : Le nom de la fonction est passé en paramètre ainsi que 
						la priorité d'interruption associée.
           Uniquement pour ADC1   
 * @note : nécessite au préalable un appel à la fonction Init_TimingADC_ActiveADC
 * @param : char Prio, la priorité d'IT 
 * @param : void (*IT_function) (void) , le nom du callback
 * @ret : _
 * @EXAMPLE Init_IT_End_Of_DMA1(1, IT_DMA);
**/
void Init_IT_End_Of_DMA1(char Prio, void (*IT_function) (void));


/*=======================================================================================
 Fonctions de configuration en DMA
=======================================================================================*/

/**
 * @name    Init_ADC1_DMA1(...)
 * @Brief  Prépare l'ADC pour rentrer en DMA. La canal est DMA1. Il est associé à ADC1.
 * @note : La DMA transferre des mots 16 bits					
 * @note : nécessite au préalable un appel à la fonction Init_TimingADC_ActiveADC
 * @param : char Circ, une nouvelle DMA s'enchaîne à la fin de la précédente
 * @param : vu16 *Ptr_Table_DMA adresse de la table de réception des données
 * @ret : _
 * @EXAMPLE Init_IT_End_Of_DMA1(1, IT_DMA);
**/
void Init_ADC1_DMA1(char Circ, vu16 *Ptr_Table_DMA);



/*=======================================================================================
 Fonctions de configuration en mode DMA multiple channel en mode continu

 	=> quand on veut échantillonner plusieurs entrées sans passer par IT => suréchantillonnage.
 	=> uniquement ADC1
=======================================================================================*/
/**
 * @name   Init_ADC1_DMA_Mult_Chan(...)
 * @Brief  Fonctions de configuration en mode DMA multiple channel en mode continu
 * @note : uniquement ADC1			
 * @note : utilisé quand on veut échantillonner plusieurs entrées sans passer par IT
           => suréchantillonnage
 * @param : char Nb_Canaux
 * @param : char Seq_Canaux[] contient le numéros de divers channels qu'on veut sampler
 * @param : float Duree_Ech_us (en us)
 * @ret : _
 * @EXAMPLE 
				char Seq[4]={0,4,8,12};
				vu16 Val[4];
				volatile float duree;

				Set Up:
				duree=Init_ADC1_DMA_Mult_Chan(4, Seq, 40.0);
	
				Pgm utilisation en IT DMA par ex
				int i;
				for (i=0; i<4;i++)
				{
					Val[i] = ADC_Lire_DMA(i);
				}
**/
float Init_ADC1_DMA_Mult_Chan(char Nb_Canaux, char Seq_Canaux[], float Duree_Ech_us);

/**
 * @name    ADC_Lire_DMA(...)
 * @Brief  Récupère la valeur postionnée au rang Voie
 * @param : int Voie indice du tableau de DMA
 * @ret : _
 * @EXAMPLE Voir fct précédente;
**/
vu16 ADC_Lire_DMA(int Voie);



/*=======================================================================================
 Fonctions et Macros d'utilisation lancement manuel ADC , DMA
=======================================================================================*/
#define Start_ADC_Conversion(ADC) ADC->CR2=	(ADC->CR2)|0x01;
#define  Stop_DMA1 DMA1_Channel1->CCR =(DMA1_Channel1->CCR) &~0x1;

/**
 * @name   Start_DMA1
 * @Brief  Lance une DMA sur le nombre de points spécifié. La zone de RAM 
	écrite est indiquée  lors de l'appel de la fonction  Init_ADC1_DMA
 * @param :u16 NbEchDMA
 * @ret : _
**/
void Start_DMA1(u16 NbEchDMA);






/*=======================================================================================
 Fonctions d'utilisation scrutation
=======================================================================================*/

/**
 * @name    Wait_On_EOC_ADC(..;)
 * @Brief  Fonction bloquante  qui scrute  l'indicateur EOC de l'ADC				
 * @param : ADC_TypeDef * ADC ADC1 ou ADC2
 * @ret : _
**/
void Wait_On_EOC_ADC ( ADC_TypeDef * ADC);

/**
 * @name   Wait_On_End_Of_DMA1()
 * @Brief  Fonction bloquante  qui scrute  l'indicateur TCIF1 de la DMA			
 * @param : _
 * @ret : _
**/
void Wait_On_End_Of_DMA1(void);
 

/*=======================================================================================
 Fonctions et Macros d'utilisation lecture de donnée convertie
=======================================================================================*/
#define Read_ADC(ADC) (ADC->DR)


/*=======================================================================================
 Divers Define
=======================================================================================*/

/* alignement des data, quelque soit le mode*/
#define ALIGN_DROITE_16_0(ADC) ADC->CR2=ADC->CR2&~(ADC_CR2_ALIGN)
#define ALIGN_GAUCHE_1_15(ADC) ADC->CR2=ADC->CR2|(ADC_CR2_ALIGN)


/*=======================================================================================
 Exemples 
=======================================================================================*/


/* Test réel effectué, Trigger sur Timer, et DMA
Duree_Reelle=Init_TimingADC_ActiveADC(ADC1, 1.0 );  	config ADC de base
Single_Channel_ADC(ADC1, 1);					 		single chan, voie 1 (potentiometre)
Init_IT_ADC_EOC(ADC1, 1, IT_ADC);						ADC en IT, inutile mais bon
Init_Conversion_On_Trig_Timer(ADC1 , TIM2_CC2, 0.001);	Conversion en trig sur Timer, 1Hz

Init_ADC1_DMA1(20, 0, TableDMA);					   	DMA activée pour 20 eoc, non circulaire
Start_DMA1(20);											Demarrage DMA
Wait_On_End_Of_DMA1();									Polling fin DMA
pipo=Read_ADC(ADC1);
*/


/*
TEST DE DMA EN SIMULATION
Les lancement ADC sont faits sur IT T3.

void IT_T3 (void)
{
Start_ADC_Conversion(ADC1);
}

main
{
Duree_Reelle=Init_TimingADC_ActiveADC(ADC1, 1.0 );
Single_Channel_ADC(ADC1, Voie);
Timer_1234_Init(TIM3, 10.0);
Active_IT_Debordement_Timer(TIM3, 1, IT_T3);
Init_ADC1_DMA1(0, Ptr_Tab_DMA);
}
*/

#endif
