#include "Arduino.h"
#include "TimerOne.h"
#include "header.h"

int speedUP;  // ускорение в %
int slowDown; // торможение в %

void switchTopInterrupt();
void switchBottomInterrupt();
void initializeSpeed();
void initializePosition();
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
    initializePosition();

    attachInterrupt(digitalPinToInterrupt(switchTOP), switchTopInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(switchBOTTOM), switchBottomInterrupt, CHANGE);
    Timer1.initialize(1000000 / 18500); // 18.5 KHz
}

bool isCommand = 0; // если ли команда
int countError = 0; // счетчик ошибок
int countWork = 0;
bool wasCheck = 0; // флаг, была ли проверка

void loop()
{ // если сигнал на включение ШИМ активен
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
    else // если сигнал на включение ШИМ НЕ активен
    {
        // проверяем, действительно ли нет сигнала на входе
        if (digitalRead(inputCommand) == LOW)
        {
            // если концвой свитч сработал, ничего не делаем
            if (digitalRead(switchTOP) && !digitalRead(switchBOTTOM) || 
            !digitalRead(switchTOP) && digitalRead(switchBOTTOM))
            {
                countWork += countError > 0 ? 1 : 0; // если ошибки в работе были, считаем количество открываний без ошибок
                //countError = countWork > 3 ? 0 : countError; // если количество открываний без ошибок > 3, обнуляем счетчик ошибок
                if (countWork > 3)
                {
                    countError = 0;
                    Serial.println("The error counter has been RESET.");
                }
            }
            // если концвой свитч НЕ сработал...
            else if (!wasCheck) // если еще не проверяли состояние свитчей - делаем это!
            {
                countError++; // инкрементируем счетчик ошибок
                wasCheck = 1; // говорим что мы уже проверили состояние свитчей
                if (countError < amountErrorSwitch)
                {
                    Serial.print("ERROR SWITCH = ");
                    Serial.println(countError);
                }
                else 
                {
                    // если количество ошибок = допустимому, останавливаем работу контроллера до перезапуска вручную
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
            wasCheck = 0; // обнулить флаг проверки свитчей
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

void initializePosition()
{
    // Определяем текущую позицию стрелы
    if (digitalRead(switchTOP) && !digitalRead(switchBOTTOM))
    {
        digitalWrite(ledBOTTOM, HIGH);
        digitalWrite(ledTOP, LOW);
        Serial.println("The barrier is CLOSED");
    }
    else if (!digitalRead(switchTOP) && digitalRead(switchBOTTOM))
    {
        digitalWrite(ledTOP, HIGH);
        digitalWrite(ledBOTTOM, LOW);
        Serial.println("The barrier is OPENED");
    }
    else Serial.println("Barrier in the MIDDLE position");
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
        // если кнопка в 1 (отпущена), значит стрела двигается вниз, включаем замедление
        digitalWrite(ledBOTTOM, LOW);
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