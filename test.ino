#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ==============================
// Wi-Fi 설정
// ==============================
const char* ssid = "FirstClass2.4G";
const char* password = "12345678";

IPAddress local_IP(192, 168, 0, 50);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// ==============================
// NeoPixel 설정
// ==============================
#define LED_PIN 27
#define LED_COUNT 2

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ==============================
// 모터 드라이버 핀 설정
// ==============================
const int PWMA = 19;
const int PWMB = 18;
const int AIN1 = 33;
const int AIN2 = 32;
const int BIN1 = 25;
const int BIN2 = 26;
const int STBY = 5;
const int motorSpeed = 90;

int motorPins[] = {PWMA, PWMB, AIN1, AIN2, BIN1, BIN2, STBY};

void ledColor(int r, int g, int b) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void motor(int leftSpeed, int rightSpeed, int a1, int a2, int b1, int b2){
  analogWrite(PWMA, leftSpeed);
  analogWrite(PWMB, rightSpeed);
  digitalWrite(AIN1, a1);
  digitalWrite(AIN2, a2);
  digitalWrite(BIN1, b1);
  digitalWrite(BIN2, b2);
}

void forward() {
  motor(motorSpeed, motorSpeed, LOW, HIGH, HIGH, LOW);
  ledColor(0, 255, 0);
}

void backward() {
  motor(motorSpeed, motorSpeed, HIGH, LOW, LOW, HIGH);
  ledColor(255, 0, 0);
}

void leftTurn() {
  motor(motorSpeed, motorSpeed, LOW, HIGH, LOW, HIGH);
  ledColor(255, 255, 0);
}

void rightTurn() {
  motor(motorSpeed, motorSpeed, HIGH, LOW, HIGH, LOW);
  ledColor(0, 0, 255);
}

void stopMotor() {
  motor(0, 0, LOW, LOW, LOW, LOW);
  ledColor(255, 255, 255);
}

void handleControl() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");

    if (cmd == "f") { forward(); server.send(200, "text/plain", "OK"); }
    else if (cmd == "b") { backward(); server.send(200, "text/plain", "OK"); }
    else if (cmd == "l") { leftTurn(); server.send(200, "text/plain", "OK"); }
    else if (cmd == "r") { rightTurn(); server.send(200, "text/plain", "OK"); }
    else if (cmd == "s") { stopMotor(); server.send(200, "text/plain", "OK"); }
    else { server.send(400, "text/plain", "Invalid"); }
  } else {
    server.send(400, "text/plain", "Missing");
  }
}

void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Ready");
}

void setup() {
  Serial.begin(115200);

  for(int i = 0; i < 7; i++) {
    pinMode(motorPins[i], OUTPUT);
  }
  digitalWrite(STBY, HIGH);

  strip.begin();
  stopMotor();

  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/control", HTTP_GET, handleControl);

  server.begin();
}

void loop() {
  server.handleClient();
}
