//!  Código de control del Simulador Clínico Bebé
/*!
Este código controla el NodeMCU 32S del Simulador Clínico Bebé del Laboratorio de Robótica Educativa de la Uiversidad Nacional de Tucumán
*/

/** @file NodeMCU-32S.cpp
 *  @version 1.0
 *  @date 10/08/2021
 *  @author Gustavo Sosa
 *  @brief Código de implementación para SCB
 */
 
#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <Stepper.h>

//#include <RGBLed.h>

// setting PWM properties
const int freq = 500;
const int ledChannelR = 0; //Para pin 25. Ver pinout.
const int ledChannelG = 1; //Para pin 26. Ver pinout.
const int ledChannelB = 2; //Para pin 27. Ver pinout.
const int resolution = 8;

float brillo = 0.8; //Elegir un valor entre 0 y 1. Actualmente no se esta usando.

char horizontal[]="------------------------------";

#define TRUE 1
#define FALSE 0

//**************************************
//*********** MQTT CONFIG **************
//**************************************
const char *mqtt_server = "ioticos.org";
const int mqtt_port = 1883;
const char *mqtt_user = "83dFsL3tRLFvHEQ";
const char *mqtt_pass = "AcvmIul5coPyvHv";
const char *root_topic_subscribe = "BQzOw1AzvrkaX2P/input";
const char *root_topic_publish = "BQzOw1AzvrkaX2P/output";


//**************************************
//*********** WIFICONFIG ***************
//**************************************
char* ssid = "Redmi 9";
const char* password =  "gusta123";



//**************************************
//*********** GLOBALES   ***************
//**************************************
WiFiClient espClient;
PubSubClient client(espClient);
char msg[25];
long count=0;


//************************
//** F U N C I O N E S ***
//************************
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void setup_wifi();

//************************
//** V A R I A B L E S ***
//************************
bool flag_connected = TRUE;
const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution

int set_ppm = 100;     //Valor inicial de pulsos por minuto
int coloracion = 1;   //Valor inicial de coloración: 0=normal
int pin_manual = 34; //PIN de lectura análógica para la coloración

long int analog_lm35;
float temperatura;

int pin_salida = 13; //PIN de salida para celda peltier
int dutyTemp = 650;

int setTemperatura = 35;

// initialize the stepper library on pins from NodeMCU-32S:
Stepper myStepper(stepsPerRevolution, 4, 0, 2, 15);


/*************************** INICIO FUNCION SETUP **************************************************/
void setup() {
    // configure LED PWM functionalitites
  ledcSetup(ledChannelR, freq, resolution); //configuración PWM de COLOR R
  ledcSetup(ledChannelG, freq, resolution); //configuración PWM de COLOR G
  ledcSetup(ledChannelB, freq, resolution); //configuración PWM de COLOR B
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(25, ledChannelR); //pin de COLOR R
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(26, ledChannelG); //pin de COLOR G
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(27, ledChannelB); //pin de COLOR B
  
  Serial.println("Configuración de salidas PWM para RGB:");
  Serial.println("\tR: Pin 25. Canal 0. G: Pin 26. Canal 1. B: Pin 27. Canal 2. ");
  Serial.println("Probando Colores:");
  //Color de Prueba
  Serial.println("Rojo:");
  ledcWrite(ledChannelR, 100); //Logica invertida. 255 = color apagado. 0 = color intesidad maxima.
  ledcWrite(ledChannelG, 255);
  ledcWrite(ledChannelB, 255);
  delay(250);
  //Color de Prueba
  Serial.println("Verde:");
  ledcWrite(ledChannelR, 255);
  ledcWrite(ledChannelG, 100);
  ledcWrite(ledChannelB, 255);
  delay(250);
  //Color de Prueba
  Serial.println("Azul:");
  ledcWrite(ledChannelR, 255);
  ledcWrite(ledChannelG, 255);
  ledcWrite(ledChannelB, 100);
  delay(250);
  //Color de Prueba
  Serial.println("Cianosis:");
  ledcWrite(ledChannelR, 50);
  ledcWrite(ledChannelG, 255);
  ledcWrite(ledChannelB, 60);
  delay(500);
  //Color de Prueba
  Serial.println("Ictericia:");
  ledcWrite(ledChannelR, 50);
  ledcWrite(ledChannelG, 100);
  ledcWrite(ledChannelB, 255);
  delay(500);
  
  pinMode(pin_manual,INPUT);
  pinMode(pin_salida,OUTPUT);
  
  Serial.begin(115200); //Puerto de comunicación con el host
  setup_wifi();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

/*************************** MODULO DE CONFIGURACION AL INICIO. RESETEAR PARA CONFIGURAR. **************************************************/
/************** COLORACION **************************************************/
Serial.println("Ingrese el caso de coloración: 1. Sano - 2. Ictericia - 3. Cianosis");
while(!Serial.available()){} //Espero hasta que ingrese un caracter
  char data = Serial.read();
  while (Serial.available()>0)  Serial.read();
  if (data >= '1' && data <= '3')
    {
      char strSetup[40];
      sprintf(strSetup, "Valor ingresado: %c", data);
      Serial.println(strSetup);
      coloracion = data - '0'; //Convertido de caracter a entero
    }
  else Serial.println("Dato mal ingresado. Reiniciar para configurar de nuevo.");

/************** VELOCIDAD DEL PULSO **************************************************/

/*
while (Serial.available()>0)  Serial.read();
Serial.println("Ingrese la velocidad en pulsos por minuto (ppm):");
while(!Serial.available()){} //Espero hasta que ingrese un caracter
while(Serial.available()) {
     character = Serial.read();
     content.concat(character);
}
  //int input[2];
  //int input2 = Serial.read();
  if (content != "") {
     Serial.println(content);
}
  char strSetup[40];
      sprintf(strSetup, "Valor ingresado: %c%c", input[0],input[1]);
      Serial.println(strSetup);
  while (Serial.available()>0)  Serial.read();*/
  /*int ppmNum = (input1 - '0')*10 + (input2 - '0');
  if (ppmNum >= 10 && ppmNum <= 99)
    {
      
      set_ppm = ppmNum;
    }
  else Serial.println("Dato mal ingresado. Reiniciar para configurar de nuevo.");*/

/************** TEMPERATURA **************************************************/
/*Serial.println("Ingrese el caso de coloración: 1. Sano - 2. Ictericia - 3. Cianosis");
while(!Serial.available()){} //Espero hasta que ingrese un caracter
  char data = Serial.read();
  if (data >= '1' && data <= '3')
    {
      char strSetup[40];
      sprintf(strSetup, "Valor ingresado: %c", data);
      Serial.println(strSetup);
      coloracion = data - '0'; //Convertido de caracter a entero
    }
  else Serial.println("Dato mal ingresado. Reiniciar para configurar de nuevo.");
*/
/*************************** FIN FUNCION SETUP **************************************************/
}
/*************************** INICIO FUNCION LOOP **************************************************/
void loop() {
  
  if (!client.connected()) {
    reconnect();
    flag_connected = FALSE;
  }
  
  if (client.connected()){
    /* Confirmación de conexión */
    if(flag_connected){
      Serial.println("Conectado");
      String str = "Conectado";
      str.toCharArray(msg,40);
      client.publish(root_topic_publish,msg);
    }
    
    pulso(set_ppm); //****da una vuelta a la velocidad set_ppm
    
    temperatura = 0;
    for(int i=0; i<10; i++){
      analog_lm35 = analogRead(pin_manual);
      temperatura = temperatura + (analog_lm35 * 5 * 100) / 4095;      //CALIBRAR
      delay(10);// delay entre las mediciones del LM35
      }
      
    temperatura = temperatura/10;
    char str2[40];
    Serial.println(horizontal);
    sprintf(str2, "Analog read: %d", analog_lm35);
    Serial.println(str2);
    Serial.println(horizontal);
    sprintf(str2, "Temperatura leída: %.2f", temperatura);
    Serial.println(str2);

/*------------------ Sistema de control de celda peltier --------------------*/
    if(temperatura<setTemperatura){
      digitalWrite(pin_salida, HIGH);
      delay(dutyTemp);
      digitalWrite(pin_salida, LOW);
      //delay(100-dutyTemp);
      Serial.println(horizontal);
      Serial.println("Calentando");
    }
    
    sprintf(str2, "Temperatura seteada: %.2f", setTemperatura);
    Serial.println(horizontal);
    Serial.println(str2);
    
    if(temperatura>1.2*setTemperatura){
      Serial.println(horizontal);
      Serial.println("Sobrecalentado 20%");
    }
    if(temperatura>=setTemperatura && temperatura<1.2*setTemperatura){
      Serial.println(horizontal);
      Serial.println("Temperatura en rango");
    }
    
    //str.toCharArray(msg,40);
    //client.publish(root_topic_publish,str2);
    //delay(50);
    //led.setColor(manual_red, 0, 0); //seteamos el color neutral
    switch(coloracion){
      case 1: //seteamos el color rojo
              Serial.println(horizontal);
              Serial.println("Color: Sano");
              ledcWrite(ledChannelR, 255);
              ledcWrite(ledChannelG, 255);
              ledcWrite(ledChannelB, 255);
              break;
              
      case 2: //seteamos el color amarillo
              Serial.println(horizontal);
              Serial.println("Color: Ictericia");
              ledcWrite(ledChannelR, 50);
              ledcWrite(ledChannelG, 100);
              ledcWrite(ledChannelB, 255);
              break;
      case 3: //seteamos el color violeta
              Serial.println(horizontal);
              Serial.println("Color: Cianosis");
              Serial.println(horizontal);
              ledcWrite(ledChannelR, 200);
              ledcWrite(ledChannelG, 255);
              ledcWrite(ledChannelB, 80);
              break;
      default:
      break;
    }
    //delay(100);
    
  }
  client.loop();
}
/*************************** FIN FUNCION LOOP **************************************************/

/*************************** INICIO FUNCION PULSO **************************************************/
bool pulso(int ppm)
{
    // set the speed at 60 rpm:
    myStepper.setSpeed(ppm/2);
    myStepper.step(stepsPerRevolution);
    char str[40];
    sprintf(str, "Pulso cardiaco a %d ppm", ppm);
    Serial.println(horizontal);
    Serial.println(str);
    //str.toCharArray(msg,40);
    client.publish(root_topic_publish,str);
    delay(50);
}

/*************************** INICIO FUNCION PULSO **************************************************/

//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi(){
  delay(10);
  // Nos conectamos a nuestra red Wifi
  Serial.println();
  Serial.print("Conectando a ssid: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a red WiFi!");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}



//*****************************
//***    CONEXION MQTT      ***
//*****************************

void reconnect() {

  while (!client.connected()) {
    Serial.print("Intentando conexión Mqtt...");
    // Creamos un cliente ID
    String clientId = "IOTICOS_H_W_";
    clientId += String(random(0xffff), HEX);
    // Intentamos conectar
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("Conectado!");
      // Nos suscribimos
      if(client.subscribe(root_topic_subscribe)){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("fallo Suscripciión");
      }
    } else {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");
      delay(5000);
    }
  }
}


//*****************************
//***       CALLBACK        ***
//*****************************

void callback(char* topic, byte* payload, unsigned int length){
  String incoming = "";
  Serial.print("Mensaje recibido desde -> ");
  Serial.print(topic);
  Serial.println("");
  for (int i = 0; i < length; i++) {
    incoming += (char)payload[i];
  }
  incoming.trim();
  Serial.println("Mensaje -> " + incoming);

}
