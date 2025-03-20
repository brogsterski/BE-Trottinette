#ifndef _TimeManag_H__
#define _TimeManag_H__

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


#include "Timer_1234.h"

/**
 * Enumération TimeBaseName
 * @note : Il s'agit de définir jusqu'à 6 chronomètres pouvant fonctionner
					en même temps.
 **/ 
typedef enum {
	Chrono_1,
	Chrono_2,
	Chrono_3,
	Chrono_4,
	Chrono_5,
	Chrono_6, 
}TimeBaseName;
#define TimeBaseName_Nb 6 


/**
 * @name  Time_Init()
 * @Brief Initialise le systick, le lance, et incrémente en interne une variable volatile, 
	Cpt_1ms, à chaque débordement du Systick.
 * @param _
 * @ret : _
 * @EXAMPLE Time_TimeOutInit();
**/
void Time_Init(void);



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
void Time_ChronoStart(TimeBaseName Name , int ms);



/**
 * @name  Time_IsTimeOut(...)
 * @Brief  Interroge si un timeout a eu lieu sur le chrono considéré. Si oui, renvoie 1, sinon 0
 * @note : vient lire la valeur courante de Cpt_1ms et la compare à la date d'échéance prévue.
 * @param : TimeBaseName Name, au choix parmi Chrono_1 à Chrono_6
 * @return : 1 si timeout, 0 sinon 
 * @EXAMPLE Time_IsTimeOut(Chrono_2); // demande si le chrono_2 est arrivé à échéance
 **/ 
char Time_IsTimeOut(TimeBaseName Name);


/**
 * @name  Time_Delay_x_ms(...)
 * @Brief  Fonction bloquante qui redonne la main après x ms
 * @note : Elabore la date d'échéance d'un chrono interne (autre que les 6 définis),
					attend l'échéance
 * @param : in x : nombre de ms à compter (1ms près par définition)
 * @return : _
 * Exemple :Time_Delay_x_ms(100); // attend 100ms (1ms près)
 **/ 
void Time_Delay_x_ms(int x);

#endif
