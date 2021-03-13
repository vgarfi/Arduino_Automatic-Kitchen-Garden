#include <TimerOne.h>
#include <LiquidCrystal.h>
/*
 * CODIGO UTILIZADO PARA EXPOCISIÓN: La huerta automática toma los valores de sus sensores y cada un minuto (60 segundos) evalúa si debe ser regada o no, y por cuanot tiempo
 * Utiliza un sensor de humedad resistivo (para sensar la humedad de la tierra), una manguera plástica de 6 mm (para transportar el agua),
 * una bomba de agua WATER-PUMP-120LH (para mover el agua desde el tanque hasta la tierra por la manguera), un sensor LDR de luz (para detectar niveles de luz solar) y
 * una pantalla LCD de 16x2 (para mostrar los valores de los sensores de luz y de humedad)
 
 */

#define US_TIMERONE           1000000 // Queremos que cuente cada segundos, no necesitamos que sea de a milisegundos

#define PIN_RS                7
#define PIN_EN                8
#define PIN_D4                9
#define PIN_D5                10
#define PIN_D6                11
#define PIN_D7                12

#define CANT_COL          16
#define CANT_FIL          2

#define PRIM_COL          0
#define PRIM_FIL          0
#define SEG_FIL           1

#define MSJ_1             "Humedad: "
#define MSJ_2             "Luz: "
#define MSJ_PORCENTAJE    "%"

#define KG_TIERRA         2 //este valor lo modifica el usuario, dependiendo de la cantidad de tierra que posea su huerta

#define LUZ_ALTA          500
#define LUZ_MEDIA         150
#define LUZ_BAJA          80

#define HUMEDAD_ALTA      70
#define HUMEDAD_MEDIA     45
#define HUMEDAD_BAJA      20

LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_D4, PIN_D5, PIN_D6, PIN_D7);

#define PIN_SENSOR_H          A0
#define PIN_SENSOR_L          A1
#define PIN_BOMBA_AGUA        6

#define ESTADO_ESPERA         0
#define LECTURA_IMPRESION_LCD 1
#define EVALUACION_DATOS      2
#define ACTIVAR_RIEGO         3

#define TIEMPO_DE_ESPERA      8
#define RESETEO_CONTADOR      60

#define TIEMPO_ESPERA_EXPO    1

int estadoHuerta = ESTADO_ESPERA;

int valorSensorHumedad, valorSensorLuz;

bool flagTiempoBomba = false;
int tiempoBomba, segundosRiego;
float segundosDeRiegoFloat;

int segsEspera, minsEspera, horasEspera;

int leerSensorYMapear(int pinSensor, int valorMaximo);
void imprimirLCD (int valorHumedad, int valorLuz);
void declararPines (void);
void maquinaHuertaAutomatica (void);

float tiempoDeRiego (int cantidadHumedad, int cantidadLuz, int cantidadTierra);

void timer (void);

void setup() {
  // put your setup code here, to run once:
declararPines();
lcd.begin(CANT_COL, CANT_FIL);
Timer1.initialize(US_TIMERONE);
Timer1.attachInterrupt(timer);

Serial.begin(9600);
valorSensorHumedad = leerSensorYMapear(PIN_SENSOR_H, 100);
valorSensorLuz = leerSensorYMapear(PIN_SENSOR_L, 1000);
imprimirLCD(valorSensorHumedad, valorSensorLuz);
}

void loop() {
  maquinaHuertaAutomatica();
}

void maquinaHuertaAutomatica (void)
{
  switch (estadoHuerta)
  {
    case ESTADO_ESPERA:
                         if (minsEspera >= TIEMPO_ESPERA_EXPO) // seteamos este tiempo solo para exponer la huerta
                         {
                            minsEspera = 0;
                            estadoHuerta = LECTURA_IMPRESION_LCD; 
                         }
    break;

    case LECTURA_IMPRESION_LCD:
                          lcd.clear();
                          valorSensorHumedad = leerSensorYMapear(PIN_SENSOR_H, 100);
                          valorSensorLuz = leerSensorYMapear(PIN_SENSOR_L, 1000);
                          imprimirLCD(valorSensorHumedad, valorSensorLuz);
                          estadoHuerta = EVALUACION_DATOS;
                          
    break;

    case EVALUACION_DATOS:
                         /* 0 - 30 muy seco (se riega independientemente de la luz)     10%
                          * 31 - 55 seco    (se riega dependiendo de la luz)            7%
                          * 56 - 80 normal  (se riega menos tiempo)                     5%
                          * 81 - 100 humedo (no se riega)
                          * 
                          * 1/2 litro = 15 seg
                         */
                         if (valorSensorHumedad <= HUMEDAD_ALTA) // se considera que la huerta debe recibir algún tipo de riego
                         {
                          segundosDeRiegoFloat = tiempoDeRiego (valorSensorHumedad, valorSensorLuz, KG_TIERRA);
                          segundosRiego = round(segundosDeRiegoFloat); // redondeamos el valor a un entero para poder
                          flagTiempoBomba = true;
                          estadoHuerta = ACTIVAR_RIEGO;
                         }

                         else
                         {
                          estadoHuerta = ESTADO_ESPERA;
                         }
                           
    break;

    case ACTIVAR_RIEGO:
                      digitalWrite (PIN_BOMBA_AGUA, HIGH);
                      
                      if (tiempoBomba >= segundosRiego)
                      {
                        lcd.clear();
                        tiempoBomba = 0;
                        segundosRiego = 0;
                        digitalWrite (PIN_BOMBA_AGUA, LOW);
                        flagTiempoBomba = false;
                        estadoHuerta = ESTADO_ESPERA;
                      }
    break;
  }
}

int leerSensorYMapear(int pinSensor, int valorMaximo) //CORREGIR ESTO Y SACAR IFS
{
  int lecturaSensor, valorSensor;
  
  lecturaSensor = analogRead(pinSensor);
  if(pinSensor == A1)
  {
    valorSensor = map(lecturaSensor, 1023, 0, valorMaximo, 0);
  }

  else
  {
    valorSensor = map(lecturaSensor, 1023, 0, 0, valorMaximo);
  }
  

  return valorSensor;
}

void declararPines (void)
{
  pinMode (PIN_SENSOR_H, INPUT);
  pinMode (PIN_SENSOR_L, INPUT);
  pinMode (PIN_BOMBA_AGUA, OUTPUT);
}

void imprimirLCD (int valorHumedad, int valorLuz)
{
  lcd.setCursor(PRIM_COL, PRIM_FIL);
  lcd.print(MSJ_1 + String(valorHumedad) + MSJ_PORCENTAJE);
  lcd.setCursor(PRIM_COL, SEG_FIL);
  lcd.print(MSJ_2 + String(valorLuz) + MSJ_PORCENTAJE);
}



float tiempoDeRiego (int cantidadHumedad, int cantidadLuz, int cantidadTierra)
{
  float segsRiego;
  float ltsRiego;

  // 1 lt = 30 seg
  
  if (cantidadHumedad <= HUMEDAD_BAJA) // estado de tierra CRITICO (se riega el 10%)
  {
    ltsRiego = cantidadTierra * 0.1;
    segsRiego = ltsRiego * 30;
  }

  else if (cantidadHumedad > HUMEDAD_BAJA && cantidadHumedad <= HUMEDAD_MEDIA && cantidadLuz <= LUZ_MEDIA) // estado de tierra SECO (se riega el 7% si es el atardecer o la noche)
  {
    ltsRiego = cantidadTierra * 0.07;
    segsRiego = ltsRiego * 30;
  }

  else if (cantidadHumedad > HUMEDAD_MEDIA && cantidadHumedad <= HUMEDAD_ALTA && cantidadLuz <= LUZ_BAJA) // estado de tierra NORMAL (se riega el 5% solo si es de noche)
  {
    ltsRiego = cantidadTierra * 0.05;
    segsRiego = ltsRiego * 30; 
  }

  return segsRiego;
}

void timer (void)
{
  segsEspera = segsEspera + 1;

  if (segsEspera >= RESETEO_CONTADOR)
  {
    minsEspera = minsEspera + 1;
    segsEspera = 0;
  }

  if (minsEspera >= RESETEO_CONTADOR)
  {
    horasEspera = horasEspera + 1;
    minsEspera = 0;
  }


  if (flagTiempoBomba == true)
  {
    tiempoBomba = tiempoBomba + 1;
  }
}
