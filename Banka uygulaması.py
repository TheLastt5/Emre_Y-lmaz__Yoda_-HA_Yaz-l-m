class Hesap:
    """
    Hesap sahibinin bilgileri, hesap numarası ve bakiye gibi özellikleri tutar.
    """
    
    # Kurucu Metot (Constructor)
    # Yeni bir Hesap nesnesi oluşturulduğunda ilk çalışır.
    def __init__(self, sahip_adi, hesap_no, bakiye=1000):
        self.sahip_adi = sahip_adi
        self.hesap_no = hesap_no
        self.bakiye = bakiye 

    # Görseldeki menü fonksiyonu
    def menu(self):
        """Kullanıcıya seçenekleri sunar."""
        print("\n--- İŞLEMLER ---")
        print("Para yatırmak için 1'i tuşlayın.")
        print("Para çekmek için 2'yi tuşlayın.")
        print("Hesabınızın bakiyesini öğrenmek için 3'ü tuşlayın.")
        print("Çıkış yapmak için 4'ü tuşlayın.")
    
    # Görseldeki para_yatir fonksiyonunun sınıf versiyonu
    def para_yatir(self, miktar):
        """Belirtilen miktarı hesaba yatırır ve 500 TL limitini kontrol eder."""
        if miktar <= 0:
            print("Geçersiz tutar.")
            return

        if miktar > 500: # Görseldeki 500 TL limitini koruduk
            print("Günlük yatırma limiti 500 TL'dir.")
            return

        self.bakiye += miktar
        print(f"Başarıyla {miktar:.2f} TL yatırıldı.")
    
    # Görseldeki para_cek fonksiyonunun sınıf versiyonu
    def para_cek(self, miktar):
        """Belirtilen miktarı hesaptan çeker, yeterli bakiye kontrolü yapar."""
        if miktar <= 0:
            print("Geçersiz tutar.")
            return
            
        if self.bakiye < miktar:
            print("Hesabınızda yeteri kadar para bulunmamaktadır.")
            return

        self.bakiye -= miktar
        print(f"Başarıyla {miktar:.2f} TL çekildi.")
    
    # Bakiye görüntüleme
    def bakiye_goruntule(self):
        print(f"Mevcut Bakiyeniz: {self.bakiye:.2f} TL")


def ana_program():
    """Kullanıcı girişi ile hesap yönetimini başlatan ana akış."""
    
    # Kullanıcıdan gerekli bilgiler alınır
    sahip = input("Hesap Sahibi Adını Girin: ")
    numara = input("Hesap Numarasını Girin: ")
    
    # Yeni Hesap nesnesi oluşturuluyor
    kullanici_hesabi = Hesap(sahip, numara)
    print(f"\nHesap Oluşturuldu! Sahip: {sahip}, No: {numara}, Başlangıç Bakiyesi: 1000.00 TL")

    while True:
        kullanici_hesabi.menu()
        
        try:
            secim = int(input("Seçiminiz: "))
            
            if secim == 1:
                miktar = float(input("Yatırmak istediğiniz miktarı giriniz: "))
                kullanici_hesabi.para_yatir(miktar)
                
            elif secim == 2:
                miktar = float(input("Çekmek istediğiniz miktarı giriniz: "))
                kullanici_hesabi.para_cek(miktar)
                
            elif secim == 3:
                kullanici_hesabi.bakiye_goruntule()
                
            elif secim == 4:
                print("Çıkış yapılıyor. İyi günler dileriz.")
                break # Döngüden çıkış
                
            else:
                print("Lütfen geçerli bir sayı giriniz.")
                
        except ValueError:
            print("Hata: Lütfen sadece sayısal bir değer giriniz.")
        
# Programı başlatma
ana_program()