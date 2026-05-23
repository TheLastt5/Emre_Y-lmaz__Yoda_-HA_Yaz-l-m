#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// ================= TEST / UÇUŞ MODU AYARI =================
// Eğer elinizde İHA yoksa ve sadece motor testi yapacaksanız burayı true yapın.
// Gerçek uçuşta Python'dan veri almak için burayı false yapın.
const bool TEST_MODU = true; 
// ==========================================================

// CNC Shield Y-Ekseni Pinleri
const int stepYPin = 3;
const int dirYPin = 6;
const int enablePin = 8;

// GPS SoftwareSerial Pinleri
const int RXPin = 10; 
const int TXPin = 11; 
const uint32_t GPSBaud = 9600; 

TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

bool baseLocationAcquired = false;
double baseLat = 0.0;
double baseLng = 0.0;

double droneLat = 0.0;
double droneLng = 0.0;
float targetAngle = 0.0;        
bool firstTargetReceived = false; 

// Motor hareket toleransı
const float deadzone = 4.0; 
float currentAngle = 0.0; 

void moveMotor(int dirPin, int stepPin, int direction);

void setup() {
  Serial.begin(115200);     
  gpsSerial.begin(GPSBaud); 

  pinMode(stepYPin, OUTPUT);
  pinMode(dirYPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW); 

  Serial.println("Sistem baslatildi...");
  if (TEST_MODU) {
    Serial.println("[MOD] TEST MODU AKTIF: Sabit 90 dereceye donus testi yapilacak.");
  } else {
    Serial.println("[MOD] UCUS MODU AKTIF: Yer GPS'i ve Python verisi bekleniyor.");
  }
}

void loop() {
  // -------------------------------------------------------------------------
  // SENARYO A: TEST MODU (İHA yoksa direkt çalışır)
  // -------------------------------------------------------------------------
  if (TEST_MODU) {
    // GPS ve Python beklemeden direkt hedefi 90 dereceye kilitler
    targetAngle = 90.0; 
    
    float errorAngle = targetAngle - currentAngle;

    if (abs(errorAngle) > deadzone) {
      if (errorAngle > 0) {
        moveMotor(dirYPin, stepYPin, HIGH); 
        currentAngle += 0.1; 
      } else {
        moveMotor(dirYPin, stepYPin, LOW);  
        currentAngle -= 0.1;
      }
      
      // Seri porttan anlık motor konumunu izleyebilirsiniz
      Serial.print("Motor Donuyor... Guncel Aci: "); Serial.println(currentAngle);
    } else {
      // Hedefe ulaşıldığında motor durur ve haber verir
      static bool hedefMesajiVerildi = false;
      if (!hedefMesajiVerildi) {
        Serial.println("-> HEDEF BASARIYLA YAKALANDI: Anten 90 derecede sabitlendi!");
        hedefMesajiVerildi = true;
      }
    }
    return; // Test modundaysa aşağıdaki karmaşık GPS kodlarına hiç girme, döngü başa dönsün
  }

  // -------------------------------------------------------------------------
  // SENARYO B: GERÇEK UÇUŞ MODU (TEST_MODU = false ise burası çalışır)
  // -------------------------------------------------------------------------
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (!baseLocationAcquired && gps.location.isValid()) {
        baseLat = gps.location.lat();
        baseLng = gps.location.lng();
        baseLocationAcquired = true;
        Serial.println("\n--- YER (BAZ) KONUMU SAKLANDI ---");
      }
    }
  }

  if (baseLocationAcquired && Serial.available() > 0) {
    String incomingData = Serial.readStringUntil('\n');
    incomingData.trim();
    int commaIndex = incomingData.indexOf(',');
    if (commaIndex > 0) {
      droneLat = incomingData.substring(0, commaIndex).toDouble();
      droneLng = incomingData.substring(commaIndex + 1).toDouble();
      targetAngle = gps.courseTo(baseLat, baseLng, droneLat, droneLng);
      firstTargetReceived = true;
    }
  }

  if (baseLocationAcquired && firstTargetReceived) {
    float errorAngle = targetAngle - currentAngle;
    if (abs(errorAngle) > deadzone) {
      if (errorAngle > 0) {
        moveMotor(dirYPin, stepYPin, HIGH);
        currentAngle += 0.1;
      } else {
        moveMotor(dirYPin, stepYPin, LOW);
        currentAngle -= 0.1;
      }
    }
  }
}

void moveMotor(int dirPin, int stepPin, int direction) {
  digitalWrite(dirPin, direction);
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(800); 
  digitalWrite(stepPin, LOW);
  delayMicroseconds(800);
}
