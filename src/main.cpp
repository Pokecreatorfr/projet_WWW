#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>

#define ECHANTILLON_MOY 10
#define NB_CAPTEURS 10

int mode = 1;/*0 maintenance ; 1 standard  ; 2 eco || définit le mode actuel  */
#define STANDARD 0 
#define ECO 1
#define MAINTENANCE 2

//variables pour chauqe boutons servant dans le changement de mode
bool stepredbutton = false;
bool stepgreenbutton = false;
unsigned long red_timer , green_timer;

int LOG_INTERVALL=10; // intervale entre 2 mesures
int FILE_MAX_SIZE=4096; // taille d'un fichier log
int TIMEOUT=30; //

typedef struct historique
{
  int data; ///a faire pour chaque capteur 
  int seconde;
  int minute;
  int heure;
  int jour;
  int mois;
  int annee;
  struct historique*suivant;
}historique;

typedef struct
{
  String value; // la valeur sera stockée sous forme de texte
  bool error;
  int rgberror[3];                     
  int hertzerror;
  int pin; // 255 correspond au port serie 
}capteur;

typedef struct 
{
  int seconde;
  int minute;
  int heure;
  int jour;
  int mois;
  int annee;
}structRTC;


void setup() 
{
  Serial.begin(9600);

  attachInterrupt(,changemode_red_button ,CHANGE)
  attachInterrupt(,changemode_green_button ,CHANGE)
}

void loop() 
{
  int *tabptr[NB_CAPTEURS];
  int i ;
  if (mode > 0)
  {
    if (millis() % (LOG_INTERVALL*60000 * mode) == 0)
    {
      readRTC();
      for (i = 0 ; i< NB_CAPTEURS ; i++)
      {
        if (mode == 2 )
        {
          if (i == No_GPS)
          {
            if (millis() % (LOG_INTERVALL*60000 * 4) == 0) mesure_capteurs(tabptr[i]) ;
          }
          else
          {
            mesure_capteurs(tabptr[i]) ;
          }
        }
        else
        {
          mesure_capteurs(tabptr[i]) ;
        }

      data_to_history();
      }
    }
  }
  else
  {
    while (mode == 2)
    {
      ;//permettre d'ecceder aux données de la carte SD depuis le port série avec des commandes
    }
}

}

void changement_mode(int newmode)
{
  mode = newmode;
}

void changemode_red_button()
{
  double long a;
  double long b;
  
  if (stepredbutton == false) 
  {
  stepredbutton = true;
  red_timer = millis();
  }
  else if( millis() - red_timer >= 5000)
  {
    switch (mode)
    {
    case 0:
      
      break;
    
    case 1:

      break;
    
    case 2:

      break;
    }
    stepredbutton = true;
  }
  else stepredbutton = true;
  for (a=0; a<10000 ; a++) //cela prend environ 100ms a s'executer et remplace donc delay(100)
  {
    b++;
  }
}

void changemode_green_button()
{
  double long a;
  double long b;
  
  if (stepgreenbutton == false) stepgreenbutton = true;
  else
  {
    switch (mode)
    {
    case 0:
      
      break;
    
    case 1:

      break;
    
    case 2:

      break;
    }
    stepgreenbutton = true;
  }
  for (a=0; a<10000 ; a++) //cela prend environ 100ms a s'executer et remplace donc delay(100)
  {
    b++;
  }
}

void mesure_capteurs(capteur *pointeur)
{

}