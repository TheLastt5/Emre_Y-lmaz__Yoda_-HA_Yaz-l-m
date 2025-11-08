import string # Noktalama işaretlerini temizlemek için string modülünü içe aktarıyoruz

def kelime_sayaci():
    
    metin = input("Lütfen bir cümle veya paragraf giriniz: ")

    
    metin_kucuk = metin.lower()
    # string.punctuation, tüm noktalama işaretlerini içerir.
    temiz_metin = metin_kucuk.translate(str.maketrans('', '', string.punctuation))

    # Kelimeleri ayırmak için metni boşluklara göre bölüyoruz.
    kelime_listesi = temiz_metin.split()

    # 1. Toplam Kelime Sayısı
    toplam_kelime_sayisi = len(kelime_listesi)

    # 2. Toplam Karakter Sayısı (Boşluklar dahil)
    toplam_karakter_sayisi = len(metin)

    # 3. En Uzun Kelimenin Uzunluğu
    en_uzun_uzunluk = 0 # Başlangıçta en uzun kelime uzunluğunu 0 olarak belirliyoruz
    if kelime_listesi: # Metinde kelime olup olmadığını kontrol ediyoruz (boş metin durumunda hata vermemesi için
        en_uzun_kelime = max(kelime_listesi, key=len)
        en_uzun_uzunluk = len(en_uzun_kelime)
    
    # 4. Her Bir Kelimenin Tekrar Sayısı (Sözlük Kullanımı)
    kelime_sikliklari = {} # Kelime ve tekrar sayısını tutacak boş bir sözlük (dictionary) oluşturuyoruz
    for kelime in kelime_listesi:
        # Kelime zaten sözlükte varsa değerini 1 artırıyoruz (Tekrar etti)
        if kelime in kelime_sikliklari:
            kelime_sikliklari[kelime] += 1
        # Kelime sözlükte yoksa, yeni bir giriş oluşturup değerini 1 olarak ayarlıyoruz
        else:
            kelime_sikliklari[kelime] = 1

    # --- Sonuçların Ekrana Yazdırılması ---

    print("\n METİN İSTATİSTİKLERİ ")
    print(f"1. Toplam Kelime Sayısı: {toplam_kelime_sayisi}")
    print(f"2. Toplam Karakter Sayısı (Boşluklar Dahil): {toplam_karakter_sayisi}")
    print(f"3. En Uzun Kelimenin Uzunluğu: {en_uzun_uzunluk}")
    
    print("\n4. Kelime Tekrarları:")
    # Sözlükteki her bir kelimeyi ve tekrar sayısını ekrana yazdırıyoruz
    for kelime, sayi in kelime_sikliklari.items():
        print(f"   - '{kelime}': {sayi} kez")
    
    # Eğer metin boşsa kullanıcıya bilgi veriyoruz
    if not metin:
        print("\nUyarı: Metin girişi boştu, istatistikler 0'dır.")

kelime_sayaci()