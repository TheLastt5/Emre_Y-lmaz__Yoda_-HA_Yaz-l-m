#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// CNC Shield Y-Ekseni Pinleri
const int stepYPin = 3;
const int dirYPin = 6;
const int enablePin = 8;

// GPS SoftwareSerial Pinleri (Yer İstasyonu GPS'i)
const int RXPin = 10; // GPS TX buraya bağlanır
const int TXPin = 11; // GPS RX buraya bağlanır
const uint32_t GPSBaud = 9600; // Ublox M8N varsayılan baud hızı

TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// Başlangıç (Yer İstasyonu) konumu değişkenleri
bool baseLocationAcquired = false;
double baseLat = 0.0;
double baseLng = 0.0;

// Python'dan (Bilgisayardan) gelecek anlık drone konumları
double droneLat = 0.0;
double droneLng = 0.0;
float targetAngle = 0.0;        // Dinamik hesaplanan hedef açı
bool firstTargetReceived = false; // Python'dan veri akışı başladı mı?

// Motor hareket toleransı
const float deadzone = 4.0; // 4 derecelik ölü bölge (Gereksiz titremeyi engeller)
float currentAngle = 0.0; 

void moveMotor(int dirPin, int stepPin, int direction);

void setup() {
  Serial.begin(115200);     // Python'dan gelen verileri OKUMAK için (Hızlar eşitlendi)
  gpsSerial.begin(GPSBaud); // Yer istasyonundaki kendi GPS modülümüz için

  // Motor pin kurulumları
  pinMode(stepYPin, OUTPUT);
  pinMode(dirYPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW); // Motor sürücülerini aktif et (LOW = Enable)

  Serial.println("Arduino Sistemi baslatildi. Yer GPS'i uydulari ariyor...");
}

void loop() {
  // -------------------------------------------------------------------------
  // ADIM 1: Yer İstasyonunun Sabit Konumunu Alma (Sistem açılınca 1 kez)
  // -------------------------------------------------------------------------
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (!baseLocationAcquired && gps.location.isValid()) {
        baseLat = gps.location.lat();
        baseLng = gps.location.lng();
        baseLocationAcquired = true;
        
        Serial.println("\n--- YER (BAZ) KONUMU SAKLANDI ---");
        Serial.print("Enlem: "); Serial.println(baseLat, 6);
        Serial.print("Boylam: "); Serial.println(baseLng, 6);
        Serial.println("---------------------------------");
        Serial.println("Yer sabitlendi. Bilgisayardan (Python) veri bekleniyor...");
      }
    }
  }

  // -------------------------------------------------------------------------
  // ADIM 2: Bilgisayardan (Python) Gelen Verileri Dinleme ve Açı Hesaplama
  // -------------------------------------------------------------------------
  // Kendi konumumuzu aldıysak ve bilgisayardan veri akışı başladıysa içeri girer
  if (baseLocationAcquired && Serial.available() > 0) {
    
    // Satır sonu karakterine (\n) kadar gelen "Enlem,Boylam" metnini oku
    String incomingData = Serial.readStringUntil('\n');
    incomingData.trim();
    
    int commaIndex = incomingData.indexOf(',');
    
    // Eğer gelen veri kurallara uygunsa (virgül içeriyorsa) parçala
    if (commaIndex > 0) {
      droneLat = incomingData.substring(0, commaIndex).toDouble();
      droneLng = incomingData.substring(commaIndex + 1).toDouble();
      
      // TAMİR EDİLEN NOKTA: Sabit 90 derece silindi. 
      // Kendi GPS'imiz ile Dronun GPS'i arasındaki coğrafi pusula açısı hesaplanıyor.
      targetAngle = gps.courseTo(baseLat, baseLng, droneLat, droneLng);
      firstTargetReceived = true;
    }
  }

  // -------------------------------------------------------------------------
  // ADIM 3: Motoru Hedef Açıya Doğru Çevirme
  // -------------------------------------------------------------------------
  if (baseLocationAcquired && firstTargetReceived) {
    
    float errorAngle = targetAngle - currentAngle;

    // Açısal hata tolerans sınırından büyükse motoru adım attır
    if (abs(errorAngle) > deadzone) {
      if (errorAngle > 0) {
        moveMotor(dirYPin, stepYPin, HIGH); // Saat yönü dön
        currentAngle += 0.1; // Motorunuzun adım/derece kalibrasyonuna göre güncelleyebilirsiniz
      } else {
        moveMotor(dirYPin, stepYPin, LOW);  // Saat yönünün tersine dön
        currentAngle -= 0.1;
      }
    }
  }
}

// Step motor sürme fonksiyonu
void moveMotor(int dirPin, int stepPin, int direction) {
  digitalWrite(dirPin, direction);
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(800); // Motor dönüş hızı (Küçüldükçe hızlanır)
  digitalWrite(stepPin, LOW);
  delayMicroseconds(800);
}
