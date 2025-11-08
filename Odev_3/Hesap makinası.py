def hesap_makinesi():

    print("--- Hesap Makinesi Başlatıldı ---")

    # Kullanıcı çıkmak isteyene kadar programın çalışmasını sağlayan ana döngü
    while True:
        try:
            # 1. Kullanıcıdan sayıları al ve float'a çevir
            sayi1 = float(input("\nBirinci sayıyı girin: "))
            sayi2 = float(input("İkinci sayıyı girin: "))

            # 2. Kullanıcıdan işlemi al
            islem = input("İşlemi seçin (+, -, *, /): ")

            sonuc = None # Sonucu tutacak değişken (None ile başlatıldı)

            # 3. Seçilen işlemi gerçekleştirme
            if islem == '+':
                sonuc = sayi1 + sayi2
            elif islem == '-':
                sonuc = sayi1 - sayi2
            elif islem == '*':
                sonuc = sayi1 * sayi2
            elif islem == '/':
                # Sıfıra bölme kontrolü
                if sayi2 == 0:
                    print("Hata: Sıfıra bölme yapılamaz.")
                    # Bu işlemi atlayıp döngünün başına dön
                    continue
                sonuc = sayi1 / sayi2
            else:
                print("Hata: Geçersiz işlem.")
                # Bu işlemi atlayıp döngünün başına dön
                continue

            
            # Geçersiz işlem veya sıfıra bölme hatası yoksa sonuç yazdırılır
            if sonuc is not None:
                print(f"\nSonuç: {sayi1} {islem} {sayi2} = {sonuc}")
                
        except ValueError:
            # Kullanıcı sayı yerine metin girerse bu hata yakalanır
            print("\nHata: Lütfen geçerli sayı formatında giriş yapın.")
            
        except Exception as e:
            # Diğer olası hatalar için genel yakalama bloğu
            print(f"\nBeklenmedik bir hata oluştu: {e}")
            
        # 5. Başka işlem yapmak isteyip istemediğini sorma
        devam_et = input("\nBaşka bir işlem yapmak ister misiniz? (E/H): ").upper()

        # SADECE 'H' GİRİLİRSE ÇIKILACAK ŞEKİLDE DÜZELTİLDİ
        if devam_et == 'H':
            print("\nHesap makinesi kapatılıyor. Güç seninle olsun!")
            break

# Programı başlatma
hesap_makinesi()