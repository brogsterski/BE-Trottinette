#ifndef _Clock_F103_H__
#define _Clock_F103_H__
#include "stm32f10x.h"

typedef enum {
	Clock_HSE_Quartz,
	Clock_HSE_Bypass,
	Clock_HSI,
}Clock_CkSrc;


/**
  * @brief  Règle le µC avec une Fréq CPU de 64MHz de 3 manières différentes
	* - Clock_HSE_Quartz : on utilise le montage à quartz avec les 2 capacités
	* - Clock_HSE_Bypass : on utilise un signal externe (supposé à 64MHz)
	* - Clock_HSI : on utilise le RC interne à 8 MHz environ
  * @Note  
Clock_HSE_Quartz                Clock_HSE_Bypass 

       |\                           |\   
	 ----| >o----                 ----| >o----
    |  |/    |                   |  |/    
    |        |                   |        
	 _|osc_in _| osc_out          _|osc_in _ osc_out
--|_|------|_|-------       ---|_|------|_|---------- 
    |   __   |                   |
    |_||  ||_|                   |
    | ||__|| |                   ^
   _|_   Q  _|_                _   _
   _ _ C1   _ _ C2           _| |_| |_ signal externe
	  |        |
	 _|_      _|_
	 ///	    ///
	 
  * @param  Clock_CkSrc CkSrc, un des 3 paramètres énum
  * @retval _
	* 
  **/

void Clock_F103_ConfMainClk64MHz(Clock_CkSrc CkSrc);


/**
  * @brief  Règle le µC avec une Fréq CPU de 72MHz en HSE Quartz
           C'est le réglage hyper classique Nucléo par exemple
	* @Note 	La conf hard de la Nucléo est celle d'un Bypass (pas
						de quartz et capa connectées). Mais le mode HSE fonctionne
						très bien puisqu'aucun composant n'est soudé. Il n'y a pas
						de conflit sur la sortie OscOut.
  * @param  _
  * @retval -
  **/
void Clock_F103_ConfMainClk72MHzHSE(void);


/**
  * @brief  Fonction qui renvoie la fréquence CPU
  * @Note   
  * @param  
  * @retval int : la valeur de la fréquence (Hz)
	* @Note  renvoie -1, -2 ... si échec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getSYSCLK(void);


/**
  * @brief  Fonction qui renvoie la fréquence Timer
  * @Note   
  * @param  
  * @retval int : la valeur de la fréquence (Hz)
	* @Note  renvoie -1, -2 ... si échec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getTimerCLK(TIM_TypeDef *Timer);

/**
  * @brief  Fonction qui renvoie la fréquence ADC
  * @Note   
  * @param  _
  * @retval int : la valeur de la fréquence (Hz)
	* @Note  renvoie -1, -2 ... si échec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getADC_CLK(void);


/**
  * @brief  Fonction qui renvoie la fréquence HCLK pour Systick
  * @Note   
  * @param  _
  * @retval int : la valeur de la fréquence (Hz)
	* @Note  renvoie -1, -2 ... si échec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getHCLK(void);

#endif
