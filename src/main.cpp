#include <Arduino.h>
#include <config.h>
#include <config.h>
#include <ChainableLED.h>
#include <I2C_RTC.h>
#include <SparkFunBME280.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define ECHANTILLON_MOY 10
#define NB_CAPTEURS 10

#define DEBUG

#ifdef DEBUG
#define CONFIG_MODE_TIME 1
#else
#define CONFIG_MODE_TIME 30
#endif

static ChainableLED led(6, 7, 1);
static DS1307 RTC;
static BME280 vma;
SoftwareSerial SoftSerial(4, 5);
TinyGPS gps;

//tableau de couleur pour la LED
int red[3]    = {255,0,0};
int orange[3] = {153,76,0};
int yellow[3] = {255,255,0};
int green[3]  = {0,255,0};
int blue[3]   = {0,0,255};
int white[3]  = {255,255,255};


int mode = 1;/*0 maintenance ; 1 standard  ; 2 eco || définit le mode actuel  */
int previous_mode = 1;
#define MAINTENANCE 0 
#define STANDARD    1
#define ECO         2

//variables pour chauqe boutons servant dans le changement de mode
bool stepredbutton = false;
bool stepgreenbutton = false;
unsigned long red_timer , green_timer , config_timer;
const byte redbutton = 2;
const byte greenbutton = 3;

#define No_GPS  5

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
  float moyenne;
  int hist_moyenne[ECHANTILLON_MOY]; 
  int rgberror[3];                     
  int hertzerror;
  int pin; // 255 correspond au port serie / 0 à 99 digital / 100 à 200 analog
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

//prototype des fonctiyons
void mesure_capteurs(capteur *pointeur);
void changemode_red_button();
void changemode_green_button();
void set_led_color(int rgb[3]);
void changement_mode(int newmode);
void getRTC();
char getGPS();

structRTC RealTimeClock;

void setup() 
{
  Serial.begin(9600);
  led.init();
  pinMode(redbutton,INPUT);
  pinMode(greenbutton,INPUT);
  attachInterrupt(digitalPinToInterrupt(redbutton),changemode_red_button ,CHANGE);
  attachInterrupt(digitalPinToInterrupt(greenbutton),changemode_green_button ,CHANGE);

  
  SoftSerial.begin(9600);  
  RTC.begin();

  vma.settings.commInterface = I2C_MODE; 
  vma.settings.I2CAddress = 0x76;
  vma.settings.runMode = 3; 
  vma.settings.tStandby = 0;
  vma.settings.filter = 0;
  vma.settings.tempOverSample = 1 ;
  vma.settings.pressOverSample = 1;
  vma.settings.humidOverSample = 1;
  delay(20);
  vma.begin();

  if ( digitalRead(redbutton) == LOW)
  {
    
    changement_mode(4);
    set_led_color(yellow);
    config_timer = millis();
    while (millis() - config_timer < CONFIG_MODE_TIME * 60000)
    {
      Serial.println("config mode");
    }
    changement_mode(1);
  }

}

void loop() 
{
  //Serial.println(getGPS(), 2);
  delay(1000);
  switch (mode)
  {
  case 0:
    set_led_color(orange);
    break;
  case 1:
    set_led_color(green);
    break;
  case 2:
    set_led_color(blue);
    break;
  }
  capteur *tabptr[NB_CAPTEURS];
  int i ;
  if (mode > 0)
  {
    if (millis() % (LOG_INTERVALL*60000 * mode) == 0)
    {
      //readRTC();
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
            mesure_capteurs(tabptr[i]);
          }
        }
        else
        {
          mesure_capteurs(tabptr[i]) ;
        }

      //data_to_history();
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
  previous_mode = mode;
  mode = newmode;
  #ifdef DEBUG
  Serial.println(mode);
  #endif
}

void changemode_red_button()
{  
  if (stepredbutton == true  && digitalRead(redbutton) == 0 ) 
  {
    stepredbutton = false;
  }
  
  if (stepredbutton == false  && mode != 4 ) 
  {
    stepredbutton = true;
    red_timer = millis();
  }
  else if (millis() - red_timer < 300) stepredbutton = false; 
  else if (millis() - red_timer >= 5000)
  {
    switch (mode)
    {
    case 0:
      changement_mode(previous_mode);
      break;
    
    case 1:
      changement_mode(0);
      break;
    
    case 2:
      changement_mode(0);
      break;
    }
    stepredbutton = false;
  }
  else stepredbutton = false;
  #ifdef DEBUG
  Serial.print("red button time :" );
  Serial.print(digitalRead(redbutton) );
  Serial.print("   " );
  Serial.println( millis() - red_timer);
  #endif
}

void changemode_green_button()
{
  if (stepgreenbutton == true  && digitalRead(greenbutton) == HIGH ) 
  {
    stepgreenbutton = false;
  }

  if (stepgreenbutton == false && mode != 4) 
  {
    stepgreenbutton = true;
    green_timer = millis();
  }
  else if (millis() - green_timer < 200) stepgreenbutton = false; 
  else if (millis() - green_timer >= 5000)
  {
    switch (mode)
    {
    case 0:
      ;
      break;
    
    case 1:
      changement_mode(2);
      break;
    
    case 2:
      changement_mode(1);
      break;
    }
    stepgreenbutton = false;
  }
  else stepgreenbutton = false;
  #ifdef DEBUG
  Serial.print("green button time: " );
  Serial.print(stepgreenbutton);
  Serial.println( millis() - green_timer);
  #endif
}

void mesure_capteurs(capteur *pointeur)
{
  ;
}

void set_led_color(int rgb[3])
{
  led.setColorRGB(0,rgb[0],rgb[1],rgb[2]);
}

void getRTC()
{
  RealTimeClock.seconde = RTC.getSeconds();
  RealTimeClock.minute  = RTC.getMinutes();
  RealTimeClock.heure   = RTC.getHours();
  RealTimeClock.jour    = RTC.getDay();
  RealTimeClock.mois    = RTC.getMonth();
  RealTimeClock.annee   = RTC.getYear();
}

char getGPS()
{
  unsigned char buffer[64];
  int count=0;
  if (SoftSerial.available())
  {
    while(SoftSerial.available())               // reading data into char array
        {
            buffer[count++]=SoftSerial.read();      // writing data into array
            if(count == 64)break;
        }
  }
  return buffer;
}