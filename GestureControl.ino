  // ● ● ●  gesture_car_esp8266.ino
/*
 * TILT CONTROLLED ROBOT CAR (MQTT)
 * ESP8266 + L298N
 * Speed control using ENA and ENB
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ==========================================
// WIFI + MQTT
// ==========================================

const char* ssid = "Dhruv's A35";
const char* password = "dhruv2813";

const char* mqtt_topic = "my_car";
const cc:\Users\Purvang\OneDrive\Desktop\Bank management fraud detection systemhar* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

// ==========================================
// MOTOR PINS
// ==========================================

// Right Motor
#define IN1 D8
#define IN2 D7

// Left Motor
#define IN3 D4
#define IN4 D3

// Speed Pins
#define ENA D5
#define ENB D6

WiFiClient espClient;
PubSubClient client(espClient);

// ==========================================
// WIFI CONNECT
// ==========================================

void setup_wifi() {

  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ==========================================
// MQTT CALLBACK
// ==========================================

void callback(char* topic, byte* payload, unsigned int length) {

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Expected format:
  // roll,pitch

  int commaIndex = message.indexOf(',');

  if (commaIndex != -1) {

    float roll = message.substring(0, commaIndex).toFloat();
    float pitch = message.substring(commaIndex + 1).toFloat();

    // Convert tilt to speed values

    int throttle = map(constrain(pitch, -45, 45),
                       -45, 45,
                       -1023, 1023);

    int steering = map(constrain(roll, -45, 45),
                       -45, 45,
                       -500, 500);

    int leftSpeed = constrain(throttle + steering, -1023, 1023);
    int rightSpeed = constrain(throttle - steering, -1023, 1023);

    // Dead zone

    if (abs(throttle) < 150 && abs(steering) < 150) {
      stopCar();
    }
    else {
      moveCar(leftSpeed, rightSpeed);
    }
  }
}

// ==========================================
// MOTOR CONTROL
// ==========================================

void moveCar(int leftSpeed, int rightSpeed) {

  Serial.print("Left: ");
  Serial.print(leftSpeed);

  Serial.print(" | Right: ");
  Serial.println(rightSpeed);

  // LEFT MOTOR

  if (leftSpeed > 0) {

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

  } else {

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  // RIGHT MOTOR

  if (rightSpeed > 0) {

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

  } else {

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }

  // SPEED CONTROL

  analogWrite(ENA, abs(rightSpeed));
  analogWrite(ENB, abs(leftSpeed));
}

// ==========================================
// STOP CAR
// ==========================================

void stopCar() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ==========================================
// MQTT RECONNECT
// ==========================================

void reconnect() {

  while (!client.connected()) {

    Serial.print("Attempting MQTT connection...");

    String clientId = "RobotCar-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {

      Serial.println("connected");

      client.subscribe(mqtt_topic);

    } else {

      Serial.print("failed, rc=");
      Serial.print(client.state());

      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopCar();

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// ==========================================
// LOOP
// ==========================================

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}

