#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 사용자 정의 규칙에 따른 OLED 핀 설정: SDA = D5 (GPIO14), SCL = D6 (GPIO12)
#define OLED_SDA 14 // GPIO 14
#define OLED_SCL 12 // GPIO 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long lastSwitchTime = 0;
int currentMode = 0;
const unsigned long MODE_DURATION = 8000; // 각 애니메이션 당 8초

// 1. Starfield Variables
#define NUM_STARS 40
struct Star {
  float x, y, z;
};
Star stars[NUM_STARS];

// 2. Mystify Variables
#define MYSTIFY_LINES 3
struct Point {
  float x, y;
  float dx, dy;
};
Point points[4]; // 두 선을 이루는 4개의 꼭짓점
struct LineHistory {
  Point pts[4];
};
LineHistory history[MYSTIFY_LINES];
int historyIndex = 0;

// 3. Lissajous Variables
float lissajousDelta = 0;

// 4. Rotating Cube Variables
struct Point3D {
  float x, y, z;
};
Point3D cubeVertices[8] = {
  {-20, -20, -20}, {20, -20, -20}, {20, 20, -20}, {-20, 20, -20},
  {-20, -20, 20},  {20, -20, 20},  {20, 20, 20},  {-20, 20, 20}
};
// 12 모서리를 연결할 인덱스 쌍
const int cubeEdges[12][2] = {
  {0, 1}, {1, 2}, {2, 3}, {3, 0}, // 뒤쪽 면
  {4, 5}, {5, 6}, {6, 7}, {7, 4}, // 앞쪽 면
  {0, 4}, {1, 5}, {2, 6}, {3, 7}  // 연결선
};
float angleX = 0;
float angleY = 0;
float angleZ = 0;

// 5. Concentric Waves Variables
float wavePhase = 0;

void initStarfield() {
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].x = random(-64, 64);
    stars[i].y = random(-32, 32);
    stars[i].z = random(1, 128);
  }
}

void initMystify() {
  for (int i = 0; i < 4; i++) {
    points[i].x = random(0, SCREEN_WIDTH);
    points[i].y = random(0, SCREEN_HEIGHT);
    points[i].dx = (random(10, 30) / 10.0) * (random(0, 2) == 0 ? 1 : -1);
    points[i].dy = (random(10, 30) / 10.0) * (random(0, 2) == 0 ? 1 : -1);
  }
  for (int i = 0; i < MYSTIFY_LINES; i++) {
    for (int j = 0; j < 4; j++) {
      history[i].pts[j] = points[j];
    }
  }
}

void setup() {
  Serial.begin(115200);

  // 사용자 정의 규칙에 맞춰 I2C를 D5(SDA), D6(SCL)로 초기화
  Wire.begin(OLED_SDA, OLED_SCL);

  // OLED 디스플레이 시작 (주소 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.display();
  
  initStarfield();
  initMystify();
  
  lastSwitchTime = millis();
}

// 1. Starfield Animation
void drawStarfield() {
  display.clearDisplay();
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].z -= 2.0;
    if (stars[i].z <= 0) {
      stars[i].x = random(-64, 64);
      stars[i].y = random(-32, 32);
      stars[i].z = 128;
    }
    
    int sx = (int)((stars[i].x * 64.0) / stars[i].z) + 64;
    int sy = (int)((stars[i].y * 32.0) / stars[i].z) + 32;
    
    if (sx >= 0 && sx < SCREEN_WIDTH && sy >= 0 && sy < SCREEN_HEIGHT) {
      // 별의 깊이(z)가 가까울수록 크게 표현
      if (stars[i].z < 32) {
        display.fillRect(sx, sy, 2, 2, SSD1306_WHITE);
      } else {
        display.drawPixel(sx, sy, SSD1306_WHITE);
      }
    }
  }
  display.display();
}

// 2. Mystify Animation
void drawMystify() {
  display.clearDisplay();
  
  // 꼭짓점 이동 및 경계선 충돌 검사
  for (int i = 0; i < 4; i++) {
    points[i].x += points[i].dx;
    points[i].y += points[i].dy;
    
    if (points[i].x <= 0 || points[i].x >= SCREEN_WIDTH - 1) {
      points[i].dx *= -1;
      points[i].x = constrain(points[i].x, 0, SCREEN_WIDTH - 1);
    }
    if (points[i].y <= 0 || points[i].y >= SCREEN_HEIGHT - 1) {
      points[i].dy *= -1;
      points[i].y = constrain(points[i].y, 0, SCREEN_HEIGHT - 1);
    }
  }
  
  // 이력 저장
  for (int j = 0; j < 4; j++) {
    history[historyIndex].pts[j] = points[j];
  }
  
  // 잔상 그리기
  for (int i = 0; i < MYSTIFY_LINES; i++) {
    int idx = (historyIndex + i + 1) % MYSTIFY_LINES;
    display.drawLine(history[idx].pts[0].x, history[idx].pts[0].y, history[idx].pts[1].x, history[idx].pts[1].y, SSD1306_WHITE);
    display.drawLine(history[idx].pts[2].x, history[idx].pts[2].y, history[idx].pts[3].x, history[idx].pts[3].y, SSD1306_WHITE);
  }
  
  historyIndex = (historyIndex + 1) % MYSTIFY_LINES;
  display.display();
}

// 3. Lissajous Curves Animation
void drawLissajous() {
  display.clearDisplay();
  
  int prevX = 0, prevY = 0;
  float freqX = 3.0;
  float freqY = 5.0;
  
  for (float t = 0; t <= 2 * PI + 0.1; t += 0.05) {
    int x = (int)(55.0 * sin(freqX * t + lissajousDelta) + 64);
    int y = (int)(27.0 * sin(freqY * t) + 32);
    
    if (t > 0) {
      display.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
    }
    prevX = x;
    prevY = y;
  }
  
  lissajousDelta += 0.05;
  if (lissajousDelta > 2 * PI) {
    lissajousDelta = 0;
  }
  
  display.display();
}

// 4. Rotating Cube Animation
void drawRotatingCube() {
  display.clearDisplay();
  
  Point3D projected[8];
  
  for (int i = 0; i < 8; i++) {
    // X축 회전
    float y1 = cubeVertices[i].y * cos(angleX) - cubeVertices[i].z * sin(angleX);
    float z1 = cubeVertices[i].y * sin(angleX) + cubeVertices[i].z * cos(angleX);
    
    // Y축 회전
    float x2 = cubeVertices[i].x * cos(angleY) + z1 * sin(angleY);
    float z2 = -cubeVertices[i].x * sin(angleY) + z1 * cos(angleY);
    
    // Z축 회전
    float x3 = x2 * cos(angleZ) - y1 * sin(angleZ);
    float y3 = x2 * sin(angleZ) + y1 * cos(angleZ);
    
    // 원근 투영 효과
    float distance = 60.0;
    float zOffset = 100.0;
    float sz = z2 + zOffset;
    projected[i].x = (x3 * distance) / sz + 64;
    projected[i].y = (y3 * distance) / sz + 32;
  }
  
  for (int i = 0; i < 12; i++) {
    display.drawLine(
      projected[cubeEdges[i][0]].x, projected[cubeEdges[i][0]].y,
      projected[cubeEdges[i][1]].x, projected[cubeEdges[i][1]].y,
      SSD1306_WHITE
    );
  }
  
  angleX += 0.02;
  angleY += 0.03;
  angleZ += 0.01;
  
  display.display();
}

// 5. Expanding Concentric Rays Animation
void drawConcentricRays() {
  display.clearDisplay();
  
  int cx = SCREEN_WIDTH / 2;
  int cy = SCREEN_HEIGHT / 2;
  
  int numRays = 16;
  float angleStep = (2 * PI) / numRays;
  for (int i = 0; i < numRays; i++) {
    float angle = i * angleStep + wavePhase;
    int rx = (int)(cx + 60 * cos(angle));
    int ry = (int)(cy + 30 * sin(angle));
    display.drawLine(cx, cy, rx, ry, SSD1306_WHITE);
  }
  
  for (int w = 5; w < 80; w += 15) {
    int currentWidth = (w + (int)(wavePhase * 10)) % 80;
    if (currentWidth > 0) {
      int rx = cx - currentWidth;
      int ry = cy - (currentWidth / 2);
      int rw = currentWidth * 2;
      int rh = currentWidth;
      display.drawRect(rx, ry, rw, rh, SSD1306_WHITE);
    }
  }
  
  wavePhase += 0.05;
  if (wavePhase > 2 * PI) {
    wavePhase = 0;
  }
  
  display.display();
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastSwitchTime >= MODE_DURATION) {
    currentMode = (currentMode + 1) % 5;
    lastSwitchTime = currentTime;
    
    if (currentMode == 0) initStarfield();
    if (currentMode == 1) initMystify();
  }
  
  switch (currentMode) {
    case 0:
      drawStarfield();
      break;
    case 1:
      drawMystify();
      break;
    case 2:
      drawLissajous();
      break;
    case 3:
      drawRotatingCube();
      break;
    case 4:
      drawConcentricRays();
      break;
  }
  
  delay(20);
}
