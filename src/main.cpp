#include <Arduino.h>
#include <stdio.h>
#include <config.h>

#define ECHANTILLON_MOY 10;
#define NB_CAPTEURS 10;

int LOG_INTERVALL=10;
int FILE_MAX_SIZE=4096;
int TIMEOUT=30;

typedef struct capteur
{
  int value;
  bool error;
  float moyenne;
  int tab[ECHANTILLON_MOY];
};


void setup() {
  int *tabptr[]
}

void loop() {
  // put your main code here, to run repeatedly:
}