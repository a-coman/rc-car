// Librerias
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


// Entradas y salidas digitales
// Motores izquierda
const int motorIzqPWM = 16;  // D0 en NodeMCU, GPIO 16
const int dirIzq1 = 5;       // D1 en NodeMCU, GPIO 5
const int dirIzq2 = 4;       // D2 en NodeMCU, GPIO 4

// Motores derecha
const int motorDerPWM = 14;  // D5 en NodeMCU, GPIO 14
const int dirDer1 = 12;      // D6 en NodeMCU, GPIO 12
const int dirDer2 = 13;      // D7 en NodeMCU, GPIO 13


// Credenciales WiFi
#define WIFI_SSID "RED"
#define WIFI_PASS "PASS"
#define UDP_PORT 2390


// Recepcion de paquetes UDP
#define VALORES_TOTALES 5  // Valores totales que vamos a recibir
#define TAM_PAQUETE 16     // Tamaño del buffer de paquetes a recibir
char packet[TAM_PAQUETE];  // Buffer de paquetes a recibir
WiFiUDP UDP;


// Variables de control de los motores
int marchaAdelante = 0;
int mAdelanteMapeada = 0;
int marchaAtras = 0;
int mAtrasMapeada = 0;
int direccion = 0;
int giroIzq = 0;
int giroDer = 0;

// Variables para gestionar el cambio de direccion en movimiento
const int factorFreno = 30;
const int factorAceleracion = 10;



void setup() {
  // Configuracion de pines
  // Configurar pines de los motores de la izquierda
  pinMode(motorIzqPWM, OUTPUT);
  pinMode(dirIzq1, OUTPUT);
  pinMode(dirIzq2, OUTPUT);

  // Configurar pines de los motores de la derecha
  pinMode(motorDerPWM, OUTPUT);
  pinMode(dirDer1, OUTPUT);
  pinMode(dirDer2, OUTPUT);

  // Establecer rango de valores PWM
  analogWriteRange(1023);


  // Configuracion WiFi
  // Iniciar puerto serial y WiFi
  Serial.begin(115200);  // Configurar de forma adecuada el Serial Port
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.println();
  Serial.print("Conectando a... ");
  Serial.println(WIFI_SSID);

  // Esperar hasta conseguir la conexion
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  // Conectado correctamente
  Serial.println();
  Serial.println("Conectado con exito! IP: ");
  Serial.println(WiFi.localIP());

  // Recepcion de paquetes UDP
  UDP.begin(UDP_PORT);
  Serial.println("Esperando paquetes UDP por el PUERTO: ");
  Serial.println(UDP_PORT);
  Serial.println();
}


void showValues() {
  Serial.println();
  Serial.println();

  Serial.print("Marcha atras: ");
  Serial.println(marchaAtras);
  Serial.print("Marcha adelante: ");
  Serial.println(marchaAdelante);
  Serial.print("Direccion: ");
  Serial.println(direccion);

  Serial.println();
  Serial.println();
}


void server() {
  // Tamaño del paquete recibido
  int packetSize = UDP.parsePacket();

  // Tras recibir un paquete -> packetSize > 0
  if (packetSize) {
    int len = UDP.read(packet, TAM_PAQUETE);
    // Terminamos el paquete con \0
    if (len > 0) {
      packet[len] = '\0';
    }

    // Procesamos el paquete para obtener los valores
    int values[VALORES_TOTALES];
    char *token;

    token = strtok(packet, ",");

    for (int i = 0; i < VALORES_TOTALES; i++) {
      values[i] = atoi(token);
      token = strtok(NULL, ",");
    }

    // Obtenemos los valores finales y los mostramos
    marchaAtras = values[0];
    marchaAdelante = values[1];
    direccion = values[2];
    giroIzq = values[3];
    giroDer = values[4];

    showValues();

    // Mapeo de marchaAtras: mAtrasMapeada
    mAtrasMapeada = map(marchaAtras, 0, 20, 0, 800);

    // Mapeo de marchaAdelante: mAdelanteMapeada
    mAdelanteMapeada = map(marchaAdelante, 0, 20, 0, 800);
  }
}


void loop() {
  // Lee y procesa los paquetes obtenidos
  server();

  // Control
  if (giroIzq) // Giro Estatico Izquierda -> LB
  {
    // Configurar direccion de los motores de la izquierda
    digitalWrite(dirIzq1, LOW);
    digitalWrite(dirIzq2, HIGH);

    // Configurar direccion de los motores de la derecha
    digitalWrite(dirDer1, LOW);
    digitalWrite(dirDer2, HIGH);

    // Configurar señal PWM de los motores de la izquierda
    analogWrite(motorIzqPWM, 600);

    // Configurar señal PWM de los motores de la derecha
    analogWrite(motorDerPWM, 600);
  }
  else if (giroDer) // Giro Estatico Derecha -> RB
  {
    // Configurar direccion de los motores de la izquierda
    digitalWrite(dirIzq1, HIGH);
    digitalWrite(dirIzq2, LOW);

    // Configurar direccion de los motores de la derecha
    digitalWrite(dirDer1, HIGH);
    digitalWrite(dirDer2, LOW);

    // Configurar señal PWM de los motores de la izquierda
    analogWrite(motorIzqPWM, 600);

    // Configurar señal PWM de los motores de la derecha
    analogWrite(motorDerPWM, 600);
  }

  // Avance del Robot
  if ((marchaAdelante - marchaAtras) > 5)  // Coche se mueve hacia adelante
  {
    // Configurar direccion de los motores de la izquierda
    digitalWrite(dirIzq1, LOW);
    digitalWrite(dirIzq2, HIGH);

    // Configurar direccion de los motores de la derecha
    digitalWrite(dirDer1, HIGH);
    digitalWrite(dirDer2, LOW);
  }
  
  // Retroceso del Robot
  if ((marchaAdelante - marchaAtras) < 5)  // Coche se mueve hacia atras
  {
    // Configurar direccion de los motores de la izquierda
    digitalWrite(dirIzq1, HIGH);
    digitalWrite(dirIzq2, LOW);

    // Configurar direccion de los motores de la derecha
    digitalWrite(dirDer1, LOW);
    digitalWrite(dirDer2, HIGH);
  }

  // Calculo del dutyCycle de la señal PWM a enviar a los motores de la izquierda
  int dutyCycleDer = abs(mAdelanteMapeada - mAtrasMapeada);

  // Calculo del dutyCycle de la señal PWM a enviar a los motores de la derecha
  int dutyCycleIzq = abs(mAdelanteMapeada - mAtrasMapeada);

  if (direccion > 2) {
    dutyCycleIzq = dutyCycleIzq - abs(direccion) * factorFreno;        // 30
    dutyCycleDer = dutyCycleDer + abs(direccion) * factorAceleracion;  // 10
    if (dutyCycleIzq < 0) {
      dutyCycleIzq = 0;
    }
  }
  
  if (direccion < -2) {
    dutyCycleDer = dutyCycleDer - abs(direccion) * factorFreno;
    dutyCycleIzq = dutyCycleIzq + abs(direccion) * factorAceleracion;
    if (dutyCycleDer < 0) {
      dutyCycleDer = 0;
    }
  }

  if (dutyCycleIzq > 700) {
    dutyCycleIzq = 700;
  }
  if (dutyCycleDer > 700) {
    dutyCycleDer = 700;
  }

  // Enviar señales PWM
  analogWrite(motorIzqPWM, dutyCycleIzq);
  analogWrite(motorDerPWM, dutyCycleDer);
}
