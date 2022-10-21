#include <Arduino.h>
#include <ChainableLED.h>
#include <I2C_RTC.h>
#include <SparkFunBME280.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SD.h>
#include <MemoryFree.h>

#define ECHANTILLON_MOY 10
#define NB_CAPTEURS 5

//#define DEBUG

#ifdef DEBUG
#define CONFIG_MODE_TIME 1
#else
#define CONFIG_MODE_TIME 30
#endif

static ChainableLED led(6, 7, 1);
static DS1307 RTC;
static BME280 vma;
TinyGPS gps;
SoftwareSerial ss(8,9);
File myfile;

char lastfile[12];

//tableau de couleur pour la LED
const byte red[3]    = {255,0,0};
const byte orange[3] = {153,76,0};
const byte yellow[3] = {255,255,0};
const byte green[3]  = {0,255,0};
const byte blue[3]   = {0,0,255};
const byte white[3]  = {255,255,255};


byte mode = 1;/*0 maintenance ; 1 standard  ; 2 eco || définit le mode actuel  */
byte previous_mode = 1;
#define MAINTENANCE 0
#define STANDARD    1
#define ECO         2

//variables pour chauqe boutons servant dans le changement de mode
bool stepredbutton = false;
bool stepgreenbutton = false;
unsigned long red_timer , green_timer , config_timer;
const PROGMEM byte redbutton = 2;
const PROGMEM byte greenbutton = 3;

#define No_GPS  4

int LOG_INTERVALL=1; // intervale entre 2 mesures
int FILE_MAX_SIZE=2048; // taille d'un fichier log
byte TIMEOUT=30; //


typedef struct 
{
  byte seconde;
  byte minute;
  byte heure;
  byte jour;
  byte mois;
  int annee;
}structRTC;

//prototype des fonctions
String mesure_capteurs(byte num);
void changemode_red_button();
void changemode_green_button();
void set_led_color(const byte rgb[3]);
void changement_mode(byte newmode);
void getRTC();
void store_to_sd(String str);
static void smartdelay(unsigned long ms);
String getGPS();
float moy_float(int tab[ECHANTILLON_MOY]);

structRTC RealTimeClock;




void setup() 
{
  SD.begin(4);
  Serial.begin(9600);
  led.init();
  pinMode(redbutton,INPUT);
  pinMode(greenbutton,INPUT);
  attachInterrupt(digitalPinToInterrupt(redbutton),changemode_red_button ,CHANGE);
  attachInterrupt(digitalPinToInterrupt(greenbutton),changemode_green_button ,CHANGE);

  
  ss.begin(9600);  
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
  delay(300);
}

void loop() 
{
  delay(10);
  String values;
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
  byte i ;
  if (mode > 0)
  {
    if (millis() % (LOG_INTERVALL*10000 * mode) < 100 )
    {
      //readRTC();
      for (i = 0 ; i< NB_CAPTEURS ; i++)
      {
        if (mode == 2 )
        {
          if (i == No_GPS)
          {
            if (millis() % (LOG_INTERVALL*60000 * 4) <1000) values += mesure_capteurs(i) ;
          }
          else
          {
            values += mesure_capteurs(i);
          }
        }
        else
        {
          values += mesure_capteurs(i) ;
        }

      
      }
      store_to_sd(values + "\n");
      values = "";
      delay(100);
    }
  }
  else
  {
    while (mode == 0)
    {
      getRTC();
      values = String(RealTimeClock.heure) + ":" + String(RealTimeClock.minute )+ ":" + String(RealTimeClock.seconde) + "  ";
      for (i=0 ; i<NB_CAPTEURS ; i++)
      {
        values += mesure_capteurs(i);
      }
      Serial.println(values);
      values = "";
    }
    
  }
}

void changement_mode(byte newmode)
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

String mesure_capteurs(byte num)
{
  byte i = 0;
  int moyenne[ECHANTILLON_MOY];
  switch (num)
  {
  case 0:
    for (i= 0 ; i<ECHANTILLON_MOY ; i++)
    {
      moyenne[i] = vma.readFloatHumidity();
    }
    return  " H: " + String(moy_float(moyenne));
    break;
  
  case 1:
    for (i= 0 ; i<ECHANTILLON_MOY ; i++)
    {
      moyenne[i] = vma.readTempC();
    }
    return  "T :" + String(moy_float(moyenne)) + " C";
    break;
  
  case 2:
    for (i= 0 ; i<ECHANTILLON_MOY ; i++)
    {
      moyenne[i] = vma.readFloatPressure() / 100;
    }
    return "P :" +  String(moy_float(moyenne)) +" hPa" ;
    break;
  
  case 3:
    for (i= 0 ; i<ECHANTILLON_MOY ; i++)
    {
      moyenne[i] = analogRead(A0);
    }
    return "Lum :" +String(moy_float(moyenne));
    break;
  case 4:
    return  getGPS();
    break;
  }
}


void set_led_color(const byte rgb[3])
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

String getGPS()
{
  float flat, flon;
  smartdelay(200);
  gps.f_get_position(&flat, &flon);
  return "Latitude : " + String(flat , 3) + " Longitude : " + String(flon , 3);
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

float moy_float(int tab[ECHANTILLON_MOY])
{
  byte i;
  float result;
  for (i=0 ; i<ECHANTILLON_MOY ; i++)
  {
    result += tab[i];
  }
  //Serial.println(freeMemory());
  return result/ECHANTILLON_MOY;
}

void store_to_sd(String str)
{
  byte i = 0;
  String filename;
  getRTC();
  //myfile = SD.open(lastfile , FILE_WRITE);
  //Serial.println(freeMemory());
  if (myfile.size() + sizeof(str) > FILE_MAX_SIZE) 
  {
    getRTC();
    myfile.close();

    filename = String(RealTimeClock.annee) + "/"+ String(RealTimeClock.mois) +"/"+ String(RealTimeClock.jour) + "/"+  +".txt";
  }
  myfile.close();
}