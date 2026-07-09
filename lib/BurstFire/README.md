# BurstFire

## Zadanie modułu

Moduł `BurstFire` odpowiada za płynną regulację mocy grzałki metodą **Burst Fire**.

Algorytm rozdziela załączone półokresy równomiernie w oknie 100 półokresów, dzięki czemu uzyskiwana jest płynna regulacja mocy bez generowania dużych impulsów obciążenia.

---

## Funkcje

- regulacja mocy od 0 do 100%
- okno regulacji 100 półokresów
- równomierne rozmieszczenie załączonych półokresów
- wywołanie przy każdym przejściu sieci przez zero
- zwracanie decyzji:
  - `true` – załącz grzałkę
  - `false` – wyłącz grzałkę

---

## Wejścia

- zadana moc w procentach (`0...100`)

---

## Wyjście

Metoda:

```cpp
bool next();
```

zwraca informację czy w aktualnym półokresie triak powinien zostać załączony.

---

## Współpraca z modułami

```
AutoController
        │
        ▼
BurstFire
        │
        ▼
HeaterOutput
        │
        ▼
Triak
```

---

## Moduł nie odpowiada za

- pomiar częstotliwości sieci,
- detekcję przejścia przez zero,
- sterowanie użytkownika,
- zabezpieczenia Guardian,
- sterowanie triakiem.

---

## Testy

Moduł został sprawdzony podczas testów:

- regulacja 0–100%
- test z żarówką 100 W
- test z grzałką 1800 W
- poprawna współpraca z modułem `ZeroCross`
- poprawna współpraca z modułem `HeaterOutput`

---

## Status

✅ Ukończony

---

## Autor

Arkadiusz Marek

Projekt rozwijany przy współpracy z ChatGPT.

---

## Wersja modułu

**1.0**