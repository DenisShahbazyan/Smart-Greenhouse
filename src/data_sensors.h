#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SDHT.h>

/***********************************************************************
/                              ПЕРЕМЕННЫЕ                              /
***********************************************************************/
LiquidCrystal_I2C lcd(0x27, 20, 4);
SDHT dht;

/***********************************************************************
/                           ОБЪЯВЛЕНИЕ ПИНОВ                           /
***********************************************************************/
const byte PIN_DHT = A3;           //пин датчика температуры и влажности воздуха
const byte PIN_SOIL_MOISTURE = A6; //пин датчика влажности почвы

/***********************************************************************
/                                ФУНКЦИИ                               /
***********************************************************************/
//Инициализация монитора
void lcd_init()
{
  //Инициализация монитора
  lcd.init();
  lcd.backlight();
}

//Установка pinMode
void set_pinMode_for_sensors_INPUT()
{
  //Делаем пин с датчиком температуры и влажности воздуха в INPUT
  pinMode(PIN_DHT, INPUT);

  //Делаем пин для датчика влажности почвы INPUT
  pinMode(PIN_SOIL_MOISTURE, INPUT);
}

//Функция получения температуры воздуха
double get_Temperature()
{
  dht.read(DHT11, PIN_DHT);
  return (double)dht.celsius / 10;
}

//Функция получения влажности воздуха
double get_Humidity()
{
  dht.read(DHT11, PIN_DHT);
  return (double)dht.humidity / 10;
}

//Функция получения влажности почвы в %
int get_SoilMoisture()
{
  return map(analogRead(PIN_SOIL_MOISTURE), 300, 800, 100, 0);
}