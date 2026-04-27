

//GRUPO 1 - 5MB: BAGDADI, PAVLICICH, MORENO, SAMA - 2025
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h> // Agregamos las llamadas a FreeRTOS primero
//IMPORTANTE : CORRER ESTO SÍ O SÍ CON ESP32 Espressif Systems v2.0.17
#include <AsyncTCP.h>
#include <AsyncMqttClient.h>
#include <WiFi.h>
#include <Wire.h>
#include "time.h"
//#include "Arduino.h"
#include <Preferences.h>
#include <Adafruit_AHT10.h>
#include <ListLib.h>
#include <LiquidCrystal_I2C.h>  //21: SDA || 22: SCL
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <string>
#include <iostream>

// --- LIBRERÍAS Y OBJETOS DEL GPS ---
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#define RXD2 16  // Pin RX2 del ESP32 (Conectar al TX del GPS)
#define TXD2 17  // Pin TX2 del ESP32 (Conectar al RX del GPS)
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
// -----------------------------------
#define IP1_MSB 192
#define IP2 168
#define IP3 1
#define IP4_LSB 105
//////wifi
const char* ssid = "Juanse";
const char* password = "SUMO2001";

bool sentState = false;
String lastState = "";

WiFiClientSecure client;

//☺//
TaskHandle_t LoopPantallas;
TaskHandle_t LoopMQTT;
//☺//


List<int> ajustesActuales;  //para imprimir pantalla secundaria (ajustes)
List<int> minimosAjustes;         //para imprimir pantalla principal (lecturas e informacion)
List<int> maximosAjustes;
//para imprimir pantalla secundaria (ajustes)

int sumaEstado = 0;

struct Valores {
  String ambiente;
  bool sensorExtra;
  float tempLeido;
  int humLeido;
  int luzLeido;
  int mq2Leido;
  int mq9Leido;
  int sonidoLeido;   // Agregado para el KY-037
  bool releEstado;
  float latitud;     // Agregado para el GPS
  float longitud;    // Agregado para el GPS
};
bool esPeligroso = false;
int GMT;
int humUmbral;
int tempUmbral;
int luzUmbral;
int mq2Umbral;
int mq9Umbral;
int mqttTiempoEnvio;
bool releActivo;

// Inicializamos la estructura con los nuevos valores de sonido y GPS en 0
Valores lecturas = { "PELIGRO", "HIGH", 11, 20, 5, 75, 65, 0, "OFF", 0.0, 0.0 };
const char* const INDICES[] = { "GMT", "humedad", "temp", "luz", "MQ2", "MQ9", "mqtt" };
Preferences prefs;

#define DIRECCION 0
int anteriorUmbral;
// Pines
#define BOTON_ARRIBA 26
#define BOTON_ABAJO 27
#define BOTON_IR 32
#define BOTON_VOLVER 33
#define BOTON_OK 25  //26

#define PULSADO LOW
#define N_PULSADO !PULSADO

#define LDR_PIN 39
#define RELE_PIN 4
#define MQ2_PIN 35
#define MQ9_PIN 34
#define SENSOR_EXTRA_PIN 24
#define KY037_PIN 36 // Pin para el micrófono KY-037 (ADC1 para evitar conflicto con Wi-Fi)
#define ERROR 505

#define LED_PELIGRO 14
#define LED_ALERTA 12
#define LED_NORMAL 13
#define NORMAL 0
#define PELIGRO 12
#define RANGO_TEMP 5
#define RANGO_LUZ 5
#define RANGO_HUM 5
#define RANGO_MQ2 5
#define RANGO_MQ9 5
#define ALERTA_TEMP 2
#define ALERTA_LUZ 2
#define ALERTA_HUM 2
#define ALERTA_MQ2 5
#define ALERTA_MQ9 5
#define PELIGRO_TEMP 5
#define PELIGRO_LUZ 5
#define PELIGRO_HUM 5
#define PELIGRO_MQ2 15
#define PELIGRO_MQ9 15

// Estados
#define MENU_PRINCIPAL 1     //IMPRIMIR VALORES LEIDOS
#define AJUSTES 2            //IMPRIMIR VALORES DE AJUSTES
#define CONFIRMACION 3  
     //ESPERAR A QUE NINGUN BOTON ESTE PRESIONADO ANTES DE CONTINUAR
#define MODIFICAR_AJUSTES 4  //MODIFICAR EL VALOR DE UNO DE LOS AJUSTES

#define MAX_PANTALLA 1
#define MIN_PANTALLA 0
#define SUBIR -1
#define BAJAR 1

//VALORES LEIDOS
#define MENU_PANTALLA1 0
#define MENU_PANTALLA2 1
//AJUSTES ACTUALES
#define AJUSTES_PANTALLA1 2
#define AJUSTES_PANTALLA2 3
#define AJUSTES_PANTALLA3 4
#define AJUSTES_PANTALLA4 5
//NUEVO VALOR DE AJUSTES + MAX/MIN
#define MODIFICAR_PANTALLA 6
//CANTIDAD DE PANTALLAS DE VALORES LEIDOS
#define MAX_MENU MENU_PANTALLA2
#define CHAR_SELECCION "*"

#define AUMENTAR 1
#define DISMINUIR -1

int estadoActual = MENU_PRINCIPAL;
int estadoDeRetorno = MENU_PRINCIPAL;
int pantallaElegida = MENU_PANTALLA1;
int ajusteAModificar = 0;

bool botonArriba = N_PULSADO;
bool botonAbajo = N_PULSADO;
bool botonOk = N_PULSADO;
bool botonIr = N_PULSADO;
bool botonVolver = N_PULSADO;

bool seleccionado = MIN_PANTALLA;
//VALORES PARA MODIFICAR AJUSTES
int nuevoValorAjuste = 0;
int maxAjuste = 0;
int minAjuste = 0;
int modificacionAjuste = 0;
unsigned long ultimaImpresion = 0;
#define T_DE_IMPRESION 3000
unsigned long ultimalectura = 0;
unsigned long ahora = 0;
#define DIRECCION_I2C 0x27
#define LARGO_PANTALLA 16
#define ALTO_PANTALLA 2

#define MAX_GMT 12
#define MIN_GMT -12
#define MAX_HUM 100
#define MIN_HUM 0
#define MAX_TEMP 50
#define MIN_TEMP -50
#define MAX_LUZ 100
#define MIN_LUZ 0
#define MAX_MQ2 100
#define MIN_MQ2 0
#define MAX_MQ9 100
#define MIN_MQ9 0
#define MAX_MQTT 300
#define MIN_MQTT 30
#define RELE_ON LOW
#define RELE_OFF !RELE_ON

#define MIN_LECTURA_MICRO 0
#define MAX_LECTURA_MICRO 4095

LiquidCrystal_I2C lcd(DIRECCION_I2C, LARGO_PANTALLA, ALTO_PANTALLA);
Adafruit_AHT10 aht;

// Declaración de funciones
void ImprimirPantallas();
int ModificarAjuste(int, int, int, int);
void LeerSensores();
void actualizarEEPROM(int, int);


const char name_device = 21;

unsigned long now = millis();
///valor actual
unsigned long lastMeasure1 = 0;  ///variable para contar el tiempo actual
unsigned long lastMeasure2 = 0;
///variable para contar el tiempo actual
int i = 0;

///time
long unsigned int timestamp;  // hora
const char* ntpServer = "south-america.pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

///variables ingresar a la cola struct
int indice_entra = 0;
int indice_saca = 0;
bool flag_vacio = 1;

/////mqqtt
//#define MQTT_HOST IPAddress(IP1_MSB, IP2, IP3, IP4_LSB)  ///se debe cambiar por el ip de meca o hall del 4
#define MQTT_HOST "broker.emqx.io" // Servidor público y gratuito (en vez de mandarlo a la computadora)
#define MQTT_PORT 1883
//#define MQTT_USERNAME "esp32"
//#define MQTT_PASSWORD "mirko15"
char mqtt_payload[200];
/////
// Test MQTT Topic
#define MQTT_PUB "/esp32/datos_sensores"
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
bool conexionWifiPendiente = false;
bool conexionMqttPendiente = false;
bool publicacionMqttPendiente = false;

typedef struct
{
  long time;
  float T1;
  ///temp en grados
  float H1;     ///valor entre 0 y 99 // mojado es cercano al 100
  float luz;
  ///valor entre 0 y 99 . si hay luz es cercano al 100
  float G1;
  ///valor entre 0 y 99
  float G2;     ///valor entre 0 y 99
  //bool oct;
  ///Lectura del octoacoplador
  //bool Alarma;  //
  float lat;
  float lon;

} estructura;
////////////////
const int valor_max_struct = 1000;          ///valor vector de struct
estructura datos_struct[valor_max_struct];
///Guardo valores hasta que lo pueda enviar
estructura aux2;

/////*********************************************************************/////
////////////////////////////setup wifi/////////////////////////////////////////
/////*********************************************************************/////
void setupmqtt() {
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  WiFi.onEvent(WiFiEvent);
  connectToWifi();
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  //mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
  
}



////////////////////////////Envio de datos mqtt//////////////////////////////////////////
////////Funcion que envia valores cuando la estructura no este vacia ///////////////////
///////////////////////////////////////////////////////////////////////////////////////


void fun_envio_mqtt() {
  fun_saca();
  ////veo si hay valores nuevos
  if (flag_vacio == 0)  ////si hay los envio
  {
    Serial.print("enviando");
    ////genero el string a enviar 1. 2.   3.   4.   5.   6.   7   8  9.       1.         2.        3.      4.       5.           6.   7.        8.        9
    /*snprintf(mqtt_payload, 200, "%u&%ld&%.2f&%.2f&%.2f&%.2f&%.2f&%.2f&%.2f", 
    name_device, aux2.time, aux2.T1, aux2.H1, aux2.luz, aux2.G1, aux2.G2, aux2.lat, aux2.lon);  //random(10,50)
    */
    snprintf(mqtt_payload, 250, "{\"dev\":%u, \"temp\":%.2f, \"hum\":%.2f, \"luz\":%.2f, \"mq2\":%.2f, \"mq9\":%.2f, \"lat\":%.6f, \"lon\":%.6f}", 
    name_device, aux2.T1, aux2.H1, aux2.luz, aux2.G1, aux2.G2, aux2.lat, aux2.lon); //cambié a formato JSON para poder levantarlo directamente desde MQTT
    aux2.time = 0;
    ///limpio valores
    aux2.T1 = 0;
    aux2.H1 = 0;
    aux2.luz = 0;
    aux2.G1 = 0;
    aux2.G2 = 0;
    aux2.lat = 0;
    aux2.lon = 0;

    Serial.print("Publish message: ");
    Serial.println(mqtt_payload);
    // Publishes Temperature and Humidity values
    publicacionMqttPendiente = true;
    //uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB, 1, true, mqtt_payload);
  } else {
    Serial.println("no hay valores nuevos");
  }
}  ///////////////////////////////////////////////////

///////////////////////////////////////////////////
void connectToWifi() {
  conexionWifiPendiente = true;
}  ///////////////////////////////////////////////////
void connectToMqtt() {
  conexionMqttPendiente = true;
}  ///////////////////////////////////////////////////

void publishToMqtt() {
  // Verificación de seguridad CRÍTICA
  if (mqttClient.connected()) {
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB, 1, true, mqtt_payload);
    Serial.println("Se realizó publicación a MQTT con ID:");
    Serial.println(packetIdPub1);
  } else {
    Serial.println("Operación abortada: MQTT desconectado.");
  }
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0);
      // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}  ///////////////////////////////////////////////////
////////////////NO TOCAR FUNCIONES MQTT///////////////////////////////////
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}  ///////////////////////////////////////////////////

////////////////NO TOCAR FUNCIONES MQTT///////////////////////////////////
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}
////////////////NO TOCAR FUNCIONES MQTT///////////////////////////////////
void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}  ///////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
/////////////Funcion que saca un valor de la estructura para enviar //////
///////////////////////////////////////////////////////////////////////
void fun_saca() {
  if (indice_saca != indice_entra) {
    aux2.time = datos_struct[indice_saca].time;
    aux2.T1 = datos_struct[indice_saca].T1;
    aux2.H1 = datos_struct[indice_saca].H1;
    aux2.luz = datos_struct[indice_saca].luz;
    aux2.G1 = datos_struct[indice_saca].G1;
    aux2.G2 = datos_struct[indice_saca].G2;
    aux2.lat = datos_struct[indice_saca].lat;
    aux2.lon = datos_struct[indice_saca].lon;

    flag_vacio = 0;

    Serial.println(indice_saca);
    if (indice_saca >= (valor_max_struct - 1)) {
      indice_saca = 0;
    } else {
      indice_saca++;
    }
    Serial.print("saco valores de la struct isaca:");
    Serial.println(indice_saca);
  } else {
    flag_vacio = 1;  ///// no hay datos
  }
  return;
}
/////////////////////////////////////////////////////////////////////
/////////////funcion que ingresa valores a la cola struct///////////
///////////////////////////////////////////////////////////////////
void fun_entra(void) {
  if (indice_entra >= valor_max_struct) {
    indice_entra = 0;
    ///si llego al maximo de la cola se vuelve a cero
  }
  //////////// timestamp/////// consigo la hora
  Serial.print("> NTP Time:");
  timestamp = time(NULL);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
    //// si no puede conseguir la hora
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  ///////////////////////// fin de consigo la hora
  datos_struct[indice_entra].time = timestamp;
  datos_struct[indice_entra].T1 = lecturas.tempLeido;
  /// leeo los datos //aca va la funcion de cada sensor
  datos_struct[indice_entra].H1 = lecturas.humLeido;
  //// se puede pasar por un parametro valor entre 0 y 100
  datos_struct[indice_entra].luz = lecturas.luzLeido;
  datos_struct[indice_entra].G1 = lecturas.mq2Leido;
  datos_struct[indice_entra].G2 = lecturas.mq9Leido;
  datos_struct[indice_entra].lat = lecturas.latitud;
  datos_struct[indice_entra].lon = lecturas.longitud;

  indice_entra++;
  Serial.print("saco valores de la struct ientra");
  Serial.println(indice_entra);
}
////////////////////////////////////////////////////////////////////
/////////////SETUP/////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // --- INICIO DEL GPS ---
  gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println(F("Iniciando GPS u-blox NEO-6M..."));
  // ----------------------

  minimosAjustes.Add(MIN_GMT);
  minimosAjustes.Add(MIN_HUM);
  minimosAjustes.Add(MIN_TEMP);
  minimosAjustes.Add(MIN_LUZ);
  minimosAjustes.Add(MIN_MQ2);
  minimosAjustes.Add(MIN_MQ9);
  minimosAjustes.Add(MIN_MQTT);
  minimosAjustes.Add(RELE_OFF);

  maximosAjustes.Add(MAX_GMT);
  maximosAjustes.Add(MAX_HUM);
  maximosAjustes.Add(MAX_TEMP);
  maximosAjustes.Add(MAX_LUZ);
  maximosAjustes.Add(MAX_MQ2);
  maximosAjustes.Add(MAX_MQ9);
  maximosAjustes.Add(MAX_MQTT);
  maximosAjustes.Add(RELE_ON);

  ajustesActuales.Add(GMT);
  ajustesActuales.Add(humUmbral);
  ajustesActuales.Add(tempUmbral);
  ajustesActuales.Add(luzUmbral);
  ajustesActuales.Add(mq2Umbral);
  ajustesActuales.Add(mq9Umbral);
  ajustesActuales.Add(mqttTiempoEnvio);
  ajustesActuales.Add(releActivo);

  pinMode(BOTON_ARRIBA, INPUT_PULLUP);
  pinMode(BOTON_ABAJO, INPUT_PULLUP);
  pinMode(BOTON_OK, INPUT_PULLUP);
  pinMode(BOTON_IR, INPUT_PULLUP);
  pinMode(BOTON_VOLVER, INPUT_PULLUP);

  setupmqtt();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(100);

  lcd.init();
  lcd.backlight();
  /////declaro pines digitales

  //Setup de time
  prefs.begin("config", true);
  GMT = prefs.getInt("GMT", 0);
  humUmbral = prefs.getInt("humedad", 20);
  tempUmbral = prefs.getInt("temp", 20);
  luzUmbral = prefs.getInt("luz", 50);
  mq2Umbral = prefs.getInt("MQ2", 15);
  mq9Umbral = prefs.getInt("MQ9", 10);
  mqttTiempoEnvio = prefs.getInt("mqtt", 60);
  prefs.end();

  Serial.println("Valores leídos de EEPROM: ");
  Serial.print("GMT: ");
  Serial.print(GMT);
  Serial.print(" || ");
  Serial.print("hum: ");
  Serial.print(humUmbral);
  Serial.print(" || ");
  Serial.print("temp: ");
  Serial.print(tempUmbral);
  Serial.print(" || ");
  Serial.print("luz: ");
  Serial.print(luzUmbral);
  Serial.print(" || ");
  Serial.print("MQ2: ");
  Serial.print(mq2Umbral);
  Serial.print(" || ");
  Serial.print("MQ9: ");
  Serial.print(mq9Umbral);
  Serial.print(" || ");
  Serial.print("MQTT: ");
  Serial.print(mqttTiempoEnvio);
  Serial.println(" || ");

  Wire.begin();  // Inicialización I2C
  ///  WiFi.mode(WIFI_STA);
  //  WiFi.begin(ssid, password);

  // Serial.println("Conectado a WiFi con IP:");
  // Serial.println(WiFi.localIP());


//#ifdef ESP32
  //client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
//#endif

  xTaskCreatePinnedToCore(LoopPantallascode, "Ejecutar máquina de estados", 10000, NULL, 1, &LoopPantallas, 0);
  xTaskCreatePinnedToCore(LoopMQTTcode, "Correr Bot de Telegram", 10000, NULL, 1, &LoopMQTT, 1);

  LeerSensores();
  ultimalectura = millis();
}
////////////////////////////////////////////////////////////////////
/////////////LOOP DE PANTALLAS/////////////////////////////////////
////////////////////////////////////////////////////////////////////
void LoopPantallascode(void* pvParameters) {
  for (;;) {

    botonArriba = digitalRead(BOTON_ARRIBA);
    botonAbajo = digitalRead(BOTON_ABAJO);
    botonOk = digitalRead(BOTON_OK);
    botonIr = digitalRead(BOTON_IR);
    botonVolver = digitalRead(BOTON_VOLVER);

    ahora = millis();
    if (lecturas.ambiente == "NORMAL") {
      digitalWrite(LED_PELIGRO, LOW);
      digitalWrite(LED_ALERTA, LOW);
      digitalWrite(LED_NORMAL, HIGH);
    } else if (lecturas.ambiente == "ALERTA") {
      digitalWrite(LED_PELIGRO, LOW);
      digitalWrite(LED_ALERTA, HIGH);
      digitalWrite(LED_NORMAL, LOW);
    } else if (lecturas.ambiente == "PELIGRO") {
      digitalWrite(LED_PELIGRO, HIGH);
      digitalWrite(LED_ALERTA, LOW);
      digitalWrite(LED_NORMAL, LOW);
    } else {
      Serial.println("ERROR: HA HABIDO UNA FALLA IMPREVISTA");
      digitalWrite(LED_PELIGRO, HIGH);
      digitalWrite(LED_ALERTA, HIGH);
      digitalWrite(LED_PELIGRO, HIGH);
    }
    if (ahora - ultimaImpresion >= T_DE_IMPRESION) {
      ImprimirPantallas();
      ultimaImpresion = millis();
    }

    switch (estadoActual) {
      case MENU_PRINCIPAL:
        if (ahora - ultimalectura >= (mqttTiempoEnvio * 1000)) {
          LeerSensores();
          ultimalectura = ahora;
        }

        if ((botonArriba == PULSADO) || (botonAbajo == PULSADO)) {
          if (pantallaElegida == MENU_PANTALLA1) {
            pantallaElegida = MAX_MENU;
          } else {
            pantallaElegida = MENU_PANTALLA1;
          }
          estadoActual = CONFIRMACION;
        }
        if (botonIr == PULSADO) {
          pantallaElegida = AJUSTES_PANTALLA1;
          estadoActual = CONFIRMACION;
          estadoDeRetorno = AJUSTES;
          seleccionado = MIN_PANTALLA;
        }
        break;
      case AJUSTES:
        if (botonArriba == PULSADO) {
          if (seleccionado > MIN_PANTALLA) {
            seleccionado = seleccionado + SUBIR;
          } else {
            seleccionado = MAX_PANTALLA;
            if (pantallaElegida > AJUSTES_PANTALLA1) {
              pantallaElegida = pantallaElegida + SUBIR;
            } else {
              pantallaElegida = AJUSTES_PANTALLA4;
            }
          }
        }
        estadoActual = CONFIRMACION;
        if (botonAbajo == PULSADO) {
          if (seleccionado < MAX_PANTALLA) {
            seleccionado = seleccionado + BAJAR;
          } else {
            seleccionado = MIN_PANTALLA;
            pantallaElegida = pantallaElegida + BAJAR;
          }
          estadoActual = CONFIRMACION;
        }

        if ((botonIr == PULSADO) || (botonOk == PULSADO)) {
          estadoActual = CONFIRMACION;
          estadoDeRetorno = MODIFICAR_AJUSTES;
          ajusteAModificar = ((pantallaElegida - MAX_MENU - 1) * (MAX_PANTALLA + 1)) + seleccionado;
          nuevoValorAjuste = ajustesActuales[ajusteAModificar];
          maxAjuste = maximosAjustes[ajusteAModificar];
          minAjuste = minimosAjustes[ajusteAModificar];
          pantallaElegida = MODIFICAR_PANTALLA;
        }
        if (botonVolver == PULSADO) {
          estadoDeRetorno = MENU_PRINCIPAL;
          estadoActual = CONFIRMACION;
          pantallaElegida = MENU_PANTALLA1;
        }
        break;
      case CONFIRMACION:
        delay(10);
        if ((botonArriba == N_PULSADO) && (botonAbajo == N_PULSADO) && (botonOk == N_PULSADO) && (botonIr == N_PULSADO) && (botonVolver == N_PULSADO)) {
          estadoActual = estadoDeRetorno;
        }
        break;
      case MODIFICAR_AJUSTES:
        if (botonArriba == PULSADO) {
          modificacionAjuste = AUMENTAR;
          if (ajusteAModificar == 6) {
            modificacionAjuste = 30;
          }
          nuevoValorAjuste = ModificarAjuste(minAjuste, maxAjuste, nuevoValorAjuste, modificacionAjuste);
          estadoActual = CONFIRMACION;
        }

        if (botonAbajo == PULSADO) {
          modificacionAjuste = DISMINUIR;
          if (ajusteAModificar == 6) {
            modificacionAjuste = -30;
          }
          nuevoValorAjuste = ModificarAjuste(minAjuste, maxAjuste, nuevoValorAjuste, modificacionAjuste);
          estadoActual = CONFIRMACION;
        }

        if (botonVolver == PULSADO) {
          estadoDeRetorno = AJUSTES;
          estadoActual = CONFIRMACION;
          pantallaElegida = AJUSTES_PANTALLA1;
        }
        if (botonOk == PULSADO) {
          pantallaElegida = AJUSTES_PANTALLA1;
          estadoActual = CONFIRMACION;
          estadoDeRetorno = AJUSTES;
          seleccionado = MIN_PANTALLA;

          if (ajusteAModificar == 0) {
            if (GMT != nuevoValorAjuste) {
              GMT = nuevoValorAjuste;
              actualizarEEPROM(ajusteAModificar, GMT);
            }
          }
          if (ajusteAModificar == 1) {
            if (humUmbral != nuevoValorAjuste) {
              humUmbral = nuevoValorAjuste;
              actualizarEEPROM(ajusteAModificar, humUmbral);
            }
          }
          if (ajusteAModificar == 2) {
            if (tempUmbral != nuevoValorAjuste) {
              tempUmbral = nuevoValorAjuste;
              actualizarEEPROM(ajusteAModificar, tempUmbral);
            }
          }
          if (ajusteAModificar == 3) {
            if (luzUmbral != nuevoValorAjuste) {
              luzUmbral = nuevoValorAjuste;
              actualizarEEPROM(ajusteAModificar, luzUmbral);
            }
          }
          if (ajusteAModificar == 4) {
            if (mq2Umbral != nuevoValorAjuste) {
              mq2Umbral = nuevoValorAjuste;
              actualizarEEPROM(ajusteAModificar, mq2Umbral);
            }
          }
          if (ajusteAModificar == 5) {
            if (mq9Umbral != nuevoValorAjuste) {
              mq9Umbral = nuevoValorAjuste;
              actualizarEEPROM(ajusteAModificar, mq9Umbral);
            }
          }
          if (ajusteAModificar == 6) {
            if (mqttTiempoEnvio != nuevoValorAjuste) {
              mqttTiempoEnvio = nuevoValorAjuste;
              actualizarEEPROM(ajusteAModificar, mqttTiempoEnvio);
            }
          }
          if (ajusteAModificar == 7) {
            releActivo = nuevoValorAjuste;
            if (releActivo == RELE_OFF) {
              lecturas.releEstado = "OFF";
            } else {
              lecturas.releEstado = "ON";
            }
            digitalWrite(releActivo, RELE_OFF);
          }
          break;
        }
    }
  }
}
void actualizarEEPROM(int indice, int dato) {
  Serial.println("función actualizar eeprom");
  prefs.begin("config", false);
  switch (indice) {
    case 0:
      prefs.putInt(INDICES[indice], dato);
      Serial.println("GMT Actualizado");
      break;
    case 1:
      prefs.putInt(INDICES[indice], dato);
      Serial.println("Humedad Actualizado");
      break;
    case 2:
      prefs.putInt(INDICES[indice], dato);
      Serial.println("Temperatura Actualizado");
      break;
    case 3:
      prefs.putInt(INDICES[indice], dato);
      Serial.println("Luz Actualizado");
      break;
    case 4:
      prefs.putInt(INDICES[indice], dato);
      Serial.println("MQ2 Actualizado");
      break;
    case 5:
      prefs.putInt(INDICES[indice], dato);
      Serial.println("MQ9 Actualizado");
      break;
    case 6:
      prefs.putInt(INDICES[indice], dato);
      Serial.println("mqtt Actualizado");
      break;
  }
  return;
}

void ImprimirPantallas() {
  lcd.clear();
  if (pantallaElegida == MENU_PANTALLA1) {
    lcd.setCursor(0, MIN_PANTALLA);
    lcd.print("E:");
    lcd.setCursor(2, MIN_PANTALLA);
    lcd.print(lecturas.ambiente);
    lcd.setCursor(11, MIN_PANTALLA);
    lcd.print("R:");
    lcd.setCursor(13, MIN_PANTALLA);
    lcd.print(lecturas.releEstado);
    lcd.setCursor(2, MAX_PANTALLA);
    lcd.print(lecturas.tempLeido);
    lcd.setCursor(0, MAX_PANTALLA);
    lcd.print("T:");
    lcd.setCursor(7, MAX_PANTALLA);
    lcd.print("C");
    lcd.setCursor(9, MAX_PANTALLA);
    lcd.print("GAS:");
    lcd.setCursor(13, MAX_PANTALLA);
    lcd.print(lecturas.mq2Leido);
  }
  if (pantallaElegida == MENU_PANTALLA2) {
    lcd.setCursor(0, MIN_PANTALLA);
    lcd.print("PIN:");
    lcd.setCursor(4, MIN_PANTALLA);
    lcd.print(lecturas.sensorExtra);
    lcd.setCursor(9, MIN_PANTALLA);
    lcd.print("LUZ:");
    lcd.setCursor(13, MIN_PANTALLA);
    lcd.print(lecturas.luzLeido);
    lcd.setCursor(0, MAX_PANTALLA);
    lcd.print("HUM:");
    lcd.setCursor(4, MAX_PANTALLA);
    lcd.print(lecturas.humLeido);
    lcd.setCursor(9, MAX_PANTALLA);
    lcd.print("GAS:");
    lcd.setCursor(13, MAX_PANTALLA);
    lcd.print(lecturas.mq9Leido);
  }

  if ((pantallaElegida > MAX_MENU) && (pantallaElegida < MODIFICAR_PANTALLA)) {
    lcd.setCursor(0, seleccionado);
    lcd.print(CHAR_SELECCION);
  }
  if (pantallaElegida == AJUSTES_PANTALLA1) {
    lcd.setCursor(1, MIN_PANTALLA);
    lcd.print("GMT:");
    lcd.setCursor(5, MIN_PANTALLA);
    lcd.print(GMT);
    lcd.setCursor(1, MAX_PANTALLA);
    lcd.print("HUM UMBRAL:");
    lcd.setCursor(12, MAX_PANTALLA);
    lcd.print(humUmbral);
  }
  if (pantallaElegida == AJUSTES_PANTALLA2) {
    lcd.setCursor(1, MIN_PANTALLA);
    lcd.print("T UMBRAL:");
    lcd.setCursor(10, MIN_PANTALLA);
    lcd.print(tempUmbral);
    lcd.setCursor(1, MAX_PANTALLA);
    lcd.print("LUZ UMBRAL:");
    lcd.setCursor(12, MAX_PANTALLA);
    lcd.print(luzUmbral);
  }
  if (pantallaElegida == AJUSTES_PANTALLA3) {
    lcd.setCursor(1, MIN_PANTALLA);
    lcd.print("MQ2 UMBRAL:");
    lcd.setCursor(12, MIN_PANTALLA);
    lcd.print(mq2Umbral);
    lcd.setCursor(1, MAX_PANTALLA);
    lcd.print("MQ9 UMBRAL:");
    lcd.setCursor(12, MAX_PANTALLA);
    lcd.print(mq9Umbral);
  }
  if (pantallaElegida == AJUSTES_PANTALLA4) {
    lcd.setCursor(1, MIN_PANTALLA);
    lcd.print("MQTT ENVIO:");
    lcd.setCursor(12, MIN_PANTALLA);
    lcd.print(mqttTiempoEnvio);
    lcd.setCursor(1, MAX_PANTALLA);
    lcd.print("RELE:");
    lcd.setCursor(6, MAX_PANTALLA);
    lcd.print(releActivo);
  }
  if (pantallaElegida == MODIFICAR_PANTALLA) {
    lcd.setCursor(0, MIN_PANTALLA);
    lcd.print("NUEVO VALOR:");
    lcd.setCursor(12, MIN_PANTALLA);
    lcd.print(nuevoValorAjuste);
    lcd.setCursor(0, MAX_PANTALLA);
    lcd.print("MIN:");
    lcd.setCursor(4, MAX_PANTALLA);
    lcd.print(minAjuste);
    lcd.setCursor(8, MAX_PANTALLA);
    lcd.print("MAX:");
    lcd.setCursor(12, MAX_PANTALLA);
    lcd.print(maxAjuste);
  }
  return;
}

int ModificarAjuste(int minimo, int maximo, int valorAnterior, int modificacion) {
  int nuevovalor = valorAnterior + modificacion;
  if (nuevovalor < minimo) {
    nuevovalor = maximo;
  }
  if (nuevovalor > maximo) {
    nuevovalor = minimo;
  }
  return nuevovalor;
}

void LeerSensores() {
  Serial.println("leer:");
  sumaEstado = 0;
  int intentoConectar = 0;
  
  if (aht.begin()) {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    Serial.print("Temperature: ");
    Serial.print(temp.temperature);
    lecturas.tempLeido = temp.temperature;
    Serial.println(" degrees C");
    Serial.print("Humidity: ");
    Serial.print(humidity.relative_humidity);
    Serial.println("% rH");
    lecturas.humLeido = humidity.relative_humidity;
  }
  if (humUmbral < lecturas.humLeido) {
    if ((humUmbral - lecturas.humLeido) < RANGO_HUM) {
      sumaEstado = sumaEstado + ALERTA_HUM;
    } else {
      sumaEstado = sumaEstado + PELIGRO_HUM;
    }
  }
  if (tempUmbral < lecturas.tempLeido) {
    if ((tempUmbral - lecturas.tempLeido) < RANGO_TEMP) {
      sumaEstado = sumaEstado + ALERTA_TEMP;
    } else {
      sumaEstado = sumaEstado + PELIGRO_TEMP;
    }
  }
  int ldrRead = analogRead(LDR_PIN);
  Serial.print("LDR: ");
  Serial.println(ldrRead);
  lecturas.luzLeido = map(ldrRead, MIN_LECTURA_MICRO, MAX_LECTURA_MICRO, MIN_LUZ, MAX_LUZ);
  if (luzUmbral < lecturas.luzLeido) {
    if ((luzUmbral - lecturas.luzLeido) < RANGO_LUZ) {
      sumaEstado = sumaEstado + ALERTA_LUZ;
    } else {
      sumaEstado = sumaEstado + PELIGRO_LUZ;
    }
  }

  int mq2Read = analogRead(MQ2_PIN);
  Serial.print("MQ2: ");
  Serial.println(mq2Read);
  lecturas.mq2Leido = map(mq2Read, MIN_LECTURA_MICRO, MAX_LECTURA_MICRO, MIN_MQ2, MAX_MQ2);
  if (mq2Umbral < lecturas.luzLeido) {
    if ((mq2Umbral - lecturas.mq2Leido) < RANGO_MQ2) {
      sumaEstado = sumaEstado + ALERTA_MQ2;
    } else {
      sumaEstado = sumaEstado + PELIGRO_MQ2;
    }
  }

  int mq9Read = analogRead(MQ9_PIN);
  Serial.print("MQ9: ");
  Serial.println(mq9Read);
  lecturas.mq9Leido = map(mq9Read, MIN_LECTURA_MICRO, MAX_LECTURA_MICRO, MIN_MQ9, MAX_MQ9);
  if (mq9Umbral < lecturas.mq9Leido) {
    if ((mq9Umbral - lecturas.mq9Leido) < RANGO_MQ9) {
      sumaEstado = sumaEstado + ALERTA_MQ9;
    } else {
      sumaEstado = sumaEstado + PELIGRO_MQ9;
    }
  }

  // --- LECTURA DEL MICRÓFONO KY-037 ---
  int sonidoRead = analogRead(KY037_PIN);
  lecturas.sonidoLeido = map(sonidoRead, MIN_LECTURA_MICRO, MAX_LECTURA_MICRO, 0, 100);
  Serial.print("Sonido: ");
  Serial.println(lecturas.sonidoLeido);
  // ------------------------------------

  int sensorExtraRead = digitalRead(SENSOR_EXTRA_PIN);
  if (sensorExtraRead == HIGH) {
    lecturas.sensorExtra = "HIGH";
  } else {
    lecturas.sensorExtra = "LOW";
  }
  if (sumaEstado == NORMAL) {
    lecturas.ambiente = "NORMAL";
  } else if ((sumaEstado > NORMAL) && (sumaEstado <= PELIGRO)) {
    lecturas.ambiente = "ALERTA";
  } else if (sumaEstado > PELIGRO) {
    lecturas.ambiente = "PELIGRO";
    esPeligroso = true;
  }
  while(gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      
      // Guardamos los datos en tu estructura global
      if (gps.location.isValid()) {
        lecturas.latitud = gps.location.lat();
        lecturas.longitud = gps.location.lng();
        
        // Descomenta la siguiente línea si quieres que se imprima la ubicación en el Monitor Serial para verificar
        // Serial.printf("Lat: %f, Lng: %f\n", lecturas.latitud, lecturas.longitud);
      }
    }
  }

  return;
}

void LoopMQTTcode(void* pvParameters) {
  for (;;) {
    now = millis();
    if (now - lastMeasure1 > (mqttTiempoEnvio*500)) {  ////envio el doble de lectura por si falla algun envio
      lastMeasure1 = now;
      /// cargo el valor actual de millis
      fun_envio_mqtt();
      ///envio los valores por mqtt
    }
    if (now - lastMeasure2 > ((mqttTiempoEnvio*1000))) {
      lastMeasure2 = now;
      /// cargo el valor actual de millis
      fun_entra();
      ///ingreso los valores a la cola struct
    }
    if (lecturas.ambiente != lastState) {
      sentState = false;
      lastState = lecturas.ambiente;
    }
  }
}

// --- LÓGICA DEL GPS EN EL LOOP PRINCIPAL ---
void loop() {
  if (publicacionMqttPendiente){
    publishToMqtt();
    publicacionMqttPendiente = false;
  }
  if (conexionWifiPendiente){
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    conexionWifiPendiente = false;
  }

  if (conexionMqttPendiente){
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
    conexionMqttPendiente = false;
  }

}