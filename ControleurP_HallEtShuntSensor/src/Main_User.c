
/*
	!!!! NB : ALIMENTER LA CARTE AVANT DE CONNECTER L'USB !!!
	
	==== Nouvelle version de projet compatible avec Compilateur 6  ===
	====				2024																							 ===

*/


/*
STRUCTURE DES FICHIERS

COUCHE APPLI : 
Main_User.c : programme principal à modifier. 
Par défaut le pgm utilise la PWM1 et l'entrée analogique +/-10V
Le programme recopie l'état de l'entrée vers la PWM à une périodicité de 100ms: 
- potentiomètre au minimum donne le rapport cyclique le plus petit
- potentiomètre au maximum donne le rapport cyclique le plus grand.


COUCHE SERVICE :
Toolbox_NRJ_V6.c : contient toutes les fonctions utilisables, qui "parlent" à un électrotechnicien
Table_Cosinus_Tri.c : contient 3 tables de cosinus déphasés de 120°, 36 pts (10° de résolution)
à ajouter si besoin


COUCHE DRIVER :
Clock_F103.c : le fonction d'init à lancer en tout premier. Contient aussi des fonctions de 
lecture pour connaître les fréquences essentielles de sortie du RCC.
Rem : les lignes 135 et 136 de startup_stm32f10x_md.s sont commentées

lib : bibliothèque qui gère les périphériques du STM : Drivers_STM32F103_107_Nov2024.lib
*/



/*
VOIR LE FICHIER TOOLBOX_NRJ_v6.h POUR TOUTES INFOS SUR LES E/S, PWM...
En bref :

***********************
What's new vesion 5 ...
***********************
PWM et PWM_Aux
Limitation fréquence PWM à 70kHz
saturation automatique à 1µs min, T-1µs max ( R_Cyc_1 est maintenant une fonction qui gère la satu non plus une simple macro)

Resolution PWM = 1000 fixe. Cad on est à 0.1 % près

***********************
What's new vesion 6 ...
***********************

- On n'utilise plus clock.c mais Clock_F103.c qui configure la RCC
à la place du startup et qui contient des fonctions de lectures des
valeurs de fréquences essentielles (SysCk, TimxCk, ADCCk...)
- la fonctionnalité PWM est revue pour s'adapter automatiquement
à la fréquence effective des timers non forcément 72MHz.

*/

#include "ToolBox_NRJ_uC_Interface.h"
#include "Clock_F103.h"
#include "TimeManagement.h"

//=================================================================================================================
// 					USER DEFINE
//=================================================================================================================

// Choix de la fréquence PWM (en kHz)
#define FPWM_Khz 20.0 
// Période de l'interruption principale Systick
#define Te_us 100000.0 // par défaut 100ms

//==========END USER DEFINE========================================================================================

// ========= Variable globales indispensables et déclarations fct d'IT ============================================
void IT_Principale(void);
void IT_Ext_3V3(void);
//=================================================================================================================


/*=================================================================================================================
 					FONCTION MAIN : 
					NB : On veillera à allumer les diodes au niveau des E/S utilisée par le progamme. 
					

//=================================================================================================================*/

volatile unsigned int MyFreq;

int main (void)
{

	
// !OBLIGATOIRE! //	
Clock_F103_ConfMainClk72MHzHSE();	
	
Conf_Generale_IO_Carte();
Conf_Sorties_PWM_Aux_Mode_PushPull_Idpt();
//______________ Ecrire ici toutes les CONFIGURATIONS des périphériques ________________________________

	
// Paramétrage ADC pour entrée analogique
Conf_ADC();
	
// Conf IT
Conf_IT_Principale_Systick(IT_Principale, Te_us);
	
// Configuration de la PWM avec une porteuse Triangle 
UpRamp(FPWM_Khz);
Active_Voie_PWM(1);	
Bras_1_On;
Start_PWM;



// Activation LED
LED_Courant_Off;
LED_PWM_On;
LED_PWM_Aux_Off;
LED_Entree_10V_On;
LED_Entree_3V3_Off;
LED_Codeur_Off;

	while(1)
	{}

}





//=================================================================================================================
// 					FONCTION D'INTERRUPTION PRINCIPALE SYSTICK
//=================================================================================================================
int Cy_1, Cy_2, Cy_3;
int Courant_1,Courant_2,Courant_3,In_10V;



void IT_Principale(void)
{
 // Acquisition analogique (voie 11 de l'ADC)
 In_10V=Entree_10V(); // inutile dans ce code.
 
 // calcul de la PWM
 Cy_1 = (In_10V*1000)/4096;
	
 // sortie de la PWM
 R_Cyc_1(Cy_1);

}

//=================================================================================================================
// 					FONCTION D'INTERRUPTION EXTERNE (Entrée 0/3V3) 
//=================================================================================================================

void IT_Ext_3V3(void)
{
}

