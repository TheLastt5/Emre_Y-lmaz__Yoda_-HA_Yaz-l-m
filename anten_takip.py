import System
from System.IO.Ports import SerialPort

# ================= AYARLAR =================
# Anten Takip Arduino'sunun bilgisayara bağlı olduğu port
ARDUINO_COM_PORT = "COM3"  
BAUD_RATE = 115200         
GONDERIM_ARALIGI_MS = 500 # Yarım saniyede bir veri gönderir
# ===========================================

print("Anten Takip Scripti Baslatiliyor...")

try:
    # Seri portu başlat (.NET SerialPort kullanılarak)
    port = SerialPort(ARDUINO_COM_PORT, BAUD_RATE)
    port.Open()
    print(ARDUINO_COM_PORT + " portuna basariyla baglanildi.")
    
    # Sonsuz döngü (Script durdurulana kadar çalışır)
    while True:
        # Mission Planner'ın dahili 'cs' objesinden dronun konumunu al
        drone_lat = cs.lat
        drone_lon = cs.lng
        
        # Dronun GPS'i kilitlenmiş mi kontrol et (Konum 0,0 değilse)
        if drone_lat != 0 and drone_lon != 0:
            # Arduino'nun kolayca okuyup bölebilmesi için "Enlem,Boylam\n" formatı
            veri_paketi = "{:.6f},{:.6f}\n".format(drone_lat, drone_lon)
            
            # Veriyi kablo üzerinden Arduino'ya yolla
            port.Write(veri_paketi)
            print("Drone Konumu Karta Yollandi: " + veri_paketi.strip())
        else:
            print("Dronun GPS baglantisi bekleniyor (Fix yok)...")
            
        # Arayüzü dondurmamak için MP'nin Script.Sleep'ini kullanıyoruz
        Script.Sleep(GONDERIM_ARALIGI_MS)

except Exception as e:
    print("Hata Olustu: " + str(e))
    print("Lutfen Arduino'nun bagli oldugundan emin olun.")

finally:
    if 'port' in locals() and port.IsOpen:
        port.Close()
        print("Seri port guvenli bir sekilde kapatildi.")
