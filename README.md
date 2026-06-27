# AWS RFID Access System

AWS 기반 IoT RFID 출입 관리 시스템

ESP8266 NodeMCU와 RFID 모듈을 이용하여 카드 정보를 인식하고 AWS 웹 서버를 통해 관리하는 프로젝트입니다.

An AWS-based IoT RFID Access System using ESP8266 and RFID technology.

**ESP8266 · AWS EC2 · PN532 RFID · OLED Display · HTML · JavaScript**

---

# Project Overview

본 프로젝트는 ESP8266 NodeMCU와 PN532 RFID 모듈을 이용하여 RFID 카드를 인식하고, 카드 정보를 AWS EC2 기반 웹 서버와 연동하여 관리하는 IoT 시스템입니다.

RFID 태깅 시 OLED 디스플레이를 통해 카드 UID를 표시하며, 부저를 이용하여 사용자에게 인식 여부를 알려줍니다.

웹 서버에서는 RFID 카드 인식 정보를 확인할 수 있는 대시보드를 제공하도록 구성하였습니다.

---

# Features

### 1. RFID Card Detection

* PN532 RFID Reader를 이용한 카드 UID 인식
* 다양한 RFID 카드 태깅 지원
* 카드 UID 출력

### 2. OLED Display

* SSD1306 OLED 화면 출력
* 카드 UID 표시
* 현재 시간 및 상태 표시

### 3. Sound Feedback

* 카드 인식 시 부저 알림
* 사용자 인터랙션 제공

### 4. AWS Web Service

* AWS EC2 서버 구축
* 웹 기반 RFID 모니터링 페이지
* HTML Dashboard 구현
* API 연동을 통한 실시간 로그 관리 지원
  
---

# Tech Stack

## Hardware

* ESP8266 NodeMCU v2
* PN532 RFID Module
* SSD1306 OLED Display
* Passive Buzzer

## Cloud

* AWS EC2
* Linux Server
* SSH

## Frontend

* HTML5
* CSS3
* JavaScript

## Communication

* Wi-Fi
* HTTP

---

# System Flow

RFID Card

↓

ESP8266 NodeMCU

↓

Card UID Detection

↓

OLED Display & Buzzer

↓

AWS EC2 Web Server

↓

Web Dashboard

---

# Project Structure

```
AWS-RFID-System/

├── index.html                 # Web Dashboard
├── esp8266_pn532_test.ino
├── esp8266_rfid_oled_display.ino
├── esp8266_rfid_oled_clock.ino
├── esp8266_rfid_oled_time.ino
├── esp8266_rfid_oled_buzzer.ino
├── esp8266_oled_animation.ino
└── README.md
```

---

# Main Functions

* RFID 카드 UID 인식
* OLED 화면 출력
* 부저 알림
* Wi-Fi 기반 통신
* AWS EC2 웹 서버 구축
* 웹 대시보드 제공
* RFID 데이터 관리 기반 시스템

---

# Future Improvements

* REST API 연동
* 실시간 카드 로그 저장
* 사용자 인증 기능
* Database(MySQL) 연동
* 관리자 페이지 구현
* 출입 기록 조회 기능

---

