import random
sayı = random.randint(1, 100) # sistem random bir sayı tutsun diye random komutunu kullandım
hak = 10
tahmin_sayısı = 0 # Kaç tahmin yapıldığını saymak için yaptım

while hak > 0:
    tahmin_sayısı += 1 # Her döngüde yapılan tahmin sayısını 1 artır

    tahmin = int(input("Tahmininiz: "))

    if tahmin == sayı:
        # Kazanma mesajına kaçıncı denemede bildiğini ekledik
        print(f"Tebrikler, {tahmin_sayısı}. denemede bildiniz!")
        break

    elif tahmin > sayı: # kişi sayıyı bilemezse aşağı ve yukarı komutları için elif ve else ekledim
        print("Aşağı.")
    else:
        print("Yukarı.")

    hak -= 1 # hakkın giderek azalmasını sağladım

    # Kalan hak ve yapılan tahmin bilgisini göstermesi için kodu yazdım
    if hak > 0:
        print(f"Kalan hakkınız: {hak}. Toplam {tahmin_sayısı} tahmin yaptınız.")

    if hak == 0:
        print(f"Hakkınız bitmiştir. Tutulan sayı: {sayı}")