#include "Arduino.h"
#include "TimerOne.h"

#define inputCommand 13   // сигнал открыть/закрыть от контроллера
#define outputPWM 10      // выход ШИМ

#define ledUP 11          //индикация состояния верхнего микрика
#define ledDOWN 12        //индикация состояния нижнего микрика

#define switchUP 3        // вход микрика (Состояние шлагбаума: 1 - закрыт, 0 - открыт)
#define switchDOWN 2      // вход микрика (Состояние шлагбаума: 0 - закрыт, 1 - открыт)

//джамперы подстройки разгона
#define pinSpeedUp70 4
#define pinSpeedUp80 5
#define pinSpeedUp90 6

//джамперы подстройки торможения
#define pinSlowDown50 7
#define pinSlowDown70 8
#define pinSlowDown80 9

int speedUP;                  // ускорение в %
int slowDown;                 // торможение в %

void setup() {
  Serial.begin(9600);

  pinMode(inputCommand, INPUT);
  pinMode(outputPWM, OUTPUT);
  pinMode(ledUP, OUTPUT);
  pinMode(ledDOWN, OUTPUT);
  pinMode(switchUP, INPUT);
  pinMode(switchDOWN, INPUT);

  pinMode(pinSpeedUp70, INPUT_PULLUP);  //4
  pinMode(pinSpeedUp80, INPUT_PULLUP);  //5
  pinMode(pinSpeedUp90, INPUT_PULLUP);  //6

  pinMode(pinSlowDown50, INPUT_PULLUP); //7
  pinMode(pinSlowDown70, INPUT_PULLUP); //8
  pinMode(pinSlowDown80, INPUT_PULLUP); //9

  initializeSpeed();

  if (digitalRead(switchUP) && !digitalRead(switchDOWN)) Serial.println("The barrier is CLOSED");
  else if (!digitalRead(switchUP) && digitalRead(switchDOWN)) Serial.println("The barrier is OPENED");
  else Serial.println("Barrier in the MIDDLE position");

  attachInterrupt(digitalPinToInterrupt(switchDOWN), switchDownInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(switchUP), switchUpInterrupt, CHANGE);
  Timer1.initialize(1000000/30000);     //18.5 KHz
}

bool isCommand = 0;

void loop() {
  if (isCommand) {
    //проверяем, действительно ли есть сигнал на входе
    if (digitalRead(inputCommand) == HIGH) {}       //если есть, ничего не делаем
    else if (digitalRead(inputCommand) == LOW) {    //если нет - выключаем ШИМ, ставим флаг в 0
      Timer1.pwm(outputPWM, 0);
      isCommand = 0;
    }

  } else {
    //проверяем, действительно ли нет сигнала на входе    
    if (digitalRead(inputCommand) == LOW) {}        //если нет, ничего не делаем
    else if (digitalRead(inputCommand) == HIGH) {   //если есть - включаем шим, ставим флаг в 1
      Timer1.pwm(outputPWM, 1023/100*speedUP);
      isCommand = 1;
    }

  }
}

void initializeSpeed() {
  // Инициализация джамперов настройки скоростей разгона и торможения
  if (digitalRead(pinSpeedUp70) == LOW) {speedUP = 70;}
  else if(digitalRead(pinSpeedUp80) == LOW) {speedUP = 80;}
  else if(digitalRead(pinSpeedUp90) == LOW) {speedUP = 90;}

  if (digitalRead(pinSlowDown50) == LOW) {slowDown = 50;}
  else if (digitalRead(pinSlowDown70) == LOW) {slowDown = 70;}
  else if (digitalRead(pinSlowDown80) == LOW) {slowDown = 80;}
  else slowDown = 0;

  Serial.print("⇧"); //⇧⇩
  Serial.print(speedUP);
  Serial.print("⇧ ");
  Serial.print("  ⇩");
  Serial.print(slowDown);
  Serial.println("⇩");
}

void switchUpInterrupt() {
  //сработал верхний свитч - проверяем,  
  if (digitalRead(switchUP)) {                      //если кнопка в 1 (отпущена), значит стрела двигается вверх, включаем замедление
    digitalWrite(ledUP, LOW);
    if (slowDown == 0) {} else Timer1.pwm(outputPWM, 1023/100*slowDown);    
    Serial.println("UP SWITCH - Slow Down 1 ==> GO UP");
  } else if (!digitalRead(switchUP)) {              //если кнопка в 0 (нажата), значит шлагбаум открыт
    digitalWrite(ledUP, HIGH);
    Timer1.pwm(outputPWM, 1023/100*10);              //торможение в конце
    Serial.println("UP SWITCH - Slow Down 2 ==> OPENED");
  }
}

void switchDownInterrupt() {
  //сработал нижний свитч - проверяем,
  if (digitalRead(switchDOWN)) {                     //если кнопка в 1 (отпущена), значит стрела двигается вниз, включаем замедление
    digitalWrite(ledDOWN, LOW);
    if (slowDown == 0) {} else Timer1.pwm(outputPWM, 1023/100*slowDown);
    Serial.println("LOW SWITCH - Slow Down 1 ==> GO DOWN");
  } else if (!digitalRead(switchDOWN)) {             //если кнопка в 0 (нажата), значит шлагбаум закрыт
    digitalWrite(ledDOWN, HIGH);
    Timer1.pwm(outputPWM, 1023/100*10);              //торможение в конце
    Serial.println("LOW SWITCH - Slow Down 2 ==> CLOSED");
  }
}
