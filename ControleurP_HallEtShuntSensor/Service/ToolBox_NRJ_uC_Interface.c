
/******************************************************************************
* File Name          : ToolBox_NRJ_uC_Interface.c 
* Author             : T Rocacher
* Version            : V0
* dépendance				 : Drivers_STM32F103_107_2024.lib et .h associés
* Date release			 : 08/11/2024
* Description        : Driver (niveau service) de l'ensemble uC + interface :Hacheur 4Q
											 NB dérivé de Toolbox NRJ Nucleo
* Processeur				 : STM32F103, 
* Outil Developpement: KEIL 5.40.0.3
* Documentation			 : _
********************************************************************************/



//*********************************************************************************************************
//
//					TOOLBOX_NRJ Services soft embarqué pour le PCB étudiant 2024
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
					Potentiomètre 		 : PA3 - Voie ADC 3
					Entrée I filtré		 : PA1 - Voie ADC 1
					Entrée RefI 1,65V	 : PA2 - Voie ADC 2

_____________________________________________________________________________________
LA PWM
					TIM3_Ch1 : PA6
					TIM3_Ch2 : PA7
______________________________________________________________________________________

DETAILS PWM
 
-la fonction UpRamp (F_PWM_KHZ_Val)
opère en PWM edge aligned : P_PWM = CycleTimer = 1/F_PWM_KHZ_Val)
- la fonction Triangle (F_PWM_KHZ_Val)
opère en PWM center aligned : P_PWM = 2CycleTimer = 2*1/2.F_PWM_KHZ_Val)

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

 La résolution est deux fois moins grande en triangle qu'en UpRamp.
 
 #define VirtualReso_UpRamp 1000 est la résolution que le user utilise mais
 n'est pas la résolution réelle
 
 #define  VirtualReso_Triangle (VirtualReso_UpRamp/2), même chose
 
 Résolution réelle possible atteignable par le timer :
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
 
 Dans tous ces cas, la résolution "virtuelle" est garantie, c'est à dire que l'on 
 sacrifie des possibilités du timer. Ce sera 1000pts en UpRamp et 500 en triangle.
 
 NB: Le user croit que la réso est toujours de 1000, UpRamp ou Triangle. Cela signifie
 qu'il est trompé sur le triangle et sera limité à 500. C'est un choix.
 Dit autrement, en UpRamp on est précis à 0.1% (1000pts), en Triangle, on reste sur
 une résolution user de 1000pts, mais en réalité on est à 0.2%. 
 
 Exemple : R_Cyc_1(500) , R_Cyc_1(501) et R_Cyc_1(502)
 donne respectivement 50.0% , 50.1% et 50.2% en mode UpRamp
 donne respectivement 50.0% , 50.0% et 50.2% en mode Triangle (erreur 0.1% pour 501)
 
 NB : lors de l'init, un test est fait pour comparer la résolution réelle et la résolution virtuelle
 Si la réelle est moins bonne que la virtuelle, plantage while (1).
 
 NB : le système bloque le temps haut à 1µs mini et T-1µs maxi et ce automatiquement quelque soit
 la fréquence de timer utilisé. Un rapport cy de 0% ou 100% est donc impossible (protection
 MOS HighSide).
 
*/

#include "Timer_1234.h"
#include "GPIO.h"
#include "NVIC_IT_Ext.h"
#include "ADC_DMA.h"
#include "ToolBox_NRJ_uC_Interface.h"
#include "Clock_F103.h"

vu16 Resolution;
vu16 Resolution_1;
vu16 Resolution_2_3;
static int Delta_PWM; // c'est le pas min en PWM (éviter claquage High Side)

volatile int Delta_UpRamp=72;// 72 par défaut
volatile int Delta_Triangle=36 ;//(Delta_UpRamp/2)

static int VirtualReso=1000;
#define VirtualReso_UpRamp 1000  // Ne pas toucher !!! 
#define VirtualReso_Triangle (VirtualReso_UpRamp/2) // Ne pas toucher !!! 
/*===================================================================
					INITIALISATION GENERALE DES IO
=====================================================================*/

/**
 * @name   Conf_Generale_IO_Carte()
 * @Brief  La fonction permet de :
					- configurer les entrées analogiques
					- configurer la LED en sortie
					- configurer la PWM
 * @param _
 * @ret : _
 * @EXAMPLE Conf_Generale_IO_Carte();
**/
void Conf_Generale_IO_Carte(void)
{
// *********** ENTREES ANALOG *******
GPIO_Configure(U_IF_GPIO, U_IF_Pin, INPUT, ANALOG);
GPIO_Configure(MidRef_GPIO, MidRef_Pin, INPUT, ANALOG);
GPIO_Configure(Pot_GPIO, Pot_Pin, INPUT, ANALOG);
// *********** LED VISU *******	
GPIO_Configure(LED_User_GPIO, LED_User_Pin, OUTPUT, OUTPUT_PPULL);
GPIO_Clear(LED_User_GPIO,LED_User_Pin);
// *********** SORTIES PWMS *******	
GPIO_Configure(PWM1_GPIO, PWM1_Pin, OUTPUT, ALT_PPULL );
GPIO_Configure(PWM2_GPIO, PWM2_Pin, OUTPUT, ALT_PPULL );
}

/*===================================================================
					INTERRUPTION
=====================================================================*/
/**
 * @name    Conf_IT_Principale_TIM(...)
 * @Brief  La fonction permet de générer une interruption. Le nom du Callback est
						passé en paramètre
 * @param : TIM_TypeDef * Tim, Tim = Tim1 à TIM4. Exclure TIM3 car utilisé en PWM
 * @ret : _
 * @EXAMPLE  Conf_IT_Principale_TIM(TIM2,CallbackName);
**/
void Conf_IT_Principale_TIM(TIM_TypeDef * Tim, void (*IT_function) (void),float Periode_IT_us )
{
	Timer_1234_Init(Tim,Periode_IT_us);
	Active_IT_Debordement_Timer(Tim,Prio_IT_Tim,IT_function);
	Run_Timer(Tim);
}



/*===================================================================
					ADC
=====================================================================*/
/**
 * @name    Conf_ADC()
 * @Brief   Configure l'ADC avec le Sampling Time au minimum (résistance
						d'attaque doit être de l'ordre du kOhm ou condensateur suffisant sur
						la pin de l'ADC)
 * @param : _
 * @ret : _
 * @EXAMPLE  Conf_ADC();
**/
void Conf_ADC(void)
{
	Init_TimingADC_ActiveADC( ADC1, 0.0);
	Single_Channel_ADC(ADC1, 11);
}



int ToolBoxADC_Read(int channel);

/**
 * @name    Read_Cons(), Read_I(), Read_Ref()
 * @Brief   Lance l'ADC sur l'une des 3 voies, attend la fin de conversion, renvoie la
 valeur numérisée [0 : 3.3V] -> [0 : 4095]
 Voies :
 					Potentiomètre 		 : PA3 - Voie ADC 3
					Entrée I filtré		 : PA1 - Voie ADC 1
					Entrée RefI 1,65V	 : PA2 - Voie ADC 2
 * @param : _
 * @ret : _
 * @EXAMPLE  Read_Cons();
**/
int Read_Cons(void)
{
	return ToolBoxADC_Read(3);
}
int Read_I(void)
{
	return ToolBoxADC_Read(1);
}
int Read_Ref(void)
{
	return ToolBoxADC_Read(2);
}

int ToolBoxADC_Read(int channel)
{
	// Specif de la voie ADC
	Single_Channel_ADC(ADC1, channel);
	Start_ADC_Conversion(ADC1);
	Wait_On_EOC_ADC(ADC1); 
	// tant que EOC=0 rien
	return (Read_ADC(ADC1));
}




/*===================================================================
			V5 !		PWM  !! Rapport cyclique de 0 à 1000  !!
=====================================================================*/
/* 
-la fonction UpRamp (F_PWM_KHZ_Val)
opère en PWM edge aligned : P_PWM = CycleTimer = 1/F_PWM_KHZ_Val)
- la fonction Triangle (F_PWM_KHZ_Val)
opère en PWM center aligned : P_PWM = 2CycleTimer = 2*1/2.F_PWM_KHZ_Val)
car il faut deux cycles pour faire une période PWM
On a donc ici CycleTimer = 1/2.F_PWM_KHZ_Val


 
    *     *     *     *
   * *   * *   * *   * *
  *   * *   * *   * *   * 
 *     *     *     *     *    
 |P_PWM|P_PWM|P_PWM|P_PWM| 
 
       *     *     *     * 
      **    **    **    **
     * *   * *   *  *  * *
    *  *  *  *  *  *  *  *
   *   * *   * *   * *   * 
  *    **    **    **    *
 *     *     *     *     *  
 |P_PWM|P_PWM|P_PWM|P_PWM|

 La résolution est deux fois moins grande en triangle.
 
 #define VirtualReso_UpRamp 1000 est la résolution que le user utilise mais
 n'est pas la résolution réelle
 
 #define  VirtualReso_Triangle (VirtualReso_UpRamp/2)
 
 Résolution réelle possible atteignable par le timer :
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
 
 Dans tous ces cas, la résolution "virtuelle" est garantie, c'est à dire que l'on 
 sacrifie des possibilités du timer. Ce sera 1000pts en upramp et 500 en triangle.
 
 NB: Le user croit que la réso est toujours de 1000, UpRamp ou Triangle. Cela signifie
 qu'il est trompé sur le triangle et sera limité à 500. C'est un choix.
 
 NB : lors de l'init, un test est fait pour comparer la résolution réelle et la résolution virtuelle
 Si la réelle est moins bonne que la virtuelle, plantage while (1).
 
 
*/


void ToolBox_PWM_SetPWMModeTIM3(char Voie);
/**
 * @name    Triangle(...) 
 * @Brief   Génère la référence de la PWM, forme triangulaire
 * @param : float F_PWM_KHZ_Val, valeur de fréquence voulue en kHz
 * @ret : _
 * @EXAMPLE  Triangle(20.0); // PWM à 20kHz
**/
void Triangle ( float F_PWM_KHZ_Val)
{
int Freq;
/* Détermination Delta pour 1µs*/
Freq=Clock_F103_getTimerCLK(TIM3);
Delta_Triangle=Freq/2000000; /*2*0.5µs*FHz */
	
// Réglage de la porteuse en triangle symétrique :
Resolution=PWM_Init(TIM3, 1, 2*F_PWM_KHZ_Val);
Resolution=PWM_Init(TIM3, 2, 2*F_PWM_KHZ_Val);
/* blocage sortie PWM*/
Desactive_Voie_PWM(1);	
Desactive_Voie_PWM(2);
	
	
//			New V5	
if (Resolution<VirtualReso_Triangle) while(1); // F_PWM_KHZ trop élevée
//      Fin New V5	
Bloque_Timer(TIM3);
CNT(TIM3)=0;	
CptUp(TIM3);
PWM_Center_Aligned(TIM3);
Delta_PWM= Delta_Triangle;
VirtualReso=VirtualReso_Triangle;

	/* Réglage de tous les rapport cy à 50% sans pour autant sortir les signaux
le but est d'éviter de produire un signal à 100% ou 0% à l'init*/	
R_Cyc_1(500);
R_Cyc_2(500);

/* lancement du timer*/
Run_Timer(TIM3);
}


/**
 * @name    UpRamp(...) 
 * @Brief   Génère la référence de la PWM, forme dent de scie
 * @param : float F_PWM_KHZ_Val, valeur de fréquence voulue en kHz
 * @ret : _
 * @EXAMPLE  UpRamp(20.0); // PWM à 20kHz
**/
void UpRamp ( float F_PWM_KHZ_Val)
{
	
int Freq;
/* Détermination Delta pour 1µs*/
Freq=Clock_F103_getTimerCLK(TIM3);
Delta_UpRamp=Freq/1000000; /*1µs*FHz  */
	
Resolution=PWM_Init(TIM3, 1, F_PWM_KHZ_Val);
Resolution=PWM_Init(TIM3, 2, F_PWM_KHZ_Val);
/* blocage sortie PWM*/
Desactive_Voie_PWM(1);	
Desactive_Voie_PWM(2);
	
	//			New V5	
if (Resolution<VirtualReso_UpRamp) while(1); // F_PWM_KHZ doit être inférieure ou égale à 72kHz
//      Fin New V5
Bloque_Timer(TIM3);
CNT(TIM3)=0;
CptUp(TIM3);
PWM_Edge_Aligned(TIM3);
Delta_PWM= Delta_UpRamp;
VirtualReso=VirtualReso_UpRamp;
	

/* Réglage de tous les rapport cy à 50% sans pour autant sortir les signaux
le but est déviter de produire un signal à 100% ou 0% à l'init*/	
R_Cyc_1(500);
R_Cyc_2(500);
/* lancement du timer*/
Run_Timer(TIM3);
	
}




/**
 * @name    Active_Voie_PWM(...) / Desactive_Voie_PWM
 * @Brief   Valide / invalide la voie, cad, permet / empêche le signal de sortir du Timer
 * @param : char Voie, de 1 à 4
 * @ret : _
 * @EXAMPLE  Active_Voie_PWM(1);  // active la voie 1
**/
void Active_Voie_PWM(char Voie)
{
	PWM_ChannelEnable(TIM3,Voie);
	Run_Timer(TIM3);
}

void Desactive_Voie_PWM(char Voie)
{
	PWM_ChannelDisable(TIM3,Voie);
	Bloque_Timer(TIM3);
}

void Inv_Voie(char Voie)
{
	PWM_Polarite_Neg(TIM3,Voie);
}


/*===================================================================
			New V5		PWM  !! Rapport cyclique de 0 à 1000  <=> 0 à 100% !!
=====================================================================*/

/**
 * @name    R_Cyc_1 (...),  R_Cyc_2 (...)
 * @Brief   Met à jour le rapport cyclique (en synchronisme avec la référence)
						de la voie 1 et 2. 
 * @param : int Val compris entre 0 et 1000
 [0 : 1000] -> [0% : 100%]
 * @note   : 0% et 100% ne sont en réalité pas atteingnables. Une limite est prévue
						limite basse Th=1µs, limite haute Th=T-1µs
 * @ret : _
 * @EXAMPLE   R_Cyc_1(325); // impose un rapport cyclique de 32.5%
**/
void R_Cyc_1(int Val) 
{
	int MyCCR;
	if (Resolution<VirtualReso) while(1); // Appelez Triangle() ou UpRamp() avant !
																	
	MyCCR = ((Resolution*Val)/VirtualReso_UpRamp);
	if (MyCCR>=(Resolution-Delta_PWM)) MyCCR=Resolution-Delta_PWM; // max calé T -1µs
	if (MyCCR<=(Delta_PWM)) MyCCR=Delta_PWM;
	PWM_Valeur(TIM3,1)=MyCCR;
}	

void R_Cyc_2(int Val) 
{
	int MyCCR;
	if (Resolution<VirtualReso) while(1); 	// t'aurais pas oublié d'initialiser la PWM avant par hasard ?
																	// utilise la fonction Triangle par exemple lors des init !
																	
	MyCCR = ((Resolution*Val)/VirtualReso_UpRamp);
	if (MyCCR>=(Resolution-Delta_PWM)) MyCCR=Resolution-Delta_PWM; // max calé T -1µs
	if (MyCCR<=(Delta_PWM)) MyCCR=Delta_PWM;
	PWM_Valeur(TIM3,2)=MyCCR;
}



