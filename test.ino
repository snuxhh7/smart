#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ==============================
// Wi-Fi 설정 (사용 환경에 맞게 수정)
// ==============================
const char* ssid = "SecondClass2.4G";
const char* password = "12345678";

// ESP32 고정 IP 설정
IPAddress local_IP(192, 168, 0, 25);
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
const int motorSpeed = 90; // 모터 속도 (0 ~ 255)

int motorPins[] = {PWMA, PWMB, AIN1, AIN2, BIN1, BIN2, STBY};

// ==============================
// 제어 보조 함수
// ==============================
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

// -------------------------------------------------------------
// 웹 명령(cmd)과 실제 동작/LED 매칭 수정
// -------------------------------------------------------------

// 전진 (f - 웹에서 W 키 또는 전진 버튼)
void forward() {
  motor(motorSpeed, motorSpeed, LOW, HIGH, HIGH, LOW); // 기존 우회전 동작으로 변경됨 (필요시 방향 반대면 수정)
  ledColor(0, 255, 0); // 초록색
}

// 후진 (b - 웹에서 S 키 또는 후진 버튼)
void backward() {
  motor(motorSpeed, motorSpeed, HIGH, LOW, LOW, HIGH); // 기존 좌회전 동작으로 변경됨
  ledColor(255, 0, 0); // 빨간색
}

// 좌회전 (l - 웹에서 A 키 또는 좌회전 버튼) -> 실제 A를 눌렀을 때 동작하도록 매칭
void leftTurn() {
  motor(motorSpeed, motorSpeed, HIGH, LOW, HIGH, LOW); // 기존 전진 동작
  ledColor(255, 255, 0); // 노란색
}

// 우회전 (r - 웹에서 D 키 또는 우회전 버튼) -> 실제 D를 눌렀을 때 동작하도록 매칭
void rightTurn() {
  motor(motorSpeed, motorSpeed, LOW, HIGH, LOW, HIGH); // 기존 후진 동작
  ledColor(0, 0, 255); // 파란색
}

// 정지 (s)
void stopMotor() {
  motor(0, 0, LOW, LOW, LOW, LOW);
  ledColor(255, 255, 255); // 하얀색
}

// ==============================
// 웹서버 요청 처리 (/control?cmd=...)
// ==============================
void handleControl() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");

    if (cmd == "f") {
      forward();
      server.send(200, "text/plain", "FORWARD");
    }
    else if (cmd == "b") {
      backward();
      server.send(200, "text/plain", "BACKWARD");
    }
    else if (cmd == "l") {
      leftTurn();
      server.send(200, "text/plain", "LEFT");
    }
    else if (cmd == "r") {
      rightTurn();
      server.send(200, "text/plain", "RIGHT");
    }
    else if (cmd == "s") {
      stopMotor();
      server.send(200, "text/plain", "STOP");
    }
    else {
      server.send(400, "text/plain", "Invalid Command");
    }
  } else {
    server.send(400, "text/plain", "Missing cmd parameter");
  }
}

// ==============================
// 기본 페이지 (연결 테스트용)
// ==============================
void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "ESP32 Robot Server is Ready!");
}

// ==============================
// setup
// ==============================
void setup() {
  Serial.begin(115200);

  for(int i = 0; i < 7; i++) {
    pinMode(motorPins[i], OUTPUT);
  }
  digitalWrite(STBY, HIGH);

  strip.begin();
  stopMotor();

  WiFi.mode(WIFI_STA);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Static IP configuration failed");
  }

  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("=========================");
  Serial.println("Wi-Fi connected successfully!");
  Serial.print("ESP32 IP : ");
  Serial.println(WiFi.localIP());
  Serial.println("=========================");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/control", HTTP_GET, handleControl);

  server.begin();
  Serial.println("Web Server Started");
}

// ==============================
// loop
// ==============================
void loop() {
  server.handleClient();
}
