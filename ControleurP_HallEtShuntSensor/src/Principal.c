

#include "main.h"
#include "Clock_F103.h"
#include "TimeManagement.h"

#include "LCD_Balance.h"
#include "StringFct.h"
#include "ToolBox_NRJ_uC_Interface.h"


/******************************************************************************
* File Name          : Principal.c 
* Author             : T Rocacher
* Version            : V0
* Date release			 : 21/01/2025
* Description        : Code de d�part pour contr�leur de puissance Etudiant Hall Sensor						
* Processeur				 : STM32F103, 
* Outil Developpement: KEIL 5.40.0.3
* Documentation			 : _
********************************************************************************/



/* Fonctionnement de d�part 
- L'ADC est configur� en polling (lancement/attente/lecture r�sultat)
- les valeurs 12bits peuvent �tre lues par les fonctions d�crites dans ToolBox_NRJ_uC_Interface.h

- La PWM est assur�e par TIM3, configur� en mode triangle � 20kHz
- les sorties sont compl�mentaires
- la r�solution est de 1000pts 

- une interruption configur�e pour claquer � Te est mise en place,
r�cup�r�e par le callback  IT_Principale


*/


/* Define*/
#define Fe 2000.0 					/* [Hz] */
#define Te (1.0/Fe)					/* [sec] */
#define Ft 200.0						/* [Hz] */
#define TROTT 2  						/* 1 (Hall + ordre 1)
															2 (Hall + ordre 2+1)
															ou 3 (Shunt + ordre2+1) en fonction de la trott*/ 


#define R 1 
#define L 0.002
#define Ubat 24
#define GAIN_LEM 0.104
#define GAIN_SHUNT 0.12
#define R_ORDRE1 5100
#define C_ORDRE1 0.0000000022
#define PI 3.141592


#if TROTT == 1 /*hall + ordre 1*/

	#define TAU_I (2*Ubat*GAIN_LEM)/(2*PI*200)
	#define K L/(R*TAU_I)

#elif TROTT == 2 /*hall + ordre 2+1*/
	
	#define TAU_I (2*Ubat*GAIN_LEM)/(2*PI*200)
	#define K L/(R*TAU_I)

#elif TROTT == 3 /*shunt + ordre 2+1*/
	
	#define TAU_I (2*Ubat*GAIN_SHUNT)/(2*PI*200)
	#define K L/(R*TAU_I) 
#endif

#define CORRECTEUR (1 + K * TAU_I)/(TAU_I)

/*IT Functions*/
void IT_Principale(void);
void IT_signaux_carre (void);

/* LCD functions compl�mentaires*/
void LCD_ClearTopLine(void);
void LCD_ClearDownLine(void);

/*****************************************************************
**								VARIABLES     															  **
******************************************************************/
static float pot = 0;
static float courant = 0;
static float erreur = 0;
static float erreur_prev = 0;
static float sortie = 0;
static float sortie_prev = 0;




/*****************************************************************
**								MAIN Fonction 															  **
******************************************************************/


char MessageLCD[50];
int f=0;


int main(void)
{
	
	float K_2 =K;
	float tau =TAU_I;
	float te =Te;
	float coeff = -K+(Te/tau);
	/* Configuration Clock HSI si HSE ne fonctionne pas (pb quartz)*/
	Clock_F103_ConfMainClk64MHz(Clock_HSE_Quartz); //� changer si ya un pb av� le quartz
	/* intialisation Time pour pouvoir utiliser la fonction Time_Delay_x_ms*/
	Time_Init(); 
		
	/*======================= CONF IO  ============================*/
	Conf_Generale_IO_Carte();
  
	
	
	/*======================= CONF LCD ============================*/	
	/*LCD*/
	lcd_Init();
	Time_Delay_x_ms(300); /* indispensable pour que l'init du LCD se passe bien*/
	lcd_Clear();
	
	LCD_ClearTopLine();
	lcd_Print("Bienvenue ! ");
	LCD_ClearDownLine();

	/*============== Conf PWM  ==================*/
	Triangle(20.0);   /*  PWM en ref triangulaire 20kHz*/
	Active_Voie_PWM(1);
	Active_Voie_PWM(2);
	//Inv_Voie(2);
	R_Cyc_1(500);
	R_Cyc_2(500);
	
	/*======================= CONF ADC ============================*/	
	Conf_ADC();

	/*======================= CONF TIM2 en interruption � Te     ===*/	 
	Conf_IT_Principale_TIM(TIM2,IT_Principale,Te*1000000.0); 
	Conf_IT_Principale_TIM(TIM4,IT_signaux_carre,1000000.0); //appelle notre fonction carr� toutes les 1sec pour voir la r�ponse dynamique du syst
	
	/* ====================================================
	               Main loop		
		=====================================================*/ 
	while(1)
	{	
		if (f ==1){
			LCD_ClearTopLine();
			lcd_Print(MessageLCD);
			LCD_ClearDownLine();
		}
	}
	
	
}


/*****************************************************************
**								Fonction Interruptions 											  **
******************************************************************/
void IT_signaux_carre(void) //g�n�rer un signaux carr� pour voir l ar�ponse dynamique du syst�me avec un �chelon en entr�e et compare ravec le simulink
{
	if (pot <= 1.57){pot=1.80;} //pot = 1.65 �quivalent au 0 du potentiom�tre et pot =1.77 �quivalent � une consigne � 0.12A du potentiom�tre
	else 
	{pot =1.50;
	}
}

void IT_Principale(void)
{
	
	//R�cup des entr�es
  //pot = (float)(Read_Cons()*3.3/4095.0);
	courant = (float)(Read_I()*3.3/4095.0);


	//ref = Read_Ref();
	StringFct_Float2Str(courant,MessageLCD, 3, 2);
	
	erreur = (float)(pot - courant);
	
	sortie = sortie_prev + K*erreur +(-(K)+((Te)/(TAU_I)))*erreur_prev;
	f++;
	if (f==100){f=0;}
	//Saturateur
	if(sortie > 0.45){
		sortie = 0.45;
	}else if(sortie < -0.45){
		sortie = -0.45;
	}
	
	//M�J des vals prev
	erreur_prev = erreur;
	sortie_prev = sortie;
	
	//M�J des PWM
	R_Cyc_1((sortie + 0.5) * 1000);
	R_Cyc_2(1000 - ((sortie+0.5)*1000));
}



/*****************************************************************
**						Fonction 	autres..															  **
******************************************************************/



/* FUNCTIONS LCD SPECFICS*/
void LCD_ClearTopLine(void)
{
	lcd_Set_cursor(0,0);
	lcd_Print("                ");
	lcd_Set_cursor(0,0);
}

void LCD_ClearDownLine(void)
{
	lcd_Set_cursor(0,1);
	lcd_Print("                ");
	lcd_Set_cursor(0,1);
}
