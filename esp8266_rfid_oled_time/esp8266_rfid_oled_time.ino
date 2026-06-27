#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PN532.h>
#include <ESP8266WiFi.h>
#include <time.h>

// -------------------------
// 핀 설정
// -------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 사용자 정의 규칙: OLED 및 PN532 공유 I2C 핀
#define I2C_SDA 14 // D5 (GPIO 14)
#define I2C_SCL 12 // D6 (GPIO 12)

// PN532 더미 핀 (하드웨어 I2C 사용)
#define PN532_IRQ   2  // D4 (GPIO 2)
#define PN532_RESET 0  // D3 (GPIO 0)

// 패시브 부저 핀
#define BUZZER_PIN 13  // D7 (GPIO 13)

// -------------------------
// WiFi & NTP 설정
// -------------------------
const char* WIFI_SSID     = "SCNU_WiFi";
const char* WIFI_PASSWORD = "19960509";

// NTP 서버 및 한국 시간(KST = UTC+9)
const char* NTP_SERVER = "pool.ntp.org";
const long  UTC_OFFSET = 9 * 3600; // 초 단위 (+9시간)
const int   DST_OFFSET = 0;        // 한국은 서머타임 없음

// -------------------------
// 객체 생성
// -------------------------
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);

bool wifiConnected = false;
bool timeReady     = false;

// -------------------------
// 부저 사운드 함수
// -------------------------

void bootSound() {
  tone(BUZZER_PIN, 523, 120); // 도 (C5)
  delay(150);
  tone(BUZZER_PIN, 659, 120); // 미 (E5)
  delay(150);
  tone(BUZZER_PIN, 784, 200); // 솔 (G5)
  delay(250);
  noTone(BUZZER_PIN);
}

void cardDetectedSound() {
  tone(BUZZER_PIN, 1200, 80);
  delay(120);
  noTone(BUZZER_PIN);
  delay(60);
  tone(BUZZER_PIN, 1600, 80);
  delay(120);
  noTone(BUZZER_PIN);
}

// -------------------------
// 시간 문자열 반환 함수
// -------------------------

String getTimeString() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char buf[20];
  // HH:MM:SS 포맷
  sprintf(buf, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
  return String(buf);
}

String getDateString() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char buf[20];
  // YYYY-MM-DD 포맷
  sprintf(buf, "%04d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
  return String(buf);
}

// -------------------------
// 디스플레이 함수
// -------------------------

void showReadyMessage() {
  display.clearDisplay();
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

  if (timeReady) {
    // 날짜 표시 (상단)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(22, 6);
    display.println(getDateString());

    // 시간 표시 (중앙, 크게)
    display.setTextSize(2);
    display.setCursor(14, 22);
    display.println(getTimeString());

    // 안내 텍스트 (하단)
    display.setTextSize(1);
    display.setCursor(20, 50);
    display.println("Scan Your Card");
  } else {
    // WiFi 연결 중이거나 NTP 동기화 전 표시
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 18);
    display.println("RFID Reader Ready");
    display.setCursor(20, 38);
    if (!wifiConnected) {
      display.println("Connecting WiFi..");
    } else {
      display.println("Syncing Time...");
    }
  }

  display.display();
}

// -------------------------
// Setup
// -------------------------

void setup(void) {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- RFID + OLED + WiFi + NTP ---");

  // 부저 핀 초기화
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  // I2C 버스 초기화
  Wire.begin(I2C_SDA, I2C_SCL);

  // OLED 초기화
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 20);
  display.println("Connecting WiFi...");
  display.display();

  // WiFi 연결
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi connecting");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());

    // NTP 시간 동기화
    configTime(UTC_OFFSET, DST_OFFSET, NTP_SERVER);
    Serial.print("Syncing NTP time");
    unsigned long ntpStart = millis();
    while (time(nullptr) < 100000 && millis() - ntpStart < 8000) {
      delay(500);
      Serial.print(".");
    }

    if (time(nullptr) > 100000) {
      timeReady = true;
      Serial.println("\nTime synced: " + getDateString() + " " + getTimeString());
    } else {
      Serial.println("\nNTP sync failed (시간 미동기화)");
    }
  } else {
    Serial.println("\nWiFi connection failed");
    display.clearDisplay();
    display.setCursor(5, 20);
    display.println("WiFi Failed!");
    display.setCursor(5, 35);
    display.println("Check SSID/PW");
    display.display();
    delay(2000);
  }

  // PN532 초기화
  display.clearDisplay();
  display.setCursor(8, 20);
  display.println("Init RFID...");
  display.display();

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("PN532 not found");
    display.clearDisplay();
    display.setCursor(10, 20);
    display.println("PN532 Not Found!");
    display.display();
    while (1) { delay(1000); }
  }
  nfc.SAMConfig();
  Serial.println("PN532 Ready.");

  // 부팅 완료 사운드
  bootSound();

  // 대기 화면 표시
  showReadyMessage();
}

// -------------------------
// Loop
// -------------------------

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  // 대기 화면 매 루프마다 갱신 (시간 업데이트용)
  showReadyMessage();

  // 300ms 동안 카드 대기
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 300);

  if (success) {
    Serial.println("RFID Card Found!");

    // 카드 감지 사운드
    cardDetectedSound();

    // 카드 감지 시 화면
    display.clearDisplay();
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);

    // 상단 강조 헤더 (반전)
    display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(22, 4);
    display.println("CARD DETECTED!");

    // 카드 UID 정보
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 25);
    display.print("Length: ");
    display.print(uidLength, DEC);
    display.println(" bytes");

    display.setCursor(10, 42);
    display.print("UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) display.print("0");
      display.print(uid[i], HEX);
      if (i < uidLength - 1) display.print(":");
    }

    // 카드 감지 시각 표시 (우하단 소형)
    if (timeReady) {
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(70, 54);
      display.println(getTimeString());
    }

    display.display();

    // 시리얼 출력
    Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) Serial.print("0");
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // 2.5초 유지 후 복귀
    delay(2500);
  }
}
