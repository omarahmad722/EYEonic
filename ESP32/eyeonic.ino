#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "EYEonic";
const char* password = "12345678";

WiFiUDP udp;
const int UDP_PORT = 5005;

// ===== MOTOR PINS =====
#define IN1 33
#define IN2 27
#define IN3 14
#define IN4 12
#define ENA 25
#define ENB 32

// ===== ULTRASONIC =====
#define TRIG1 26
#define ECHO1 34
#define TRIG2 5
#define ECHO2 18

// ===== SERVO =====
#define SERVO_PIN 13

// ===== LED + BUZZER =====
#define LED_PIN    2
#define BUZZER_PIN 4

// ===== DFPlayer Pro =====
#define DFP_RX 16
#define DFP_TX 17

// ===== SOUND TRACKS (USB) =====
#define SND_SYSTEM_ACTIVE   1   // 001.mp3 - EYEonic System Active
#define SND_CONNECTED       2   // 002.mp3 - Connected
#define SND_CONN_LOST       3   // 003.mp3 - Connection Lost
#define SND_STOP_OBS        4   // 004.mp3 - Obstacle Detected
#define SND_EYE_ACTIVE      5   // 005.mp3 - Eye tracking active
#define SND_EMERGENCY       6   // 006.mp3 - Emergency Activated
#define SND_TRACK_COUNT     6   //

// ===== DISTANCES =====
#define DIST_SAFE      60
#define DIST_CAUTION   40
#define DIST_WARNING   20
#define DIST_EMERGENCY 10

// ===== SPEED =====
#define SPEED_MAX 180
#define SPEED_MIN  80

Servo myServo;

// ===== STATE =====
bool          clientConnected   = false;
bool          emergencyMode     = false;
bool          sensorFault       = false;
int           lastDistZone      = -1;
char          lastMovCmd        = 0;
int           zeroReadCount     = 0;
#define ZERO_LIMIT              10

unsigned long lastCmdMs         = 0;
#define CONN_TIMEOUT_MS         60000UL

// ===== LED TIMING =====
unsigned long lastLedToggle     = 0;
bool          ledState          = false;

// ===== AUDIO STATE =====
bool          audioPlaying      = false;
unsigned long audioStartMs      = 0;
unsigned long audioGuardMs      = 0;
int           pendingTrack      = 0;

const unsigned long TRACK_DURATION[] = {
    0,
    3000,  // 1 - System Active
    3000,  // 2 - Connected
    2500,  // 3 - Connection Lost
    2500,  // 4 - Obstacle Detected
    2500,  // 5 - Eye tracking active
    3000,  // 6 - Emergency Activated
};

// =====================================================
// ================= DFPlayer Pro ======================
// =====================================================

void dfSend(const char* cmd) {
    Serial2.print(cmd);
    Serial2.print("\r\n");
    Serial.printf("[DFP] %s\n", cmd);
}

void playSound(int track, bool force = false) {
    if (track < 1 || track > SND_TRACK_COUNT) return;
    if (audioPlaying && !force) {
        pendingTrack = track;
        return;
    }
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+PLAYFILE=%d", track);
    dfSend(cmd);
    audioPlaying = true;
    audioStartMs = millis();
    audioGuardMs = TRACK_DURATION[track] + 500;
    pendingTrack = 0;
    Serial.printf("🔊 Track %d (guard %lums)\n", track, audioGuardMs);
}

void updateAudio() {
    if (audioPlaying && (millis() - audioStartMs >= audioGuardMs)) {
        audioPlaying = false;
        if (pendingTrack != 0) {
            int t = pendingTrack;
            pendingTrack = 0;
            playSound(t);
        }
    }
    while (Serial2.available()) {
        String resp = Serial2.readStringUntil('\n');
        Serial.print("[DFP] ");
        Serial.println(resp);
    }
}

void setVolume(int vol) {
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "AT+VOL=%d", vol);
    dfSend(cmd);
    delay(200);
}

// =====================================================
// ================= MOTOR FUNCTIONS ===================
// =====================================================

void stopCar() {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);    analogWrite(ENB, 0);
}

void moveForward(int spd = SPEED_MAX) {
    analogWrite(ENA, spd); analogWrite(ENB, spd);
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void moveBackward() {
    analogWrite(ENA, SPEED_MAX); analogWrite(ENB, SPEED_MAX);
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void turnLeft(int spd = SPEED_MAX) {
    analogWrite(ENA, spd); analogWrite(ENB, spd);
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight(int spd = SPEED_MAX) {
    analogWrite(ENA, spd); analogWrite(ENB, spd);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

// =====================================================
// ================= ULTRASONIC ========================
// =====================================================

long readUltrasonic(int trig, int echo) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long duration = pulseIn(echo, HIGH, 30000);
    return duration * 0.034 / 2;
}

// =====================================================
// ================= SENSOR FAULT ======================
// =====================================================

void checkSensorFault(long d1, long d2) {
    if (d1 == 0 && d2 == 0) {
        zeroReadCount++;
        if (zeroReadCount >= ZERO_LIMIT && !sensorFault) {
            sensorFault = true;
            stopCar();
            Serial.println("⚠️ Sensor Fault!");
        }
    } else {
        zeroReadCount = 0;
        if (sensorFault) {
            sensorFault = false;
            Serial.println("✅ Sensor OK");
        }
    }
}

// =====================================================
// ================= LED PATTERN =======================
// =====================================================

void ledPattern(int mode) {
    switch (mode) {
        case 0: digitalWrite(LED_PIN, LOW);  break;
        case 1: digitalWrite(LED_PIN, HIGH); break;
        case 2:
            if (millis() - lastLedToggle > 800) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState);
                lastLedToggle = millis();
            }
            break;
        case 3:
            if (millis() - lastLedToggle > 200) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState);
                lastLedToggle = millis();
            }
            break;
        case 4:
            if (millis() - lastLedToggle > 100) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState);
                lastLedToggle = millis();
            }
            break;
        case 5:
            if (millis() - lastLedToggle > 150) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState);
                lastLedToggle = millis();
            }
            break;
    }
}

// =====================================================
// ================= ALERT SYSTEM ======================
// =====================================================

void handleAlerts(long dist) {
    if (sensorFault) {
        ledPattern(5);
        if (millis() % 600 < 100) digitalWrite(BUZZER_PIN, HIGH);
        else                       digitalWrite(BUZZER_PIN, LOW);
        return;
    }
    if (dist == 0 || dist > DIST_SAFE) {
        ledPattern(1);
        digitalWrite(BUZZER_PIN, LOW);
        if (lastDistZone != 1) lastDistZone = 1;
        return;
    }
    if (dist > DIST_CAUTION && dist <= DIST_SAFE) {
        ledPattern(2);
        digitalWrite(BUZZER_PIN, LOW);
        if (lastDistZone != 2) lastDistZone = 2;
        return;
    }
    if (dist > DIST_WARNING && dist <= DIST_CAUTION) {
        ledPattern(3);
        if (millis() % 400 < 120) digitalWrite(BUZZER_PIN, HIGH);
        else                       digitalWrite(BUZZER_PIN, LOW);
        if (lastDistZone != 3) lastDistZone = 3;
        return;
    }
    if (dist > DIST_EMERGENCY && dist <= DIST_WARNING) {
        ledPattern(4);
        if (millis() % 200 < 80) digitalWrite(BUZZER_PIN, HIGH);
        else                      digitalWrite(BUZZER_PIN, LOW);
        if (lastDistZone != 4) lastDistZone = 4;
        return;
    }
    if (dist <= DIST_EMERGENCY) {
        ledPattern(0);
        digitalWrite(BUZZER_PIN, HIGH);
        if (lastDistZone != 5) {
            lastDistZone = 5;
            playSound(SND_STOP_OBS, true);
        }
    }
}

// =====================================================
// ================= EXECUTE COMMAND ===================
// =====================================================

void executeCommand(char cmd, long frontDist) {
    if (emergencyMode && cmd != 'X' && cmd != 'S') {
        Serial.println("🚨 Emergency blocked");
        return;
    }
    if (sensorFault && (cmd=='F'||cmd=='L'||cmd=='R')) {
        stopCar(); return;
    }
    switch (cmd) {
        case 'W':
            playSound(SND_EYE_ACTIVE);  // 005 - Eye tracking active
            break;
        case 'E':
            stopCar();
            emergencyMode = true;
            lastMovCmd    = 0;
            playSound(SND_EMERGENCY, true);  // 006 - Emergency Activated
            Serial.println("🚨 EMERGENCY ON");
            break;
        case 'X':
            emergencyMode = false;
            lastMovCmd    = 0;
            lastDistZone  = -1;
            playSound(SND_CONNECTED);  // 002 - Connected
            Serial.println("✅ Emergency OFF");
            break;
        case 'S':
            stopCar();
            lastMovCmd = 0;
            break;
        case 'B':
            moveBackward();
            lastMovCmd = 'B';
            break;
        case 'F': {
            stopCar();
            myServo.write(90); delay(300);
            long sd = readUltrasonic(TRIG2, ECHO2);
            if ((frontDist > DIST_WARNING || frontDist == 0) &&
                (sd > DIST_WARNING || sd == 0)) {
                int spd = SPEED_MAX;
                if (frontDist > 0 && frontDist <= DIST_SAFE)
                    spd = constrain(map(frontDist, DIST_WARNING, DIST_SAFE,
                                        SPEED_MIN, SPEED_MAX), SPEED_MIN, SPEED_MAX);
                moveForward(spd);
                lastMovCmd = 'F';
            } else {
                stopCar();
                playSound(SND_STOP_OBS);  // 004 - Obstacle Detected
            }
            break;
        }
        case 'L': {
            stopCar();
            myServo.write(150); delay(300);
            long sd = readUltrasonic(TRIG2, ECHO2);
            if (sd > DIST_WARNING || sd == 0) {
                turnLeft();
                lastMovCmd = 'L';
            } else {
                stopCar();
                playSound(SND_STOP_OBS);  // 004 - Obstacle Detected
            }
            break;
        }
        case 'R': {
            stopCar();
            myServo.write(30); delay(300);
            long sd = readUltrasonic(TRIG2, ECHO2);
            if (sd > DIST_WARNING || sd == 0) {
                turnRight();
                lastMovCmd = 'R';
            } else {
                stopCar();
                playSound(SND_STOP_OBS);  // 004 - Obstacle Detected
            }
            break;
        }
    }
}

// =====================================================
// ================= SETUP =============================
// =====================================================

void setup() {
    Serial.begin(9600);

    Serial2.begin(115200, SERIAL_8N1, DFP_RX, DFP_TX);
    delay(2000);
    dfSend("AT");
    delay(200);
    dfSend("AT+PROMPT=OFF");
    delay(200);
    dfSend("AT+FUNCTION=1");
    delay(200);
    dfSend("AT+PLAYMODE=3");
    delay(200);
    setVolume(20);

    pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
    pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);
    pinMode(ENA,OUTPUT); pinMode(ENB,OUTPUT);
    pinMode(TRIG1,OUTPUT); pinMode(ECHO1,INPUT);
    pinMode(TRIG2,OUTPUT); pinMode(ECHO2,INPUT);
    pinMode(LED_PIN,    OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(LED_PIN,    LOW);
    digitalWrite(BUZZER_PIN, LOW);

    myServo.attach(SERVO_PIN);
    myServo.write(90);
    stopCar();

    playSound(SND_SYSTEM_ACTIVE, true);  // 001 - EYEonic System Active
    delay(3500);

    WiFi.softAP(ssid, password);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
    udp.begin(UDP_PORT);

    Serial.println("✅ EYEonic Ready");
}

// =====================================================
// ================= LOOP ==============================
// =====================================================

void loop() {
    updateAudio();

    // ===== Read UDP =====
    char incoming  = 0;
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        udp.read(&incoming, 1);
        Serial.printf("CMD: %c\n", incoming);

        if (!clientConnected) {
            clientConnected = true;
            lastCmdMs       = millis();
            playSound(SND_CONNECTED, true);  // 002 - Connected
            Serial.println("✅ Client Connected");
        } else {
            lastCmdMs = millis();
        }
    }

    // ===== Connection Timeout =====
    if (clientConnected && (millis() - lastCmdMs >= CONN_TIMEOUT_MS)) {
        clientConnected = false;
        emergencyMode   = false;
        lastMovCmd      = 0;
        lastDistZone    = -1;
        stopCar();
        playSound(SND_CONN_LOST, true);  // 003 - Connection Lost
        Serial.println("❌ Connection Lost — timeout 60s");
    }

    // ===== Sensors =====
    long frontDist = readUltrasonic(TRIG1, ECHO1);
    long scanDist  = readUltrasonic(TRIG2, ECHO2);
    checkSensorFault(frontDist, scanDist);

    Serial.printf("F:%ld S:%ld Em:%d Fault:%d Conn:%d\n",
                  frontDist, scanDist, emergencyMode, sensorFault, clientConnected);

    // ===== Hardware Emergency < 10cm =====
    if (!sensorFault &&
        ((frontDist > 0 && frontDist <= DIST_EMERGENCY) ||
         (scanDist  > 0 && scanDist  <= DIST_EMERGENCY))) {
        stopCar();
        ledPattern(0);
        digitalWrite(BUZZER_PIN, HIGH);
        if (lastDistZone != 5) {
            lastDistZone = 5;
            playSound(SND_STOP_OBS, true);  // 004 - Obstacle Detected
        }
        return;
    } else {
        if (!emergencyMode) digitalWrite(BUZZER_PIN, LOW);
    }

    // ===== Alerts =====
    if (emergencyMode) {
        ledPattern(3);
        if (millis() % 1000 < 100) digitalWrite(BUZZER_PIN, HIGH);
        else                        digitalWrite(BUZZER_PIN, LOW);
    } else {
        handleAlerts(frontDist);
    }

    // ===== Execute Command =====
    if (incoming != 0) executeCommand(incoming, frontDist);

    delay(30);
}