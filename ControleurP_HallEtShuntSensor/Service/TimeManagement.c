/*!
 * \brief 	Gestion du temps
 * \details module permettant de gérer les délais et les timeout avec
						le timer Systick (quantum : 1ms).
						Le module propose donc des fonctions d'attente indépendantes
						du compilateur et de ses options de compilation
 * \author 	T.R. 
 * \date   	2024
 * \version 1.0
 * \warning Systick clocké à 72MHz supposé.
 * \SoftDep	Timer_1234 
 * \HardDep	Systick
 */


#include "TimeManagement.h"



/* la date d'échéance pour les diverses base de tps */
int Time_TimeOutDate[TimeBaseName_Nb];  

/*______________________________________________________________________________

* __________________________________________________________________________________*/
#define PrioSystick 0
volatile int Cpt_1ms;

/**
 * @name  Time_Init()
 * @Brief Initialise le systick, le lance, et incrémente en interne une variable volatile, 
	Cpt_1ms, à chaque débordement du Systick.
 * @param _
 * @ret : _
 * @EXAMPLE Time_TimeOutInit();
**/
void Time_Init(void)
{
	/* La fonction ne se lancera qu'une seule fois même si appelée plusieurs fois*/

		if ((SysTick->CTRL&0x1)==0) // test pour voir si pas déjà parti !
		{
			NVIC_SetPriority(SysTick_IRQn, PrioSystick);
			NVIC_EnableIRQ(SysTick_IRQn);

			Cpt_1ms=0;
			/* Config Systick à 1ms d'autoreload + lancement*/
			SysTick->LOAD=9000-1; // 72MHz/8 = 9Mhz, config à 100µs
			SysTick->CTRL|=1<<1; // Interruption locale validée
			SysTick->CTRL|=1<<0; // systick on
		}
	
}


int SystickGet(void)
{
	return Cpt_1ms;
}

/* Handler qui claque toutes les 1ms*/
void SysTick_Handler(void)
{
	Cpt_1ms++;
}


/**
 * @name  Time_ChronoStart(...)
 * @Brief Démarre un des 6 chronomètres. Précis à 1ms près par définition.
 * @note : dans le détail, cette fonction fixe la date d'échéance, cad la valeur que doit
					atteindre la variable centrale Cpt_1ms
 * @param : TimeBaseName Name, au choix parmi Chrono_1 à Chrono_6
 * @param : valeur en ms
 * @return : _
 * @EXAMPLE Time_ChronoStart(Chrono_2, 10); // lance le chrono 2 pour 10ms à 1ms près
 **/ 
void Time_ChronoStart(TimeBaseName Name , int ms)
{
	Time_TimeOutDate[Name]=ms+SystickGet();
}


/**
 * @name  Time_IsTimeOut(...)
 * @Brief  Interroge si un timeout a eu lieu sur le chrono considéré. Si oui, renvoie 1, sinon 0
 * @note : vient lire la valeur courante de Cpt_1ms et la compare à la date d'échéance prévue.
 * @param : TimeBaseName Name, au choix parmi Chrono_1 à Chrono_6
 * @return : 1 si timeout, 0 sinon 
 * @EXAMPLE Time_IsTimeOut(Chrono_2); // demande si le chrono_2 est arrivé à échéance
 **/
char Time_IsTimeOut(TimeBaseName Name)
{
	if ((SystickGet()-Time_TimeOutDate[Name])>=0) return 1;
	else return 0;
}


/**
 * @name  Time_Delay_x_ms(...)
 * @Brief  Fonction bloquante qui redonne la main après x ms
 * @note : Elabore la date d'échéance d'un chrono interne (autre que les 6 définis),
					attend l'échéance
 * @param : in x : nombre de ms à compter (1ms près par définition)
 * @return : _
 * Exemple :Time_Delay_x_ms(100); // attend 100ms (1ms près)
 **/ 
void Time_Delay_x_ms(int x)
{
	int Chrono_Delay;
	/* photo du temps au départ du chrono (c'est là que se trouve la précsion de 1ms ..*/
	Chrono_Delay=x+SystickGet();
	while((SystickGet()-Chrono_Delay)<=0);
}