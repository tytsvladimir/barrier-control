#include "Arduino.h"
#include "TimerOne.h"

#define inputCommand 9   // сигнал открыть/закрыть от контроллера
#define outputPWM 10  // выход ШИМ
#define switchUP 2   // вход микрика (Состояние шлагбаума: 1 - закрыт, 0 - открыт)
#define switchDOWN 3 // вход микрика (Состояние шлагбаума: 0 - закрыт, 1 - открыт)
#define speedUP 90    // ускорение в %
#define slowDown 40   // торможение в %

int openedOrClosed = false; // false - шлагбаум закрыт, true = открыт


void setup() {
  Serial.begin(9600);
  pinMode(inputCommand, INPUT);
  pinMode(outputPWM, OUTPUT);
  pinMode(switchUP, INPUT);
  pinMode(switchDOWN, INPUT);

  if (digitalRead(switchUP) && !digitalRead(switchDOWN)) {
    openedOrClosed = false;
    Serial.println("The barrier is closed");
  } else if (!digitalRead(switchUP) && digitalRead(switchDOWN)) {
    openedOrClosed = true;
    Serial.println("The barrier is opened");
  } else {
    openedOrClosed = NULL;
    Serial.println("Barrier in the middle position");
  }

  attachInterrupt(digitalPinToInterrupt(switchDOWN), switchDownInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(switchUP), switchUpInterrupt, CHANGE);
  Timer1.initialize(1000000/18500);
}

bool isCommand = 0;

void loop() {
  if (isCommand) {
    //проверяем, действительно ли есть сигнал на входе
    //если есть, ничего не делаем, если нет - выключаем ШИМ, ставим флаг в 0
    if (digitalRead(inputCommand) == HIGH) {}
    else if (digitalRead(inputCommand) == LOW) {
      Timer1.pwm(outputPWM, 0);
      isCommand = 0;
    }

  } else {
    //проверяем, действительно ли нет сигнала на входе
    //если нет, ничего не делаем, если есть - включаем шим, ставим флаг в 1
    if (digitalRead(inputCommand) == LOW) {}
    else if (digitalRead(inputCommand) == HIGH) {
      Timer1.pwm(outputPWM, 1023/100*speedUP);
      isCommand = 1;
    }

  }
}


void switchDownInterrupt() {
  //сработал верхний свитч - проверяем, если кнопка в 1 (отпущена),
  //значит стрела двигается вниз, включаем замедление
  //если кнопка в 0 (нажата), значит шлагбаум закрыт
  if (digitalRead(switchDOWN)) {
    Timer1.pwm(outputPWM, 1023/100*slowDown);
    Serial.println("Slow Down ==> GO-UP");
  } else if (!digitalRead(switchDOWN)) {
    Serial.println("The barrier was closed");
  }
}

void switchUpInterrupt() {
  //сработал нижний свитч - проверяем, если кнопка в 1 (отпущена),
  //значит стрела двигается вверх, включаем замедление
  //если кнопка в 0 (нажата), значит шлагбаум открыт
  if (digitalRead(switchUP)) {
    Timer1.pwm(outputPWM, 1023/100*slowDown);
    Serial.println("Slow Down ==> GO-DOWN");
  } else if (!digitalRead(switchUP)) {
    Serial.println("The barrier was opened");
  }
}
