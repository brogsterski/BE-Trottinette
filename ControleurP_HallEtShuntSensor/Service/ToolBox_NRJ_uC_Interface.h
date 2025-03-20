
/******************************************************************************
* File Name          : ToolBox_NRJ_uC_Interface.c 
* Author             : T Rocacher
* Version            : V0
* d�pendance				 : Drivers_STM32F103_107_2024.lib et .h associ�s
* Date release			 : 08/11/2024
* Description        : Driver (niveau service) de l'ensemble uC + interface :Hacheur 4Q
											 NB d�riv� de Toolbox NRJ Nucleo
* Processeur				 : STM32F103, 
* Outil Developpement: KEIL 5.40.0.3
* Documentation			 : _
********************************************************************************/




//*********************************************************************************************************
//
//					TOOLBOX_NRJ Services soft embarqu� pour le PCB �tudiant 2024
//
//*********************************************************************************************************
	 

/* Configuration logicielle */
/*
- MODIFIER startup_stm32f10x_md.s : commenter les lignes 135 et 136
                 ;LDR     R0, =SystemInit
                 ;BLX     R0

choisir HSE, HSI ou Bypass selon la configuration hard :
Clock_F103_ConfMainClk64MHz(...);
ou encore
Clock_F103_ConfMainClk72MHzHSE() 
NB : quelle que soit la fonction, HSE uniquement si quartz OK. Sinon, HSI !

*/



/*=========================================================================================================
		
				ENTREE ET SORTIE PAR GRANDES FONCTIONS :
				
===========================================================================================================

_____________________________________________________________________________________
ENTREES Anaogique:
					Potentiom�tre 		 : PA3 - Voie ADC 3
					Entr�e I filtr�		 : PA1 - Voie ADC 1
					Entr�e RefI 1,65V	 : PA2 - Voie ADC 2

_____________________________________________________________________________________
LA PWM
					TIM3_Ch1 : PA6
					TIM3_Ch2 : PA7
______________________________________________________________________________________

DETAILS PWM
 
-la fonction UpRamp (F_PWM_KHZ_Val)
op�re en PWM edge aligned : P_PWM = CycleTimer = 1/F_PWM_KHZ_Val)
- la fonction Triangle (F_PWM_KHZ_Val)
op�re en PWM center aligned : P_PWM = 2CycleTimer = 2*1/2.F_PWM_KHZ_Val)

TRIANGLE
 
    *     *     *     *
   * *   * *   * *   * *
  *   * *   * *   * *   * 
 *     *     *     *     *    
 |P_PWM|P_PWM|P_PWM|P_PWM| 
 

UpRamp

       *     *     *     * 
      **    **    **    **
     * *   * *   *  *  * *
    *  *  *  *  *  *  *  *
   *   * *   * *   * *   * 
  *    **    **    **    *
 *     *     *     *     *  
 |P_PWM|P_PWM|P_PWM|P_PWM|

 La r�solution est deux fois moins grande en triangle qu'en UpRamp.
 
 #define VirtualReso_UpRamp 1000 est la r�solution que le user utilise mais
 n'est pas la r�solution r�elle
 
 #define  VirtualReso_Triangle (VirtualReso_UpRamp/2), m�me chose
 
 R�solution r�elle possible atteignable par le timer :
 20kHz UpRamp / 72MHz -> Reso 3600pts
 50kHz UpRamp / 72MHz -> Reso 1440pts
 72kHz UpRamp / 72MHz -> Reso 1000pts
 20kHz Triangle / 72MHz -> Reso 1800pts
 50kHz Triangle / 72MHz -> Reso 720pts
 72kHz Triangle / 72MHz -> Reso 500pts

 20kHz UpRamp / 64MHz -> Reso 3200pts
 50kHz UpRamp / 64MHz -> Reso 1280pts
 64kHz UpRamp / 64MHz -> Reso 1000pts
 20kHz Triangle / 64MHz -> Reso 1600pts
 50kHz Triangle / 64MHz -> Reso 640pts
 64kHz Triangle / 64MHz -> Reso 500pts
 
 Dans tous ces cas, la r�solution "virtuelle" est garantie, c'est � dire que l'on 
 sacrifie des possibilit�s du timer. Ce sera 1000pts en UpRamp et 500 en triangle.
 
 NB: Le user croit que la r�so est toujours de 1000, UpRamp ou Triangle. Cela signifie
 qu'il est tromp� sur le triangle et sera limit� � 500. C'est un choix.
 Dit autrement, en UpRamp on est pr�cis � 0.1% (1000pts), en Triangle, on reste sur
 une r�solution user de 1000pts, mais en r�alit� on est � 0.2%. 
 
 Exemple : R_Cyc_1(500) , R_Cyc_1(501) et R_Cyc_1(502)
 donne respectivement 50.0% , 50.1% et 50.2% en mode UpRamp
 donne respectivement 50.0% , 50.0% et 50.2% en mode Triangle (erreur 0.1% pour 501)
 
 NB : lors de l'init, un test est fait pour comparer la r�solution r�elle et la r�solution virtuelle
 Si la r�elle est moins bonne que la virtuelle, plantage while (1).
 
 NB : le syst�me bloque le temps haut � 1�s mini et T-1�s maxi et ce automatiquement quelque soit
 la fr�quence de timer utilis�. Un rapport cy de 0% ou 100% est donc impossible (protection
 MOS HighSide).
 
*/

#ifndef _TOOLBOX_H__
#define _TOOLBOX_H__
#include "stm32f10x.h"
#include "GPIO.h"
#include "main.h"

/*===================================================================
					USER Define
=====================================================================*/
// Choisir la priorit� d'interruption de 0 � 15, 0 le plus prioritaire.
 #define Prio_IT_Tim 1

/*===================================================================*/


/*===================================================================
					INITIALISATION GENERALE DES IO
=====================================================================*/
/**
 * @name   Conf_Generale_IO_Carte()
 * @Brief  La fonction permet de :
					- configurer les entr�es analogiques
					- configurer la LED en sortie
					- configurer la PWM
 * @param _
 * @ret : _
 * @EXAMPLE�Conf_Generale_IO_Carte();
**/
void Conf_Generale_IO_Carte (void);


/*===================================================================
					LED
=====================================================================*/
#define LED_User_On 							GPIO_Set(LED_User_GPIO,LED_User_Pin) 
#define LED_User_Off 							GPIO_Clear(LED_User_GPIO,LED_User_Pin) 


/*===================================================================
					INTERRUPTIONS
			
=====================================================================*/
/**
 * @name    Conf_IT_Principale_TIM(...)
 * @Brief  La fonction permet de g�n�rer une interruption. Le nom du Callback est
						pass� en param�tre
 * @param : TIM_TypeDef * Tim, Tim = Tim1 � TIM4. Exclure TIM3 car utilis� en PWM
 * @ret : _
 * @EXAMPLE� Conf_IT_Principale_TIM(TIM2,CallbackName);
**/
void Conf_IT_Principale_TIM(TIM_TypeDef * Tim, void (*IT_function) (void),float Periode_IT_us );


/*===================================================================
					ENTREE ANALOGIQUE
=====================================================================*/
/**
 * @name    Conf_ADC()
 * @Brief   Configure l'ADC avec le Sampling Time au minimum (r�sistance
						d'attaque doit �tre de l'ordre du kOhm ou condensateur suffisant sur
						la pin de l'ADC)
 * @param : _
 * @ret : _
 * @EXAMPLE� Conf_ADC();
**/
void Conf_ADC(void);

/**
 * @name    Read_Cons(), Read_I(), Read_Ref()
 * @Brief   Lance l'ADC sur l'une des 3 voies, attend la fin de conversion, renvoie la
 valeur num�ris�e [0 : 3.3V] -> [0 : 4095]
 Voies :
 					Potentiom�tre 		 : PA3 - Voie ADC 3
					Entr�e I filtr�		 : PA1 - Voie ADC 1
					Entr�e RefI 1,65V	 : PA2 - Voie ADC 2
 * @param : _
 * @ret : _
 * @EXAMPLE� Read_Cons();
**/
int Read_Cons(void); 
int Read_I(void);
int Read_Ref(void);



/*===================================================================
		PWM  !! Rapport cyclique de 0 � 1000  <=> 0 � 100% !!
=====================================================================*/


/**
 * @name    Triangle(...) 
 * @Brief   G�n�re la r�f�rence de la PWM, forme triangulaire
 * @Note  	� l'issue de la configuration, les 2 PWM sont � 50% mais 
						les deux canaux ne sont pas valid�s (signaux ne sortent pas).
 * @param : float F_PWM_KHZ_Val, valeur de fr�quence voulue en kHz
 * @ret : _
 * @EXAMPLE� Triangle(20.0); // PWM � 20kHz
**/
void Triangle ( float F_PWM_KHZ_Val);

/**
 * @name    UpRamp(...) 
 * @Brief   G�n�re la r�f�rence de la PWM, forme dent de scie
 * @Note  	� l'issue de la configuration, les 2 PWM sont � 50% mais 
						les deux canaux ne sont pas valid�s (signaux ne sortent pas).
 * @param : float F_PWM_KHZ_Val, valeur de fr�quence voulue en kHz
 * @ret : _
 * @EXAMPLE� UpRamp(20.0); // PWM � 20kHz
**/
void UpRamp ( float F_PWM_KHZ_Val);



/**
 * @name    Active_Voie_PWM(...) / Desactive_Voie_PWM
 * @Brief   Valide / invalide la voie, cad, permet / emp�che le signal de sortir du Timer
 * @param : char Voie, de 1 � 4
 * @ret : _
 * @EXAMPLE� Active_Voie_PWM(1);  // active la voie 1
**/
void Active_Voie_PWM(char Voie); // Voie de 1 � 3.
void Desactive_Voie_PWM(char Voie);

/**
 * @name    Inv_Voie(...) 
 * @Brief   Inverse la polarit� du signal PWM 
 * @param : char Voie, de 1 � 4
 * @ret : _
 * @EXAMPLE� Inv_Voie(1); // Inverse la voie 1
**/
void Inv_Voie(char Voie); // permet de donner une logique inverse � la voie PWM (utile pour un pont complet)




/**
 * @name    R_Cyc_1 (...),  R_Cyc_2 (...)
 * @Brief   Met � jour le rapport cyclique (en synchronisme avec la r�f�rence)
						de la voie 1 et 2. 
 * @param : int Val compris entre 0 et 1000
 [0 : 1000] -> [0% : 100%]
 * @note   : 0% et 100% ne sont en r�alit� pas atteingnables. Une limite est pr�vue
						limite basse Th=1�s, limite haute Th=T-1�s
 * @ret : _
 * @EXAMPLE�  R_Cyc_1(325); // impose un rapport cyclique de 32.5%
**/
void R_Cyc_1(int Val);
void R_Cyc_2(int Val);




#endif
