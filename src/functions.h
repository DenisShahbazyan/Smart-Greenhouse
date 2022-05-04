#include <Arduino.h>
#include <data_sensors.h>

/***********************************************************************
/                           ОБЪЯВЛЕНИЕ ПИНОВ                           /
***********************************************************************/
const byte PIN_RELAY_00 = A0; //Пин для переполюсовки 24 вольт
const byte PIN_RELAY_01 = A1; //Пин для переполюсовки 24 вольт
const byte PIN_RELAY_02 = 2;  //Пин для включения оключения 220в на преобразователь
const byte PIN_RELAY_03 = 3;  //Пин для включения отключения 220в на соленоидный клапан для полива

/***********************************************************************
/                              ПЕРЕМЕННЫЕ                              /
***********************************************************************/
unsigned long last_time_lcd = 0; // millis обновления температуры и влажности воздуха

unsigned long last_time_door_open;
unsigned long last_time_door_close;
unsigned long last_time_poliv;

bool flag_door = false; //флаг хранит положение открытия или закрытия двери

bool flag_open = false;  //флаг хранит положение вошли ли мы хоть раз в открытие двери, чтобы если после первого ожидания температура упадет, мы завершили полноценно открытие двери
bool flag_close = false; //флаг хранит положение вошли ли мы хоть раз в закрытие двери, чтобы если после первого ожидания температура поднимется, мы завершили полноценно закрытие двери

//Флаги каждого хода открытия двери (чтобы каждый этап не повторялся)
bool flag_open_1 = false;
bool flag_open_2 = false;
bool flag_open_3 = false;

//Флаги каждого хода закрытия двери (чтобы каждый этап не повторялся)
bool flag_close_1 = false;
bool flag_close_2 = false;
bool flag_close_3 = false;
/***********************************************************************
/                               КОНСТАНТЫ                              /
***********************************************************************/
const int SLEEP_LCD = 1000; //Задержка обновления экрана

const int TEMPERATURE_MAX = 35; //Температура при которой открывается дверь
const int TEMPERATURE_MIN = 20; //Температура при которой закрывается дверь

const int SOIL_MIN = 80; //Порог влажности в % при котором начинается полив
const int SOIL_MAX = 90; //Порог влажности в % при котором останавливается полив

/***********************************************************************
/                                ФУНКЦИИ                               /
***********************************************************************/
//Делаем все пины, на которых сидят реле выходными
void set_pinMode_for_relay_OUTPUT()
{
    //Делаем пины, на которых сидят реле ВЫХОДНЫМИ
    pinMode(PIN_RELAY_00, OUTPUT);
    pinMode(PIN_RELAY_01, OUTPUT);
    pinMode(PIN_RELAY_02, OUTPUT);
    pinMode(PIN_RELAY_03, OUTPUT);
}

//Задаем стартовые позиции для реле
void set_start_position_for_relay()
{
    //Отключаем все реле
    digitalWrite(PIN_RELAY_00, 1);
    digitalWrite(PIN_RELAY_01, 1);
    digitalWrite(PIN_RELAY_02, 0); //Вырубаем питание 220в на открытие дверей
    digitalWrite(PIN_RELAY_02, 0); //Вырубаем питание 220в на клапане
}

//Отображение температуры и влажности воздуха
void lcd_view()
{
    if (millis() - last_time_lcd > SLEEP_LCD)
    {
        last_time_lcd = millis();

        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(get_Temperature());
        lcd.print(" C");

        lcd.setCursor(0, 1);
        lcd.print("Soil: ");
        lcd.print(get_SoilMoisture());
        lcd.print(" %");
    }
}

void door()
{
    if ((get_Temperature() >= TEMPERATURE_MAX || flag_open == true) && flag_door == false)
    {
        if (millis() - last_time_door_open > 1000 && flag_open_1 == false)
        {
            last_time_door_open = millis();
            flag_open_1 = true;
            flag_open = true;

            digitalWrite(PIN_RELAY_00, 0); // 1 - задвинуть, 0 - выдвинуть
            digitalWrite(PIN_RELAY_01, 0); // 1 - задвинуть, 0 - выдвинуть
        }
        if (millis() - last_time_door_open > 2000 && flag_open_2 == false)
        {
            last_time_door_open = millis();
            flag_open_2 = true;

            digitalWrite(PIN_RELAY_02, 1); // 0 - убрать пиатние 220 с преобразователя, 1 - подать питание
        }
        if (millis() - last_time_door_open > 22000 && flag_open_3 == false)
        {
            last_time_door_open = millis();
            flag_open_3 = true;

            digitalWrite(PIN_RELAY_02, 0); // 0 - убрать пиатние 220 с преобразователя, 1 - подать питание

            flag_door = true;

            flag_open = false;

            flag_close_1 = false;
            flag_close_2 = false;
            flag_close_3 = false;
        }
    }

    if ((get_Temperature() <= TEMPERATURE_MIN || flag_close == true) && flag_door == true)
    {
        if (millis() - last_time_door_close > 1000 && flag_close_1 == false)
        {
            last_time_door_close = millis();
            flag_close_1 = true;
            flag_close = true;

            digitalWrite(PIN_RELAY_00, 1); // 1 - задвинуть, 0 - выдвинуть
            digitalWrite(PIN_RELAY_01, 1); // 1 - задвинуть, 0 - выдвинуть
        }
        if (millis() - last_time_door_close > 2000 && flag_close_2 == false)
        {
            last_time_door_close = millis();
            flag_close_2 = true;

            digitalWrite(PIN_RELAY_02, 1); // 0 - убрать пиатние 220 с преобразователя, 1 - подать питание
        }
        if (millis() - last_time_door_close > 22000 && flag_close_3 == false)
        {
            last_time_door_close = millis();
            flag_close_3 = true;

            digitalWrite(PIN_RELAY_02, 0); // 0 - убрать пиатние 220 с преобразователя, 1 - подать питание

            flag_door = false;

            flag_close = false;

            flag_open_1 = false;
            flag_open_2 = false;
            flag_open_3 = false;
        }
    }
}

void poliv()
{
    // Нужно сделать так, чтобы полив включался с 28%, догонял влажность до 35% и отключался, влажность снова упадет до 28% и полив должен включиться
    if (millis() - last_time_poliv > 1000)
    {
        last_time_poliv = millis();

        if (get_SoilMoisture() <= SOIL_MIN)
        {
            digitalWrite(PIN_RELAY_03, 1); // 0 - убрать пиатние 220 с клапана, 1 - подать питание
        }
        if (get_SoilMoisture() >= SOIL_MAX)
        {
            digitalWrite(PIN_RELAY_03, 0); // 0 - убрать пиатние 220 с клапана, 1 - подать питание
        }
    }
}