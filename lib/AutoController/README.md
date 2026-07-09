# Moduł AutoController (Off-Grid EMS)

Moduł odpowiada za automatyczną, płynną regulację mocy grzałki CWU w systemie Off-Grid z magazynem energii, współpracującym z falownikiem Anenji.

---

## 💡 Główne Założenia Algorytmu

1. **Pre-positioning (Inteligentny Start):**
   Gdy automatyka startuje, nie zaczyna od 0%. Jeśli wykryje, że bateria jest aktualnie ładowana (nadwyżka PV), natychmiast ustawia moc początkową grzałki równą mocy ładowania baterii, optymalizując czas reakcji.

2. **Anty-Czajnik (Szybki Zrzut):**
   Jeśli w domu zostanie włączone duże urządzenie (np. czajnik), moc pobierana z baterii gwałtownie skoczy. Jeśli skok ten przekroczy próg bezpiecznego rozładowania o więcej niż 500W, sterownik natychmiast zrzuca moc grzałki o 30% lub do zera w ułamku sekundy.

3. **Regulacja Krokowa (500ms):**
   Co 500 ms algorytm analizuje bilans baterii:
   * **Rozładowanie > próg (np. 400W):** Zmniejsza moc grzałki o około 200W, by odciążyć akumulator.
   * **Rozładowanie < próg / Ładowanie:** Zwiększa moc grzałki o około 100W, by "wyciągnąć" maksymalną dostępną moc z paneli PV.

---

## 🛠 Struktura Modułu

* `AutoController.h` - Definicja klasy, zmiennych wewnętrznych oraz metod sterujących.
* `AutoController.cpp` - Logika wykonawcza algorytmu dopasowana do sensorów falownika Anenji.

---

## 🧭 Metody Publiczne

```cpp
void begin(uint16_t nominalHeaterPower);
void reset();
uint8_t calculateOffGridPower(int32_t powerPV, int32_t powerInv, int32_t powerBat, int32_t maxBatDischargeW, bool guardianBlocked);