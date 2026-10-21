#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>

const char* SSID     = "Wokwi-GUEST";
const char* PASSWORD = "";
const char* BROKER_MQTT = "3.135.234.101";
const int   BROKER_PORT = 1883;

const char* DEVICE = "device010";
const char* ID_MQTT = "fiware_010";
const char* TOPICO_SUBSCRIBE = "/TEF/device010/cmd";
const char* TOPICO_ATTRS     = "/TEF/device010/attrs";
const char* TOPICO_CMDEXE    = "/TEF/device010/cmdexe";

#define PINO_LED 2
#define PINO_LDR 34
#define PINO_DHT 15

DHT dht(PINO_DHT, DHT22);
WiFiClient espClient;
PubSubClient MQTT(espClient);

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 5000;

String timestampISO() {
  struct tm t;
  if (!getLocalTime(&t, 2000)) return "";
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S-03:00", &t);
  return String(buf);
}

void reconectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(SSID, PASSWORD, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
}

void publicaEstadoLed(bool ligado) {
  MQTT.publish(TOPICO_ATTRS, ligado ? "s|on" : "s|off");
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.print("- Mensagem recebida: ");
  Serial.println(msg);

  String onCmd  = String(DEVICE) + "@on|";
  String offCmd = String(DEVICE) + "@off|";

  if (msg.equals(onCmd)) {
    digitalWrite(PINO_LED, HIGH);
    publicaEstadoLed(true);
    MQTT.publish(TOPICO_CMDEXE, (String(DEVICE) + "@on|LED ligado").c_str());
  } else if (msg.equals(offCmd)) {
    digitalWrite(PINO_LED, LOW);
    publicaEstadoLed(false);
    MQTT.publish(TOPICO_CMDEXE, (String(DEVICE) + "@off|LED desligado").c_str());
  }
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("* Conectando ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
      publicaEstadoLed(digitalRead(PINO_LED) == HIGH);
    } else {
      Serial.println("Falha. Nova tentativa em 2s");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_LED, LOW);
  dht.begin();
  analogReadResolution(12);

  reconectWiFi();
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.google.com");

  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

void loop() {
  reconectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
  MQTT.loop();

  unsigned long agora = millis();
  if (agora - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = agora;

    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();
    int luminosidade = map(analogRead(PINO_LDR), 0, 4095, 0, 100);

    if (isnan(temperatura) || isnan(umidade)) {
      Serial.println("Falha na leitura do DHT22");
      return;
    }

    String payload = "t|" + String(temperatura, 1) +
                     "|h|" + String(umidade, 1) +
                     "|l|" + String(luminosidade);

    String ts = timestampISO();
    if (ts.length() > 0) payload += "|TimeInstant|" + ts;

    MQTT.publish(TOPICO_ATTRS, payload.c_str());
    Serial.print("Publicado: ");
    Serial.println(payload);
  }
}
