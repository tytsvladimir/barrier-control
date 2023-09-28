#include "Arduino.h"
#include "TimerOne.h"

#define inputCommand 13 // сигнал открыть/закрыть от контроллера
#define outputPWM 10    // выход ШИМ

#define ledTOP 11    // индикация состояния верхнего микрика
#define ledBOTTOM 12 // индикация состояния нижнего микрика

#define switchTOP 3    // вход микрика (Состояние шлагбаума: 1 - закрыт, 0 - открыт)
#define switchBOTTOM 2 // вход микрика (Состояние шлагбаума: 0 - закрыт, 1 - открыт)

#define amountErrorSwitch 3 // количество допустимых ошибок свитчей (когда один из них сломался либо стрела не дошла до конца)

// джамперы подстройки разгона
#define pinSpeedUp1 6
#define pinSpeedUp2 5
#define pinSpeedUp3 4

// настройка скорости разгона
#define speedUp1 70
#define speedUp2 80
#define speedUp3 90

// джамперы подстройки торможения
#define pinSlowDown1 9
#define pinSlowDown2 8
#define pinSlowDown3 7

// настройка скорости торможения
#define slowDown1 70
#define slowDown2 80
#define slowDown3 90

int speedUP;  // ускорение в %
int slowDown; // торможение в %

void switchTopInterrupt();
void switchBottomInterrupt();
void initializeSpeed();
void warning();

void setup()
{
    Serial.begin(9600);

    pinMode(inputCommand, INPUT);
    pinMode(outputPWM, OUTPUT);
    pinMode(ledTOP, OUTPUT);
    pinMode(ledBOTTOM, OUTPUT);
    pinMode(switchTOP, INPUT);
    pinMode(switchBOTTOM, INPUT);

    pinMode(pinSpeedUp1, INPUT_PULLUP); // 6 pin
    pinMode(pinSpeedUp2, INPUT_PULLUP); // 5 pin
    pinMode(pinSpeedUp3, INPUT_PULLUP); // 4 pin

    pinMode(pinSlowDown1, INPUT_PULLUP); // 9 pin
    pinMode(pinSlowDown2, INPUT_PULLUP); // 8 pin
    pinMode(pinSlowDown3, INPUT_PULLUP); // 7 pin
    
    initializeSpeed();

    if (digitalRead(switchTOP) && !digitalRead(switchBOTTOM)) Serial.println("The barrier is CLOSED");
    else if (!digitalRead(switchTOP) && digitalRead(switchBOTTOM)) Serial.println("The barrier is OPENED");
    else Serial.println("Barrier in the MIDDLE position");

    attachInterrupt(digitalPinToInterrupt(switchTOP), switchTopInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(switchBOTTOM), switchBottomInterrupt, CHANGE);
    Timer1.initialize(1000000 / 18500); // 18.5 KHz
}

bool isCommand = 0;
int countError = 0;
bool isTest = 0;

void loop()
{
    if (isCommand)
    {
        // проверяем, действительно ли есть сигнал на входе
        if (digitalRead(inputCommand) == HIGH) {} // если есть, ничего не делаем
        else if (digitalRead(inputCommand) == LOW)
        {
            // если нет - выключаем ШИМ, ставим флаг в 0
            Timer1.pwm(outputPWM, 0);
            isCommand = 0;
        }
    }
    else
    {
        // проверяем, действительно ли нет сигнала на входе
        if (digitalRead(inputCommand) == LOW)
        {
            // если концвой свитч сработал, ничего не делаем
            if (digitalRead(switchTOP) && !digitalRead(switchBOTTOM) || 
            !digitalRead(switchTOP) && digitalRead(switchBOTTOM)) {}
            // если не сработал - останавливаем работу, включаем индикацию
            else if (!isTest) // если еще не проверяли состояние свитчей - делаем это!
            {
                countError++; // инкрементируем счетчик ошибок
                isTest = 1; // говорим что мы уже проверили состояние свитчей
                if (countError < amountErrorSwitch)
                {
                    Serial.print("ERROR SWITCH = ");
                    Serial.println(countError);
                }
                else 
                {
                    Serial.print("ERROR SWITCH = ");
                    Serial.println(countError);
                    Serial.println("WARNING SWITCH");
                    Timer1.pwm(outputPWM, 0);
                    detachInterrupt(digitalPinToInterrupt(switchTOP));
                    detachInterrupt(digitalPinToInterrupt(switchBOTTOM));
                    warning();
                }
            }
        }
        // если есть - включаем шим, ставим флаг в 1
        else if (digitalRead(inputCommand) == HIGH)
        {
            Timer1.pwm(outputPWM, 1023 / 100 * speedUP);
            isCommand = 1;
            isTest = 0;
        }
    }
}

void initializeSpeed()
{
    // Инициализация джамперов настройки скоростей разгона и торможения
    if (digitalRead(pinSpeedUp1) == LOW) {speedUP = speedUp1;}
    else if (digitalRead(pinSpeedUp2) == LOW) speedUP = speedUp2;
    else if (digitalRead(pinSpeedUp3) == LOW) speedUP = speedUp3;

    if (digitalRead(pinSlowDown1) == LOW) slowDown = slowDown1;
    else if (digitalRead(pinSlowDown2) == LOW) slowDown = slowDown2;
    else if (digitalRead(pinSlowDown3) == LOW) slowDown = slowDown3;

    Serial.print("⇧"); // ⇧⇩
    Serial.print(speedUP);
    Serial.print("⇧ ");
    Serial.print("  ⇩");
    Serial.print(slowDown);
    Serial.println("⇩");
}

void switchTopInterrupt()
{
    // сработал верхний свитч - проверяем,
    if (digitalRead(switchTOP))
    {
        // если кнопка в 1 (отпущена), значит стрела двигается вверх, включаем замедление
        digitalWrite(ledTOP, LOW);
        Timer1.pwm(outputPWM, 1023 / 100 * slowDown);
        Serial.println("Top switch is release (1) >> Slow Down 1 >> ⇩⇩⇩ GO DOWN ⇩⇩⇩");
    }
    else if (!digitalRead(switchTOP))
    {
        // если кнопка в 0 (нажата), включаем торможение, шлагбаум открыт
        digitalWrite(ledTOP, HIGH);
        Timer1.pwm(outputPWM, 1023 / 100 * 10); // торможение в конце
        Serial.println("Top switch is pressed (0) >> Slow Down 2 >> || OPENED ||");
    }
}

void switchBottomInterrupt()
{
    // сработал нижний свитч - проверяем,
    if (digitalRead(switchBOTTOM))
    {
        digitalWrite(ledBOTTOM, LOW);
        // если кнопка в 1 (отпущена), значит стрела двигается вниз, включаем замедление
        if (slowDown == 0) {}
        else Timer1.pwm(outputPWM, 1023 / 100 * slowDown);
        Serial.println("Bottom switch is release (1) - Slow Down 1 >> ⇧⇧⇧ GO UP ⇧⇧⇧");
    }
    else if (!digitalRead(switchBOTTOM))
    {
        // если кнопка в 0 (нажата), включаем торможение, шлагбаум закрыт
        digitalWrite(ledBOTTOM, HIGH);
        Timer1.pwm(outputPWM, 1023 / 100 * 10); // торможение в конце
        Serial.println("Bottom switch is pressed (0) >> Slow Down 2 >> == CLOSED ==");
    }
}

void warning() {
    // индикация ошибки
    while (1)
        {
            digitalWrite(ledTOP, HIGH);
            digitalWrite(ledBOTTOM, LOW);
            delay(300);
            digitalWrite(ledTOP, LOW);
            digitalWrite(ledBOTTOM, HIGH);
            delay(300);
        }
}