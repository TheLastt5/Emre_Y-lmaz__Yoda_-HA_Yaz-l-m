import cv2
import time
from collections import deque 
import numpy as np # Gerekli olmasa da bazı OpenCV işlemlerinde kullanılabilir.

# AYARLAR

FACE_CASCADE = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
# Renkler
BOX_COLOR = (255, 0, 0)
TRACE_COLOR = (255, 0, 0) 
TEXT_COLOR = (255, 255, 255) 
COUNTER_COLOR = (0, 255, 0) 

center_points_history = deque(maxlen=60)

def main(source=0):
    """
    OpenCV yüz takip uygulamasını başlatır.
    source: 0 (canlı kamera) veya 'video_dosyasi.mp4' (video dosya yolu).
    """
    
    # Kamerayı/Videoyu Başlat
    cap = cv2.VideoCapture(source)

    if not cap.isOpened():
        print(f"HATA: Kaynak ({source}) açılamadı. Kamera takılı mı veya dosya yolu doğru mu?")
        return

    # Sayaç değişkenleri
    face_detected = False
    start_time = None
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Video akışı/dosya sonu.")
            break

        # İşlem için gri Tonlamaya Çevir
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Yüzleri Tespit Et
        faces = FACE_CASCADE.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

        # O anki karede yüz tespit edildi mi
        current_face_detected = len(faces) > 0

        # --- Sayaç Mekanizması Kontrolü (Görev 4.2) ---
        if current_face_detected and not face_detected:
            # Yüz ilk kez tespit edildi: Sayacı başlat
            start_time = time.time()
            face_detected = True
        elif not current_face_detected:
            # Yüz kayboldu: Sayacı sıfırla [cite: 20] ve geçmişi temizle
            start_time = None
            face_detected = False
            center_points_history.clear() 

        for (x, y, w, h) in faces:
            # 1. Bounding Box Çiz (Mavi) 
            cv2.rectangle(frame, (x, y), (x + w, y + h), BOX_COLOR, 2)

            # Merkez Koordinatlarını Hesapla
            center_x = x + w // 2
            center_y = y + h // 2
            current_center = (center_x, center_y)
            
            # Merkez noktayı geçmişe ekle
            center_points_history.append(current_center) # Koordinatlar kaydediliyor 

            # 2. Merkez Koordinatlarını Sol Üst Köşeye Yazdır 
            center_text = f"Merkez: ({center_x}, {center_y})"
            cv2.putText(frame, center_text, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, TEXT_COLOR, 2, cv2.LINE_AA)
            
        if len(center_points_history) > 1:
            for i in range(1, len(center_points_history)):
                pt1 = center_points_history[i - 1]
                pt2 = center_points_history[i]
                # Mavi Çizgi Çiz
                cv2.line(frame, pt1, pt2, TRACE_COLOR, 2)

        # Sayaç Süresini 
        
        detection_duration_text = "Yuz Yok"
        if face_detected and start_time is not None:
            # Yüz kaç saniyedir tespit edildi [cite: 19]
            elapsed_time = time.time() - start_time
            detection_duration_text = f"Sure: {elapsed_time:.1f} sn"
        
        # Sağ üst köşeye sayaç metnini yazdırma
        cv2.putText(frame, detection_duration_text, (frame.shape[1] - 250, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.9, COUNTER_COLOR, 2, cv2.LINE_AA)

        # Kareyi Görüntüle
        cv2.imshow('YODA IHA Yuz Takip (Asama 4)', frame)

        # 'q' tuşuna basıldığında çık
        if cv2.waitKey(1) & 0xFF == ord('d'):
            break

    # Uygulama bittiğinde kaynakları serbest bırak
    cap.release()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    # Canlı kamera ile başlatmak için bu satırı kullanın (source=0)
    main(source=0) 
    
