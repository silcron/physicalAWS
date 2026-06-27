#include <Wire.h>
#include <Adafruit_PN532.h>

// 사용자 정의 규칙에 따른 OLED 및 PN532 공유 I2C 핀 설정
#define I2C_SDA 14 // D5 (GPIO 14)
#define I2C_SCL 12 // D6 (GPIO 12)

// PN532 모듈의 IRQ와 RESET 핀은 하드웨어 I2C 통신 시 필수 연결이 아니므로
// 보드의 사용하지 않는 핀들을 더미(Dummy)로 지정해 둡니다.
#define PN532_IRQ   2  // D4 (GPIO 2)
#define PN532_RESET 0  // D3 (GPIO 0)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);

void setup(void) {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- PN532 RFID Reader Test ---");

  // I2C 버스를 D5(SDA), D6(SCL)로 명시적 초기화
  Wire.begin(I2C_SDA, I2C_SCL);

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    // 기기를 찾지 못하면 루프를 돌며 대기
    while (1) {
      delay(1000);
    }
  }
  
  // 발견한 칩 정보 시리얼 출력
  Serial.print("Found chip PN5"); 
  Serial.println((versiondata >> 24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); 
  Serial.print((versiondata >> 16) & 0xFF, DEC); 
  Serial.print('.'); 
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  
  // 카드를 읽기 위해 보안 액세스 모듈(SAM) 환경설정
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card (Mifare)...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // UID 저장 버퍼
  uint8_t uidLength;                        // UID 바이트 길이

  // 카드 태그를 1000ms 동안 대기 (비동기 루프를 위해 타임아웃 적용)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);
  
  if (success) {
    Serial.println("Found an RFID card!");
    Serial.print("UID Length: "); 
    Serial.print(uidLength, DEC); 
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    
    // 16진수 형태로 시리얼 출력
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) Serial.print("0");
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println("\n");
    
    // 동일 카드 중복 인식 지연
    delay(1500);
  } else {
    // 카드가 감지되지 않은 평상시 상태 출력
    Serial.println("Searching for card...");
  }
}
