#include "stm32f10x.h"
#include "Clock_F103.h"
/* =================================================================================
* ==================    Clock_F103.c	     =================================
 *
 *   Created on: 06/11/24
 *   Author: T.Rocacher
 *   Tool : KEIL V5.40.0.3
 *   Target : STM32F103RTB6
 *   Dépendance : _
 *   Revision : V0.00
 *  ------------------------------------------------------------------------------
 *  Initialise la Clk HSI, HSE ou ByPass
 *  !! utilisé avec les cartes Etudiants -> DECOMMENTEZ DS STARTUP.s L'APPEL A :
                  ;LDR     R0, =SystemInit
                  ;BLX     R0
 * Renvoie les fréquences en Hz au niveau des principale sortie RCC
 *  un define indique la valeur du quartz externe, par défaut 8 000 000
* =================================================================================*/



void Clock_F103_ConfPrivate(uint32_t PLLMul,Clock_CkSrc CkSrc);



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
   _|_   Q  _|_                _ | _
   _ _ C1   _ _ C2           _| |_| |_ signal externe
	  |        |
	 _|_      _|_
	 ///	    ///
	 
  * @param  Clock_CkSrc CkSrc, un des 3 paramètres énum
  * @retval _
	* 
  **/


void Clock_F103_ConfMainClk64MHz(Clock_CkSrc CkSrc)
{
	Clock_F103_ConfPrivate(RCC_CFGR_PLLMULL8,CkSrc);
}


/**
  * @brief  Règle le µC avec une Fréq CPU de 72MHz en HSE 
  * @Note   C'est le réglage hyper classique Nucléo par exemple
  * @param  _
  * @retval -
  **/
void Clock_F103_ConfMainClk72MHzHSE(void)
{
	Clock_F103_ConfPrivate(RCC_CFGR_PLLMULL9,Clock_HSE_Quartz);
}



#define HSE_Hz 8000000
#define HSI_Hz 8000000
/**
  * @brief  Fonction qui renvoie la fréquence CPU
  * @Note   
  * @param  
  * @retval int : la valeur de la fréquence (Hz)
	* @Note  renvoie -1, -2 ... si échec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getSYSCLK(void)
{

	int Freq;
	int Mul;
	Freq=-1;
	if ((RCC->CFGR&RCC_CFGR_SWS)==RCC_CFGR_SWS_PLL) /* on est sur la PLL*/
	{
		/* calcul coeff multiplication PLL*/
		Mul=((RCC->CFGR&RCC_CFGR_PLLMULL)>>18)+2; /* max = 15+2 = 17 à saturer à 16*/
		if (Mul==17) Mul=16;
		
		/* Chemin HSI ... */
		if ((RCC->CR&RCC_CR_HSION)==RCC_CR_HSION) 
		{
			if ((RCC->CFGR&RCC_CFGR_PLLSRC)==0) /* confirmation Mux3 sur HSI*/
			{
				Freq=HSI_Hz*Mul/2; /* fréquence en entrée Mux3*/
			}
			else Freq=-2;
		}
		/* Chemin HSE ... */
		else if ((RCC->CR&RCC_CR_HSEON)==RCC_CR_HSEON) 
		{
			if ((RCC->CFGR&RCC_CFGR_PLLSRC)==RCC_CFGR_PLLSRC) /* confirmation Mux3 sur HSE*/
			{
				if ((RCC->CFGR&RCC_CFGR_PLLXTPRE)==0) /* pas de prédiv de HSE*/
				{
					Freq=HSE_Hz*Mul; /* fréquence en entrée Mux4*/
				}
				else Freq=HSE_Hz*Mul/2; /* prédiv de HSE*/
			}
			else Freq=-3;
		}
		else Freq=-4;
		
	}
	else Freq=-1;
	return Freq;
}

/**
  * @brief  Fonction qui renvoie la fréquence Timer
  * @Note   
  * @param  
  * @retval int : la valeur de la fréquence (Hz)
	* @Note  renvoie -1, -2 ... si échec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getTimerCLK(TIM_TypeDef *Timer)
{
	int Freq;
	int Div;

	Freq=Clock_F103_getSYSCLK();
	if (Freq>0) /* pas d'erreur*/
	{
		/* sortie de AHB*/
		Div = 1;
		if ((RCC->CFGR&RCC_CFGR_HPRE)>=8) /* diviseur autre que 1*/ 
		{
			Div = 1<<((RCC->CFGR&RCC_CFGR_HPRE)-7); /* ex HPRE = 11, 1<<11-7 = 1<<4 = /16*/
			if (Div>16) Div=Div<<1; /* calcul pas bon, il faut multiplier par 2 (manque /32 ds les réglages possibles...) 
														ex HPRE = 12, Div = 1<<12-7 = 1<<5 = /32 -> Div = 32*2 = 64  */
		}
		Freq=Freq/Div; /* en sortie AHB*/
		if (Timer!=TIM1) /*TIMXCLK - APB1 */
		{
			/* sortie de APB1*/
			Div = 1;
			if ((RCC->CFGR& RCC_CFGR_PPRE1 )>=4) /* diviseur autre que 1*/ 
			{
				Div = 1<<((RCC->CFGR&RCC_CFGR_PPRE1)-3); /* ex PPRE1 = 7, 1<<7-3 = 1<<4 = /16*/
				Div = Div/2; /* si le diviseur est autre que 1, on multiplie par 2...*/
			}
			Freq=Freq/Div;	/*en sortie APB1*/		
		}
		else /*TIMxCLK - APB2, TIM2,3,4 */
		{
		/* sortie de APB2*/
			Div = 1;
			if ((RCC->CFGR& RCC_CFGR_PPRE2 )>=4) /* diviseur autre que 1*/ 
			{
				Div = 1<<((RCC->CFGR&RCC_CFGR_PPRE2)-3); /* ex PPRE2 = 7, 1<<7-3 = 1<<4 = /16*/
				Div = Div/2; /* si le diviseur est autre que 1, on multiplie par 2...*/
			}
			Freq=Freq/Div;	/*en sortie APB2*/	
		}	
	}
	
	return Freq;
	
}


/**
  * @brief  Fonction qui renvoie la fréquence ADC
  * @Note   
  * @param  _
  * @retval int : la valeur de la fréquence (Hz)
	* @Note  renvoie -1, -2 ... si échec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getADC_CLK(void)
{
	int Freq;
	int Div;

	Freq=Clock_F103_getSYSCLK();
	if (Freq>0) /* pas d'erreur*/
	{
		/* sortie de AHB*/
		Div = 1;
		if ((RCC->CFGR&RCC_CFGR_HPRE)>=8) /* diviseur autre que 1*/ 
		{
			Div = 1<<((RCC->CFGR&RCC_CFGR_HPRE)-7); /* ex HPRE = 11, 1<<11-7 = 1<<4 = /16*/
			if (Div>16) Div=Div<<1; /* calcul pas bon, il faut multiplier par 2 (manque /32 ds les réglages possibles...) 
														ex HPRE = 12, Div = 1<<12-7 = 1<<5 = /32 -> Div = 32*2 = 64  */
		}
		Freq=Freq/Div; /* en sortie AHB*/

		/* sortie de APB2*/
		Div = 1;
		if ((RCC->CFGR& RCC_CFGR_PPRE2 )>=4) /* diviseur autre que 1*/ 
		{
			Div = 1<<((RCC->CFGR&RCC_CFGR_PPRE2)-3); /* ex PPRE2 = 7, 1<<7-3 = 1<<4 = /16*/
		}
		Freq=Freq/Div;	/*en sortie APB2*/	
		
		/*Feq ADC*/
		if ((RCC->CFGR&RCC_CFGR_ADCPRE)==RCC_CFGR_ADCPRE_DIV2) Freq=Freq/2;
		else if ((RCC->CFGR&RCC_CFGR_ADCPRE)==RCC_CFGR_ADCPRE_DIV4) Freq=Freq/4;
		else if ((RCC->CFGR&RCC_CFGR_ADCPRE)==RCC_CFGR_ADCPRE_DIV6) Freq=Freq/6;
		else Freq=Freq/8;
	}
	
	return Freq;
	
}

/**
  * @brief  Fonction qui renvoie la fréquence HCLK pour Systick
  * @Note   
  * @param  _
  * @retval int : la valeur de la fréquence (Hz)
	* @Note  renvoie -1, -2 ... si échec (par ex Mux2 n'est pas sur la PLL)
  **/
int Clock_F103_getHCLK(void)
{
	int Freq;
	int Div;

	Freq=Clock_F103_getSYSCLK();
	if (Freq>0) /* pas d'erreur*/
	{
		/* sortie de AHB*/
		Div = 1;
		if ((RCC->CFGR&RCC_CFGR_HPRE)>=8) /* diviseur autre que 1*/ 
		{
			Div = 1<<((RCC->CFGR&RCC_CFGR_HPRE)-7); /* ex HPRE = 11, 1<<11-7 = 1<<4 = /16*/
			if (Div>16) Div=Div<<1; /* calcul pas bon, il faut multiplier par 2 (manque /32 ds les réglages possibles...) 
														ex HPRE = 12, Div = 1<<12-7 = 1<<5 = /32 -> Div = 32*2 = 64  */
		}
		Freq=Freq/Div; /* en sortie AHB donc HCLK*/
	}
	return Freq;
	
}	


/* =================================================================================
 ==================    PRIVATE FCTS 	     =================================
 =================================================================================*/

void Clock_F103_ConfPrivate(uint32_t PLLMul,Clock_CkSrc CkSrc)
{
	/* Reset the RCC clock configuration to the default reset state(for debug purpose) */
  /* Set HSION bit */
	RCC->CR |= (uint32_t)0x00000001;
	/* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
	RCC->CFGR &= (uint32_t)0xF8FF0000;
	/* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;
  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;
	/* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
  RCC->CFGR &= (uint32_t)0xFF80FFFF;
	/* Disable all interrupts and clear pending bits  */
  RCC->CIR = 0x009F0000;
	
	
	/* Enable Prefetch Buffer */
	FLASH->ACR |= FLASH_ACR_PRFTBE;
	/* Flash 2 wait state */
	FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
  FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2; 
	
	switch(CkSrc)
	{
		case Clock_HSE_Quartz:
		{

			RCC->CR|=RCC_CR_HSEON;
	   	while((RCC->CR & RCC_CR_HSERDY)==0);
			
			RCC->CFGR|=RCC_CFGR_PLLSRC; // HSE en entrée de PLL
			RCC->CFGR&=~RCC_CFGR_PLLMULL; // PLL x8 pour obtenir 64MHz FCPU
			RCC->CFGR|=PLLMul; 
			
			RCC->CR|=RCC_CR_PLLON;			// PLL on
			while((RCC->CR & RCC_CR_PLLRDY )==0);
			
			RCC->CFGR&=~RCC_CFGR_SW;  // Choix de la sortie PLL (64MHz) pour sysclk
			RCC->CFGR|=RCC_CFGR_SW_PLL;
			while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08); // attente que SysClk soit bien drivé par la PLL    
		
			RCC->CR&=~RCC_CR_HSION; // Neutralisation HSI faisable que si HSE tourne
			break;
		}
		
		case Clock_HSE_Bypass:
		{
			
			RCC->CR|=RCC_CR_HSEBYP; // Bypass : doit être positionné avant de valider HSEON
			
			RCC->CR|=RCC_CR_HSEON;
	   	while((RCC->CR & RCC_CR_HSERDY)==0);
			
			RCC->CFGR|=RCC_CFGR_PLLSRC; // HSE en entrée de PLL
			RCC->CFGR&=~RCC_CFGR_PLLMULL; // PLL x8 pour obtenir 64MHz FCPU
			RCC->CFGR|=PLLMul; 
			
			RCC->CR|=RCC_CR_PLLON;			// PLL on
			while((RCC->CR & RCC_CR_PLLRDY )==0);
			
			RCC->CFGR&=~RCC_CFGR_SW;  // Choix de la sortie PLL (64MHz) pour sysclk
			RCC->CFGR|=RCC_CFGR_SW_PLL;
			while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08); // attente que SysClk soit bien drivé par la PLL    
		
			RCC->CR&=~RCC_CR_HSION; // Neutralisation HSI faisable que si HSE tourne
			break;
			
		}
		case Clock_HSI:
		{
			RCC->CFGR&=~RCC_CFGR_PLLSRC; // HSI/2 en entrée de PLL
			RCC->CFGR&=~RCC_CFGR_PLLMULL; // PLL x16 pour obtenir 64MHz FCPU
			RCC->CFGR|=RCC_CFGR_PLLMULL16; 

			RCC->CR&=~RCC_CR_HSEON; // Neutralisation HSE
			RCC->CR|=RCC_CR_HSION;
			while((RCC->CR & RCC_CR_HSIRDY)==0);
			RCC->CR|=RCC_CR_PLLON;			// PLL on
			while((RCC->CR & RCC_CR_PLLRDY )==0);
			
			RCC->CFGR&=~RCC_CFGR_SW;  // Choix de la sortie PLL (64MHz) pour sysclk
			RCC->CFGR|=RCC_CFGR_SW_PLL;
			while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08); // attente que SysClk soit bien drivé par la PLL      
	
			break;
		}
		default: while(1); // ERREUR !
	}
	// conf ADC avec /6 (12M à 72MHz, et 10.66 à 64MHz)
	RCC->CFGR&=~RCC_CFGR_ADCPRE;
	RCC->CFGR|=RCC_CFGR_ADCPRE_DIV6;
}


