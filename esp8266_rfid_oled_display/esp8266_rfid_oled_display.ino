#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PN532.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 사용자 정의 규칙에 따른 OLED 및 PN532 공유 I2C 핀 설정
#define I2C_SDA 14 // D5 (GPIO 14)
#define I2C_SCL 12 // D6 (GPIO 12)

// PN532 더미 핀
#define PN532_IRQ   2  // D4 (GPIO 2)
#define PN532_RESET 0  // D3 (GPIO 0)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);

void showReadyMessage() {
  display.clearDisplay();
  
  // 테두리 상자 그리기
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 18);
  display.println("RFID Reader Ready");
  
  display.setCursor(20, 38);
  display.println("Scan Your Card");
  
  display.display();
}

void setup(void) {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- PN532 RFID + OLED Display ---");

  // I2C 버스 초기화
  Wire.begin(I2C_SDA, I2C_SCL);

  // OLED 초기화 (I2C 주소 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Initializing...");
  display.display();

  // PN532 초기화
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    display.clearDisplay();
    display.setCursor(10, 20);
    display.println("PN532 Not Found!");
    display.display();
    while (1) {
      delay(1000);
    }
  }
  
  // SAM 환경설정
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card (Mifare)...");
  showReadyMessage();
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // UID 저장 버퍼
  uint8_t uidLength;                        // UID 바이트 길이

  // 300ms 동안 카드 대기 (비동기 루프 대기 시간 최적화)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 300);
  
  if (success) {
    Serial.println("Found an RFID card!");
    
    // OLED에 카드 정보 표시
    display.clearDisplay();
    
    // 카드 감지 헤더 프레임
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK); // 상단 바 텍스트 색상 반전
    display.setCursor(22, 4);
    display.println("CARD DETECTED!");
    
    // 카드 상세 정보
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 25);
    display.print("Length: ");
    display.print(uidLength, DEC);
    display.println(" bytes");
    
    // UID 값 출력 (대문자 HEX 포맷)
    display.setCursor(10, 42);
    display.print("UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) display.print("0");
      display.print(uid[i], HEX);
      if (i < uidLength - 1) display.print(":");
    }
    
    display.display();
    
    // 시리얼 콘솔 출력
    Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) Serial.print("0");
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println("\n");
    
    // 카드 정보를 2.5초간 화면에 유지
    delay(2500);
    
    // 대기 메시지로 복귀
    showReadyMessage();
  }
}
