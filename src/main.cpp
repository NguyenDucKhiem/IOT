#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>

#define LED       D0 //Define blinking LED pin
#define LED_LIVINGROOM  D2
#define DHTPIN    D7
#define DHTTYPE   DHT11
#define MQTT_PORT 1883
#define MQTT_NAME   "khiemnd97"
#define MQTT_PASS   "19579fdd6f0f47c98e9e83493a95db19"

const char REQUEST_TOPIC[] = "khiemnd97/feeds/request";
char WIFI_SSID[][15] = {"ABC", "Doanh"};
char WIFI_PASS[][25] = {"motdentambangchu", "0339511474"};
const char* mqtt_server = "io.adafruit.com";
uint32_t delayMS;
float temperature, humidity;
unsigned long previousMillis;
int time_count;
const long interval = 1000; 
const int DELAY_TIME = 5000;
const int DELAY_READ_DHT = 30;  //seconds
DHT_Unified dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void MQTT_connect();
void WL_scan_wifi();
int WL_connect_wifi(char *SSID, char *pass, int time);
void DHT_init();
int DHT_read(float *temperature, float *humidity);
void DHT_read_periodic(int time_periodic);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  //Connect to WiFi
  WL_scan_wifi();
  int i = 0;
  while (WL_connect_wifi(WIFI_SSID[i], WIFI_PASS[i], DELAY_TIME) != WL_CONNECTED)
  {
    delay(DELAY_TIME);
    i++;

    if (i == 3)
      i = 0;
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, int length) {
  String data;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    data += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

if (data == "on_light_bedroom")
  {
    //Active low logic
    digitalWrite(LED, LOW);
    client.publish("khiemnd97/feeds/light_bedroom", "ON");
  }
  else if (data == "off_light_bedroom")
  {
    //Active low logic
    digitalWrite(LED, HIGH);

    client.publish("khiemnd97/feeds/light_bedroom", "OFF");
  } else if (data == "on_light_livingroom")
  {
    //Active low logic
    digitalWrite(LED_LIVINGROOM, LOW);
    client.publish("khiemnd97/feeds/light_livingroom", "ON");
  }
  else if (data == "off_light_livingroom")
  {
    //Active low logic
    digitalWrite(LED_LIVINGROOM, HIGH);

    client.publish("khiemnd97/feeds/light_livingroom", "OFF");
  }
  else if (data == "request_temperature")
  {
    char string[50];
    sprintf(string, "Temperature of house is %.2f°C", temperature);
    client.publish("khiemnd97/feeds/message", string);
  } else if (data == "request_humidity")
  {
    char string[50];
    sprintf(string, "Humidity of house is %.2f%s", humidity,"%");
    client.publish("khiemnd97/feeds/message", string);
  } else if (data == "request_temperature_humidity")
  {
    char string[100];
    sprintf(string, "Temperature of house is %.2f°C. Humidity of house is %.2f%s", temperature, humidity, "%");
    client.publish("khiemnd97/feeds/message", string);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_NAME, MQTT_PASS)) {
      client.subscribe(REQUEST_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  Serial.println("OK!");

}

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(LED_LIVINGROOM, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, MQTT_PORT);
  client.setCallback(callback);
  
  DHT_init();
  previousMillis = millis();
  time_count = 0;
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  DHT_read_periodic(DELAY_READ_DHT);
}


int WL_connect_wifi(char *SSID, char *pass, int time)
{
  Serial.print("\n\nConnecting Wifi...");
  WiFi.begin(SSID, pass);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 10)
  {
          delay(time / 10);
          Serial.print(".");
          i++;
  } 
  
  return WiFi.status();
}

void WL_scan_wifi()
{
  int numberOfNetworks = WiFi.scanNetworks();
  Serial.println("\n\n=========================================");

  for(int i =0; i<numberOfNetworks; i++){

    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.println("=========================================");

  }
}

void DHT_init()
{
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  Serial.print("Min delay DHT:  "); Serial.println(delayMS); 
  temperature = 0;
  humidity = 0;
}

int DHT_read(float *temperature, float *humidity)
{
  int ret = 0;
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    ret = -1;
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    *temperature = event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
    ret = (ret - 1) * 2;
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    *humidity = event.relative_humidity;
  }

  return ret;
}

void DHT_read_periodic(int time_periodic)
{
  if (time_count == 0){
      char string[10];
      Serial.println("Read data DHT.");
      if (!DHT_read(&temperature, &humidity)){
        sprintf(string, "%f", temperature);
        client.publish("khiemnd97/feeds/temperature", string);
        sprintf(string, "%f", humidity);
        client.publish("khiemnd97/feeds/humidity", string);
      }
    }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    time_count += (currentMillis - previousMillis) / interval;
    if (time_count >= DELAY_READ_DHT)
      time_count = 0;
    
    previousMillis = currentMillis;
  }
 
}