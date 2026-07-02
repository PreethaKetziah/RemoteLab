#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Stepper.h>

// ---------------- WIFI ----------------
const char* ssid = "NotForYou";
const char* password = "NotForYou";

// ---------------- HIVEMQ ----------------
const char* mqtt_server = "1dfb1112841b4e9aa8f1781b0fd753d3.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "RemoteLab";
const char* mqtt_pass = "Remote@Lab1";

// ---------------- MQTT TOPICS ----------------
const char* topic_cmd_value    = "remoteLab/cmd/value";
const char* topic_cmd_simulate = "remoteLab/cmd/simulate";
const char* topic_cmd_stop     = "remoteLab/cmd/stop";

const char* topic_lab_res      = "remoteLab/status/labResistance";
const char* topic_lab_state    = "remoteLab/status/labState";

// ---------------- STEPPER ----------------
const int STEPS_PER_REV = 2048;
Stepper myStepper(STEPS_PER_REV, 26,33,25,32);

// ---------------- MQTT ----------------
WiFiClientSecure espClient;
PubSubClient client(espClient);

float targetResistance = 0.0;
int currentSteps = 0;
unsigned long lastHeartbeat = 0;

// ---------- Publish state ----------
void publishLabState(const char* state) {
  client.publish(topic_lab_state, state);
}

// ---------- Publish student kit value only ----------
void publishStudentValue() {
  char msg[20];
  dtostrf(targetResistance, 0, 2, msg);
  client.publish(topic_lab_res, msg);
}

// ---------- WiFi ----------
void setupWiFi() {
  Serial.begin(115200);
  delay(1000);
  pinMode(15,INPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("WiFi connected");
}

// ---------- Move stepper based on student kit value ----------
void moveToResistance(float resistance) {
  int targetSteps = map((int)resistance, 0, 5000, 0, STEPS_PER_REV);
  targetSteps = constrain(targetSteps, 0, STEPS_PER_REV);

  int stepsToMove = targetSteps - currentSteps;

  if (stepsToMove != 0) {
    myStepper.step(stepsToMove);
    currentSteps = targetSteps;
  }
}

// ---------- MQTT callback ----------
void callback(char* topic, byte* payload, unsigned int length) {
  char msg[32];
  if (length >= sizeof(msg)) length = sizeof(msg) - 1;
  memcpy(msg, payload, length);
  msg[length] = '\0';

  String t = String(topic);
  String m = String(msg);

  if (t == topic_cmd_value) {
    targetResistance = m.toFloat();
  }

  if (t == topic_cmd_simulate) {
    if (m == "1") {
      Serial.println("Simulation started");

      publishLabState("SIMULATING");
      int angle = targetResistance* (300.0 / 5000.0);
       Serial.print(angle);
        Serial.print(" ");
        int targetSteps = angle / 0.17578125;
        myStepper.step(targetSteps);
        digitalWrite(25,LOW);
        digitalWrite(26,LOW);
        digitalWrite(32,LOW);
        digitalWrite(33,LOW);
        
      //moveToResistance(targetResistance);

      Serial.print("Student Kit Value: ");
      Serial.print(targetResistance, 2);
      Serial.println(" ohms");

      publishStudentValue();
      publishLabState("READY");
    }
  }

  if (t == topic_cmd_stop) {
    if (m == "1") {
      Serial.println("Simulation stopped");

      while(digitalRead(15) == 0)
      {
        myStepper.step(-1);
      }
      digitalWrite(25,LOW);
        digitalWrite(26,LOW);
        digitalWrite(32,LOW);
        digitalWrite(33,LOW);

      publishLabState("STOPPED");
    }
  }
}

// ---------- MQTT reconnect ----------
void reconnectMQTT() {
  while (!client.connected()) {
    String clientId = "LabKitESP32-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("MQTT connected");

      client.subscribe(topic_cmd_value);
      client.subscribe(topic_cmd_simulate);
      client.subscribe(topic_cmd_stop);

      publishLabState("READY");
      publishStudentValue();
    } else {
      delay(2000);
    }
  }
}

void setup() {
  setupWiFi();

  myStepper.setSpeed(10);

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  // heartbeat every 2 seconds
  if (millis() - lastHeartbeat > 2000) {
    lastHeartbeat = millis();
    publishLabState("READY");
    publishStudentValue();
  }
}