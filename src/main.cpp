#include "Arduino.h"
#include "TimerOne.h"

#define inputCommand 13 // сигнал открыть/закрыть от контроллера
#define outputPWM 10    // выход ШИМ

#define ledTOP 11    // индикация состояния верхнего микрика
#define ledBOTTOM 12 // индикация состояния нижнего микрика

#define switchTOP 3    // вход микрика (Состояние шлагбаума: 1 - закрыт, 0 - открыт)
#define switchBOTTOM 2 // вход микрика (Состояние шлагбаума: 0 - закрыт, 1 - открыт)

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

void setup()
{
    Serial.begin(9600);

    pinMode(inputCommand, INPUT);
    pinMode(outputPWM, OUTPUT);
    pinMode(ledTOP, OUTPUT);
    pinMode(ledBOTTOM, OUTPUT);
    pinMode(switchTOP, INPUT);
    pinMode(switchBOTTOM, INPUT);

    pinMode(pinSpeedUp1, INPUT_PULLUP); // 4
    pinMode(pinSpeedUp2, INPUT_PULLUP); // 5
    pinMode(pinSpeedUp3, INPUT_PULLUP); // 6

    pinMode(pinSlowDown1, INPUT_PULLUP); // 7
    pinMode(pinSlowDown2, INPUT_PULLUP); // 8
    pinMode(pinSlowDown3, INPUT_PULLUP); // 9
    
    initializeSpeed();

    if (digitalRead(switchTOP) && !digitalRead(switchBOTTOM)) Serial.println("The barrier is CLOSED");
    else if (!digitalRead(switchTOP) && digitalRead(switchBOTTOM)) Serial.println("The barrier is OPENED");
    else Serial.println("Barrier in the MIDDLE position");

    attachInterrupt(digitalPinToInterrupt(switchTOP), switchTopInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(switchBOTTOM), switchBottomInterrupt, CHANGE);
    Timer1.initialize(1000000 / 30000); // 18.5 KHz
}

bool isCommand = 0;

void loop()
{
    if (isCommand)
    {
        // проверяем, действительно ли есть сигнал на входе
        if (digitalRead(inputCommand) == HIGH)
        {
        } // если есть, ничего не делаем
        else if (digitalRead(inputCommand) == LOW)
        { // если нет - выключаем ШИМ, ставим флаг в 0
            Timer1.pwm(outputPWM, 0);
            isCommand = 0;
        }
    }
    else
    {
        // проверяем, действительно ли нет сигнала на входе
        if (digitalRead(inputCommand) == LOW)
        {
        } // если нет, ничего не делаем
        else if (digitalRead(inputCommand) == HIGH)
        { // если есть - включаем шим, ставим флаг в 1
            Timer1.pwm(outputPWM, 1023 / 100 * speedUP);
            isCommand = 1;
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
    { // если кнопка в 1 (отпущена), значит стрела двигается вверх, включаем замедление
        digitalWrite(ledTOP, LOW);
        Timer1.pwm(outputPWM, 1023 / 100 * slowDown);
        Serial.println("UP SWITCH - Slow Down 1 ==> GO UP");
    }
    else if (!digitalRead(switchTOP))
    { // если кнопка в 0 (нажата), значит шлагбаум открыт
        digitalWrite(ledTOP, HIGH);
        Timer1.pwm(outputPWM, 1023 / 100 * 10); // торможение в конце
        Serial.println("UP SWITCH - Slow Down 2 ==> OPENED");
    }
}

void switchBottomInterrupt()
{
    // сработал нижний свитч - проверяем,
    if (digitalRead(switchBOTTOM))
    { // если кнопка в 1 (отпущена), значит стрела двигается вниз, включаем замедление
        digitalWrite(ledBOTTOM, LOW);
        if (slowDown == 0) {}
        else Timer1.pwm(outputPWM, 1023 / 100 * slowDown);
        Serial.println("LOW SWITCH - Slow Down 1 ==> GO DOWN");
    }
    else if (!digitalRead(switchBOTTOM))
    { // если кнопка в 0 (нажата), значит шлагбаум закрыт
        digitalWrite(ledBOTTOM, HIGH);
        Timer1.pwm(outputPWM, 1023 / 100 * 10); // торможение в конце
        Serial.println("LOW SWITCH - Slow Down 2 ==> CLOSED");
    }
}
