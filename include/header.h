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

// настройка скорости разгона в %
#define speedUp1 70
#define speedUp2 80
#define speedUp3 90

// джамперы подстройки торможения
#define pinSlowDown1 9
#define pinSlowDown2 8
#define pinSlowDown3 7

// настройка скорости торможения в %
#define slowDown1 70
#define slowDown2 80
#define slowDown3 90