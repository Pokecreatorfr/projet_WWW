#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>

#define ECHANTILLON_MOY 10
#define NB_CAPTEURS 10

int mode = 0;/*0 standard ; 1 eco ; 2 maintenance || définit le mode actuel  */
#define STANDARD 0 
#define ECO 1
#define MAINTENANCE 2

int LOG_INTERVALL=10; // intervale entre 2 mesures
int FILE_MAX_SIZE=4096; // taille d'un fichier log
int TIMEOUT=30; //

typedef struct element
{
  int data;
  struct element *suivant;
}element;

typedef struct
{
  int value;
  bool error;
  float moyenne;
  element *premier_element;
}capteur;


void setup() 
{
  int *tabptr[NB_CAPTEURS];
}

void loop() {
  // put your main code here, to run repeatedly:
}

void changement_mode(int newmode)
{
  mode = newmode;
}