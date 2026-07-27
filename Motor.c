#include <Arduino.h>

// =============================================================================
// Donanımsal Seri Port (UART1) Tanımlaması
// =============================================================================
HardwareSerial Serial1(PA10, PA9); 

// =============================================================================
// Motor Pin Tanımlamaları (GÖRSEL SIRAYA GÖRE DÜZELTİLDİ: Soldan Sağa)
// =============================================================================
// Soldan 1. Konnektör (Fiziksel Görünüm: Motor 1)
#define M1_PWM PB10
#define M1_IN1 PB0
#define M1_IN2 PB1

// Soldan 2. Konnektör (Fiziksel Görünüm: Motor 2)
#define M2_PWM PA5
#define M2_IN1 PA7
#define M2_IN2 PA6

// Soldan 3. Konnektör (Fiziksel Görünüm: Motor 3)
#define M3_PWM PA15
#define M3_IN1 PB4
#define M3_IN2 PB3

// Soldan 4. Konnektör (Fiziksel Görünüm: Motor 4)
#define M4_PWM PB9
#define M4_IN1 PB5
#define M4_IN2 PB8

// =============================================================================
// Durum LED'leri ve Buton
// =============================================================================
#define LED_BLUE PC13
#define LED_R    PB13
#define LED_G    PB14
#define LED_B    PB15
#define BUTTON_USER PA0

// =============================================================================
// Güvenlik, Zamanlama ve Test Modu Ayarları
// =============================================================================
#define COMMAND_TIMEOUT_MS 500
unsigned long lastCommandTime = 0;

unsigned long greenLedOffTime = 0;
bool greenLedActive = false;

// Test Modu Değişkenleri
unsigned long buttonPressTime = 0;
bool isButtonPressed = false;
bool longPressHandled = false;
int testMotorIndex = 0; // 0=M1, 1=M2, 2=M3, 3=M4
int testMotorDir = 1;
bool testModeOverride = false; 

// =============================================================================
// Yumuşak Duruş/Kalkış ve Yazılımsal PWM Ayarları
// =============================================================================
#define RAMP_INTERVAL_MS 5  
#define RAMP_STEP 20        
#define SOFT_PWM_PERIOD_US 2000 // 500Hz frekansında yazılımsal PWM

unsigned long lastRampTime = 0;

int targetSpeeds[4]  = {0, 0, 0, 0}; 
int currentSpeeds[4] = {0, 0, 0, 0}; 
int currentDirs[4]   = {1, 1, 1, 1}; 
int targetDirs[4]    = {1, 1, 1, 1}; 

String inputBuffer = "";

// =============================================================================
// Donanıma Hız ve Yön Uygulama
// =============================================================================
void applyMotorHardware(int motorIndex, int direction, int speed) {
    int in1, in2, pwm_pin;

    switch (motorIndex) {
        case 0: in1 = M1_IN1; in2 = M1_IN2; pwm_pin = M1_PWM; break;
        case 1: in1 = M2_IN1; in2 = M2_IN2; pwm_pin = M2_PWM; break;
        case 2: in1 = M3_IN1; in2 = M3_IN2; pwm_pin = M3_PWM; break;
        case 3: in1 = M4_IN1; in2 = M4_IN2; pwm_pin = M4_PWM; break;
        default: return;
    }

    if (speed == 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, HIGH);
        // Sadece donanımsal timer kullanan kenar motorları analogWrite ile sıfırla
        if (motorIndex == 0 || motorIndex == 3) analogWrite(pwm_pin, 0);
        return;
    }

    if (direction == 1) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
    } else {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
    }
    
    // M1 ve M4 donanımsal olarak sorunsuz sürülür.
    // M2 ve M3 ana döngüde yazılımsal olarak sürülür.
    if (motorIndex == 0 || motorIndex == 3) analogWrite(pwm_pin, speed);
}

// =============================================================================
// Motor Hedef Belirleme 
// =============================================================================
void setMotorTarget(int motorIndex, int direction, int speed) {
    if (motorIndex < 0 || motorIndex > 3) return;
    
    if (direction != 0 && direction != 1) {
        Serial1.println("ERR:INVALID_DIRECTION");
        targetSpeeds[motorIndex] = 0;
        return;
    }

    targetSpeeds[motorIndex] = constrain(speed, 0, 255);
    targetDirs[motorIndex] = direction; 
}

// =============================================================================
// Tüm Motorları Yumuşak Durdurma Komutu
// =============================================================================
void softStopAllMotors() {
    for (int i = 0; i < 4; i++) {
        targetSpeeds[i] = 0;
        targetDirs[i] = currentDirs[i]; 
        currentSpeeds[i] = 0;                      
        applyMotorHardware(i, currentDirs[i], 0);   
    }
    
    digitalWrite(LED_BLUE, LOW);   
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);
}

// =============================================================================
// Test Modu RGB LED Güncelleme Yardımcısı
// =============================================================================
void updateTestModeLED(int index) {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);

    switch(index) {
        case 0: digitalWrite(LED_R, LOW); break; // M1: Kırmızı
        case 1: digitalWrite(LED_G, LOW); break; // M2: Yeşil
        case 2: digitalWrite(LED_B, LOW); break; // M3: Mavi
        case 3: 
            digitalWrite(LED_R, LOW); 
            digitalWrite(LED_G, LOW); 
            break; // M4: Sarı
    }
}

// =============================================================================
// Setup
// =============================================================================
void setup() {
    Serial1.begin(115200);

    digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, LOW);
    digitalWrite(M2_IN1, LOW); digitalWrite(M2_IN2, LOW);
    digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, LOW);
    digitalWrite(M4_IN1, LOW); digitalWrite(M4_IN2, LOW);

    pinMode(M1_PWM, OUTPUT); pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
    pinMode(M2_PWM, OUTPUT); pinMode(M2_IN1, OUTPUT); pinMode(M2_IN2, OUTPUT);
    pinMode(M3_PWM, OUTPUT); pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);
    pinMode(M4_PWM, OUTPUT); pinMode(M4_IN1, OUTPUT); pinMode(M4_IN2, OUTPUT);

    // Başlangıçta tüm motorları kapalı konuma al.
    // M2 ve M3 için analogWrite KULLANILMIYOR, donanım kilitlenmemesi için.
    digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, LOW); analogWrite(M1_PWM, 0);
    digitalWrite(M2_IN1, LOW); digitalWrite(M2_IN2, LOW); digitalWrite(M2_PWM, LOW);
    digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, LOW); digitalWrite(M3_PWM, LOW);
    digitalWrite(M4_IN1, LOW); digitalWrite(M4_IN2, LOW); analogWrite(M4_PWM, 0);

    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_R, OUTPUT); pinMode(LED_G, OUTPUT); pinMode(LED_B, OUTPUT);
    
    pinMode(BUTTON_USER, INPUT_PULLDOWN);

    softStopAllMotors();
    
    lastCommandTime = millis();
    lastRampTime = millis();
}

// =============================================================================
// Loop
// =============================================================================
void loop() {
    unsigned long currentMillis = millis();
    unsigned long currentMicros = micros();

    // -------------------------------------------------------------------------
    // Mekanizma 0: Motor 2 ve Motor 3 için YAZILIMSAL PWM (Donanım Kilit Çözümü)
    // -------------------------------------------------------------------------
    unsigned long cycleTime = currentMicros % SOFT_PWM_PERIOD_US;

    // Motor 2 Yazılımsal PWM
    if (currentSpeeds[1] == 0) {
        digitalWrite(M2_PWM, LOW);
    } else if (currentSpeeds[1] == 255) {
        digitalWrite(M2_PWM, HIGH);
    } else {
        unsigned long onTime = map(currentSpeeds[1], 0, 255, 0, SOFT_PWM_PERIOD_US);
        if (cycleTime < onTime) digitalWrite(M2_PWM, HIGH);
        else digitalWrite(M2_PWM, LOW);
    }

    // Motor 3 Yazılımsal PWM
    if (currentSpeeds[2] == 0) {
        digitalWrite(M3_PWM, LOW);
    } else if (currentSpeeds[2] == 255) {
        digitalWrite(M3_PWM, HIGH);
    } else {
        unsigned long onTime = map(currentSpeeds[2], 0, 255, 0, SOFT_PWM_PERIOD_US);
        if (cycleTime < onTime) digitalWrite(M3_PWM, HIGH);
        else digitalWrite(M3_PWM, LOW);
    }

    // -------------------------------------------------------------------------
    // Buton Kontrolü (Test Modu)
    // -------------------------------------------------------------------------
    bool currentButtonState = (digitalRead(BUTTON_USER) == HIGH);

    if (currentButtonState && !isButtonPressed) {
        buttonPressTime = currentMillis;
        isButtonPressed = true;
        longPressHandled = false;
    } 
    else if (currentButtonState && isButtonPressed) {
        if (!longPressHandled && (currentMillis - buttonPressTime >= 600)) {
            longPressHandled = true;
            testModeOverride = true; 
            
            softStopAllMotors(); 
            testMotorIndex = (testMotorIndex + 1) % 4;
            
            updateTestModeLED(testMotorIndex);
            
            setMotorTarget(testMotorIndex, testMotorDir, 255);
            
            Serial1.print("TEST MODU: M");
            Serial1.print(testMotorIndex + 1);
            Serial1.println(" SECILDI");
        }
    } 
    else if (!currentButtonState && isButtonPressed) {
        isButtonPressed = false;
        
        if (!longPressHandled) {
            if (!testModeOverride) {
                updateTestModeLED(testMotorIndex); 
            }
            
            testModeOverride = true; 
            testMotorDir = (testMotorDir == 1) ? 0 : 1;
            
            setMotorTarget(testMotorIndex, testMotorDir, 255);
            
            Serial1.print("TEST MODU: M");
            Serial1.print(testMotorIndex + 1);
            Serial1.println(testMotorDir == 1 ? " ILERI" : " GERI");
        }
    }

    // -------------------------------------------------------------------------
    // Mekanizma 1: Rampa Motoru
    // -------------------------------------------------------------------------
    if (currentMillis - lastRampTime >= RAMP_INTERVAL_MS) {
        lastRampTime = currentMillis;
        
        for (int i = 0; i < 4; i++) {
            if (currentDirs[i] != targetDirs[i]) {
                if (currentSpeeds[i] > 0) {
                    currentSpeeds[i] -= RAMP_STEP;
                    if (currentSpeeds[i] < 0) currentSpeeds[i] = 0;
                }
                if (currentSpeeds[i] == 0) {
                    currentDirs[i] = targetDirs[i]; 
                }
            } 
            else {
                if (currentSpeeds[i] < targetSpeeds[i]) {
                    currentSpeeds[i] += RAMP_STEP;
                    if (currentSpeeds[i] > targetSpeeds[i]) currentSpeeds[i] = targetSpeeds[i];
                } else if (currentSpeeds[i] > targetSpeeds[i]) {
                    currentSpeeds[i] -= RAMP_STEP;
                    if (currentSpeeds[i] < targetSpeeds[i]) currentSpeeds[i] = targetSpeeds[i];
                }
            }
            applyMotorHardware(i, currentDirs[i], currentSpeeds[i]);
        }
    }

    // -------------------------------------------------------------------------
    // Mekanizma 2: Watchdog (İletişim Kopma Koruması)
    // -------------------------------------------------------------------------
    if (!testModeOverride) {
        bool anyMotorActive = false;
        for (int i = 0; i < 4; i++) {
            if (targetSpeeds[i] > 0 || currentSpeeds[i] > 0) {
                anyMotorActive = true;
                break;
            }
        }

        if (anyMotorActive && (currentMillis - lastCommandTime > COMMAND_TIMEOUT_MS)) {
            softStopAllMotors(); 
            Serial1.println("ERR:TIMEOUT_SOFT_STOPPING");
            lastCommandTime = currentMillis; 
        }
    }

    // -------------------------------------------------------------------------
    // Mekanizma 3: Gecikmesiz Durum LED Yönetimi
    // -------------------------------------------------------------------------
    if (greenLedActive && (currentMillis >= greenLedOffTime)) {
        digitalWrite(LED_G, HIGH); 
        greenLedActive = false;
    }

   // -------------------------------------------------------------------------
    // Mekanizma 4: Karakter Bazlı Non-blocking UART Dinleyici (Gürültü Filtreli)
    // -------------------------------------------------------------------------
    while (Serial1.available() > 0) {
        char inChar = (char)Serial1.read();
        
        if (inChar == '\n') {
            inputBuffer.trim();
            if (inputBuffer.length() >= 3) { // En az "M1 1 1" kadar uzun olmalı
                
                // M harfinin konumunu bul
                int mIndex = inputBuffer.indexOf('M');
                if (mIndex != -1) {
                    // M'den sonrasını al ve başındaki S veya diğer gürültüleri at
                    String payload = inputBuffer.substring(mIndex + 1);
                    payload.trim(); // Boşlukları temizle
                    
                    // Artık elimizde temiz bir "1 1 100" veya "S1 1 100" var
                    // S harfi varsa onu da temizle
                    if (payload.startsWith("S")) payload = payload.substring(1);
                    payload.trim();

                    int motorId, dir, speed;
                    if (sscanf(payload.c_str(), "%d %d %d", &motorId, &dir, &speed) == 3) {
                        
                        // Motor Id 1-4 arası mı?
                        if (motorId >= 1 && motorId <= 4) {
                            if (speed > 0) digitalWrite(LED_BLUE, HIGH); 
                            
                            setMotorTarget(motorId - 1, dir, speed);
                            lastCommandTime = currentMillis;

                            Serial1.println("OK");
                        } else {
                            Serial1.println("ERR:INVALID_MOTOR_ID");
                        }
                    } else {
                        Serial1.println("ERR:BAD_FORMAT");
                    }
                }
            }
            inputBuffer = "";
        } else {
            if (inputBuffer.length() < 32) inputBuffer += inChar;
        }
    }
    }
