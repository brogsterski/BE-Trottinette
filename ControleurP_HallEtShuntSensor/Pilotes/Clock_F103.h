#ifndef _Clock_F103_H__
#define _Clock_F103_H__
#include "stm32f10x.h"

typedef enum {
	Clock_HSE_Quartz,
	Clock_HSE_Bypass,
	Clock_HSI,
}Clock_CkSrc;


/**
  * @brief  R�gle le �C avec une Fr�q CPU de 64MHz de 3 mani�res diff�rentes
	* - Clock_HSE_Quartz : on utilise le montage � quartz avec les 2 capacit�s
	* - Clock_HSE_Bypass : on utilise un signal externe (suppos� � 64MHz)
	* - Clock_HSI : on utilise le RC interne � 8 MHz environ
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
	 
  * @param  Clock_CkSrc CkSrc, un des 3 param�tres �num
  * @retval _
	* 
  **/

void Clock_F103_ConfMainClk64MHz(Clock_CkSrc CkSrc);


/**
  * @brief  R�gle le �C avec une Fr�q CPU de 72MHz en HSE Quartz
           C'est le r�glage hyper classique Nucl�o par exemple
	* @Note 	La conf hard de la Nucl�o est celle d'un Bypass (pas
						de quartz et capa connect�es). Mais le mode HSE fonctionne
						tr�s bien puisqu'aucun composant n'est soud�. Il n'y a pas
						de conflit sur la sortie OscOut.
  * @param  _
  * @retval -
  **/
void Clock_F103_ConfMainClk72MHzHSE(void);


/**
  * @brief  Fonction qui renvoie la fr�quence CPU
  * @Note   
  * @param  
  * @retval int : la valeur de la fr�quence (Hz)
	* @Note  renvoie -1, -2 ... si �chec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getSYSCLK(void);


/**
  * @brief  Fonction qui renvoie la fr�quence Timer
  * @Note   
  * @param  
  * @retval int : la valeur de la fr�quence (Hz)
	* @Note  renvoie -1, -2 ... si �chec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getTimerCLK(TIM_TypeDef *Timer);

/**
  * @brief  Fonction qui renvoie la fr�quence ADC
  * @Note   
  * @param  _
  * @retval int : la valeur de la fr�quence (Hz)
	* @Note  renvoie -1, -2 ... si �chec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getADC_CLK(void);


/**
  * @brief  Fonction qui renvoie la fr�quence HCLK pour Systick
  * @Note   
  * @param  _
  * @retval int : la valeur de la fr�quence (Hz)
	* @Note  renvoie -1, -2 ... si �chec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getHCLK(void);

#endif
