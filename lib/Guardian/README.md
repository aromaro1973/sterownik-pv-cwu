# Guardian

## Zadanie modułu

Moduł `Guardian` odpowiada za bezpieczeństwo pracy sterownika oraz ochronę falownika przed przeciążeniem.

Guardian nadzoruje parametry pracy i w razie przekroczenia dopuszczalnych wartości może natychmiast zablokować pracę grzałki.

---

## Funkcje

- kontrola maksymalnej mocy falownika,
- kontrola nagłego wzrostu mocy,
- blokada pracy grzałki,
- udostępnianie informacji o stanie zabezpieczenia,
- współpraca z ControlPanel.

---

## Wejścia

Guardian otrzymuje:

- aktualną moc falownika,
- aktualną moc grzałki,
- dane z ESPNowManager,
- parametry zapisane przez użytkownika.

---

## Wyjścia

Moduł udostępnia:

```cpp
isBlocked();
getBlockReason();
getMaxPower();
getPowerStep();
```

---

## Parametry użytkownika

Parametry konfigurowane z poziomu ControlPanel:

- maksymalna moc falownika [W],
- maksymalny dopuszczalny skok mocy [W].

Parametry zapisywane są w pamięci nieulotnej ESP32.

---

## Współpraca z modułami

```
ESPNowManager
      │
      ▼
   Guardian
      │
      ├────► BurstFire
      ├────► HeaterOutput
      ├────► DisplayManager
      ├────► ControlPanel
      └────► Logger
```

---

## Moduł nie odpowiada za

- sterowanie triakiem,
- BurstFire,
- AutoController,
- Home Assistant,
- WiFi,
- ESP-NOW.

Guardian podejmuje jedynie decyzję:

```
Grzałka może pracować

lub

Grzałka zablokowana
```

---

## Założenia projektowe

Guardian jest modułem zabezpieczającym pracę sterownika oraz falownika.

Guardian nie steruje mocą grzałki i nie uczestniczy w regulacji procesu.

Jedynym zadaniem modułu jest kontrola warunków bezpieczeństwa.

Guardian posiada najwyższy priorytet spośród wszystkich modułów sterownika.

Jeżeli wykryje przekroczenie ustawionych parametrów, może natychmiast zablokować pracę grzałki niezależnie od trybu pracy sterownika.

Blokada obowiązuje zarówno w trybie AUTO, jak i MANUAL.

Guardian pełni rolę elektronicznego bezpiecznika.

Cała regulacja mocy grzałki należy wyłącznie do modułu AutoController.

---

## Aktualna logika pracy

Guardian wykonuje kontrolę bezpieczeństwa po odebraniu każdej nowej ramki danych z modułu ESPNowManager.

Dla każdej nowej próbki wykonywane są następujące czynności:

1. Odczyt aktualnej mocy falownika.
2. Obliczenie przyrostu mocy względem poprzedniej próbki.
3. Porównanie z ustawieniami użytkownika.
4. W przypadku wykrycia przekroczenia ustawiany jest stan oczekiwania na potwierdzenie (Pending Alarm).
5. Po odebraniu następnej ramki:
   - jeżeli przekroczenie nadal występuje — następuje natychmiastowa blokada grzałki,
   - jeżeli przekroczenie zniknęło — alarm zostaje skasowany i sterownik kontynuuje normalną pracę.

Guardian nie analizuje trendów zmian mocy.

Guardian nie wykonuje filtracji ani uśredniania pomiarów.

Cała analiza zmian obciążenia oraz regulacja mocy realizowana jest przez moduł AutoController.

---

## Filozofia działania

Sterownik wykorzystuje dwa niezależne mechanizmy odpowiedzialne za bezpieczeństwo pracy.

### AutoController

AutoController odpowiada za regulację mocy grzałki.

Na podstawie danych z ESPNowManager analizuje zmiany obciążenia instalacji i odpowiednio zwiększa lub zmniejsza moc grzałki.

Jego zadaniem jest zapobieganie przeciążeniu falownika.

---

### Guardian

Guardian jest ostatnią linią zabezpieczenia.

Nie reguluje mocy grzałki.

Nie przewiduje zmian obciążenia.

Nie analizuje trendów.

Guardian reaguje wyłącznie na przekroczenie parametrów bezpieczeństwa.

Jeżeli AutoController nie zdąży zareagować lub nastąpi nagły wzrost obciążenia instalacji, Guardian natychmiast blokuje pracę grzałki.

Dzięki temu Guardian pełni funkcję elektronicznego bezpiecznika sterownika.

## Planowane rozszerzenia API

```cpp
setMaxPower();

getMaxPower();

setPowerStep();

getPowerStep();

isBlocked();

getBlockReason();
```

Planowane rozszerzenia:

- historia blokad,
- licznik zadziałań,
- czas ostatniej blokady,
- temperatura radiatora jako dodatkowe zabezpieczenie.

---
Planowane funkcje wewnętrzne:

```cpp
confirmAlarm();

clearAlarm();

checkMaxPower();

checkPowerStep();
```

Funkcje te będą wykorzystywane wyłącznie wewnątrz modułu Guardian i nie będą dostępne dla pozostałych modułów projektu.

## Testy

Planowane testy:

- przekroczenie maksymalnej mocy falownika,
- przekroczenie dopuszczalnego skoku mocy,
- potwierdzenie alarmu w kolejnej próbce danych,
- anulowanie alarmu po ustąpieniu przekroczenia,
- współpraca z ControlPanel,
- współpraca z DisplayManager,
- współpraca z Logger,
- współpraca z AutoController.
  

---
Guardian jest jedynym modułem, który może natychmiast zablokować pracę grzałki niezależnie od aktualnego trybu pracy sterownika.

## Status

🟡 Projektowanie modułu

---

## Autor

Arkadiusz Marek

Projekt rozwijany przy współpracy z ChatGPT.

---

## Wersja modułu

0.1
