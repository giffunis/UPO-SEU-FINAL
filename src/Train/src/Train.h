#include <LiquidCrystal.h>

class Train
{
private:

    // Declaración de los pines del motor
    int _enPin;     // PWM se conecta al pin 1 del puente-H
    int _in1Pin;    // Entrada 2 del puente-H
    int _in2Pin;    // Entrada 7 del puente-H
    
    int _pos;       // La posición del tren (m).
    int _speed;     // La velocidad del tren (m/s).
    LiquidCrystal* _lcd;
    
    
    void lcdPrintText(String displayText1, String displayText2);
    void dcMotorSetup();
public:
    Train(LiquidCrystal* lcd);
    ~Train();
    void begin(int enPin, int in1Pin, int in2Pin);
    void setSpeed(int speed);
    int getSpeed();
    
    void showTimeToStation(int stationPos);
    int timeToStation(int stationPos);
};

Train::Train(LiquidCrystal* lcd)
{
    _lcd = lcd;
}

Train::~Train()
{
}

void begin(int enPin, int in1Pin, int in2Pin)
{
    _enPin = enPin;
    _in1Pin = in1Pin;
    _in2Pin = in2Pin;

    //Inicializamos la controladora del motor
    dcMotorSetup();
}

void setSpeed(int speed)
{
    _speed = speed;

}

/**
 * Imprime el tiempo por pantalla en segundos.
*/
void showTimeToStation(int stationPos)
{
    // calculamos el tiempo
    int time = timeToStation(stationPos) / 60;

    // mostramos por el monitor lcd
    _lcd->clear();
    lcdPrint(0, "Next station in");
    lcdPrint(1, String(time) + " min")
}

/**
 * Devuelve el tiempo restante hasta la estación en segundos
*/
int timeToStation(int stationPos)
{
    return (stationPos - _pos) * 1.0 / _speed;
}






/**
 * Imprime por la pantalla lcd los dos textos que recibe por parámetros
*/
void lcdPrintText(String displayText1 = "", String displayText2 = "")
{
    _lcd->clear();
    _lcd->setCursor(0, 0); // set the cursor to column 0, line 0
    _lcd->print(displayText1);
    _lcd->setCursos(0, 1): // set the cursor to column 0, line 1
    _lcd.print(displayText2);
}


void dcMotorSetup(){
  // Definimos el modo de los pines
  pinMode(_in1Pin, OUTPUT);
  pinMode(_in2Pin, OUTPUT);

  // Establecemos el sentido del giro 'horario'
  digitalWrite(_in1Pin,LOW);
  digitalWrite(_in2Pin,HIGH);

  // No le proporcionamos potencia al motor.
  analogWrite(_enPin, 0);
}