// Autor: DjEdu28 <GitHub.com/DjEdu28>
// # Como configurar o ESP32 no Arduino IDE
// https://www.crescerengenharia.com/post/instalando-esp32-arduino
// # Config_Arduino https://dl.espressif.com/dl/package_esp32_index.json
//
// # Projetos WOKWI
// ESP32 com DHT22 by DjEdu28 https://wokwi.com/projects/397207107109591041
// https://wokwi.com/projects/397128466983540737

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Substitua com suas credenciais de WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Substitua com o endereço do seu broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const char* clientId = "esp32_djedu28_397207107109591041";
const String cod = "";

const char* TOPIC_temperature = String("esp32_djedu28_temperature"+cod).c_str();
const char* TOPIC_humidity = String("esp32_djedu28_humidity"+cod).c_str();
const char* TOPIC_ledControl = String("esp32_djedu28_ledControl"+cod).c_str();

WiFiClient espClient;
PubSubClient client(espClient);

// Substitua o pino do LED
#define LED_PIN 15

// Substitua o pino do sensor DHT
#define DHTPIN 4
// DHTTYPE DHT22 ou DHT11
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

unsigned int tempo_ans;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  Serial.begin(115200);
  Serial.print("Iniciando ");
  Serial.println(TOPIC_temperature);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
  digitalWrite(LED_PIN, LOW);
  tempo_ans = millis();
}

void setup_wifi() {
  delay(10);
  Serial.println("");
  Serial.print("Conectando... ");
  Serial.println(ssid);

  WiFi.begin(ssid, password, 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print(" WiFi Conectado!");
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar MQTT ... ");
    if (client.connect(clientId)) {
      Serial.println("conectado");
      client.subscribe(TOPIC_ledControl);
      if (digitalRead(LED_PIN)) {
        client.publish(TOPIC_ledControl, String("ON").c_str());
      } else {
        client.publish(TOPIC_ledControl, String("OFF").c_str());
      }
    } else {
      Serial.print("falhou, rc =");
      Serial.print(client.state());
      Serial.println(" Tente novamente em 5 segundos");
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Falha ao ler o sensor DHT!");
    return;
  }

  if (millis() - tempo_ans >= 5000) {
    //Publica os dados de temperatura e umidade
    client.publish(TOPIC_temperature, String(temperature).c_str());
    client.publish(TOPIC_humidity, String(humidity).c_str());
    
    tempo_ans = millis();
    Serial.print("S");
  }


  Serial.print("\tTEMP\t");
  Serial.print(temperature);
  Serial.print("C\t");

  Serial.print("UMID\t");
  Serial.print(temperature);
  Serial.println("%");

  delay(500);
}

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.print("Mensagem recebida[");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(messageTemp);

  if (String(topic) == TOPIC_ledControl) {
    Serial.println("atualizando led");
    if (messageTemp == "ON") {
      digitalWrite(LED_PIN, HIGH);
    } else if (messageTemp == "OFF") {
      digitalWrite(LED_PIN, LOW);
    }
  }
}