# Architektura Sterownika Nadwyżki PV

## Główne założenie projektu

Sterownik Nadwyżki PV jest urządzeniem autonomicznym.

Oznacza to, że wszystkie podstawowe funkcje sterownika muszą działać poprawnie bez:

- Home Assistanta,
- WiFi,
- MQTT,
- Raspberry Pi,
- Internetu.

Home Assistant jest wyłącznie dodatkiem rozszerzającym możliwości sterownika.

Sterownik musi być możliwy do uruchomienia, skonfigurowania i obsługi wyłącznie za pomocą:

- wyświetlacza LCD,
- modułu ControlPanel.

---

# Priorytet rozwoju projektu

Projekt rozwijany jest etapowo.

Kolejność realizacji modułów:

1. Elektronika wykonawcza.
2. ZeroCross.
3. HeaterOutput.
4. BurstFire.
5. DisplayManager.
6. ControlPanel.
7. ESPNowManager.
8. Guardian.
9. AutoController.
10. WiFiManager.
11. Home Assistant.
12. MQTT.
13. Funkcje analityczne.

Home Assistant nie jest wymagany do poprawnej pracy sterownika.

---

# Filozofia modułów

Każdy moduł projektu jest całkowicie niezależny.

Każdy moduł odpowiada wyłącznie za swoje zadanie.

Przykład:

WiFiManager odpowiada wyłącznie za połączenie WiFi.

Guardian odpowiada wyłącznie za bezpieczeństwo pracy.

BurstFire odpowiada wyłącznie za sterowanie mocą.

ControlPanel odpowiada wyłącznie za obsługę użytkownika.

Żaden moduł nie realizuje funkcji należących do innego modułu.

---

# Warstwa sterowania

Jedynym urządzeniem sterującym jest ESP32.

ESP32 podejmuje wszystkie decyzje dotyczące:

- mocy grzałki,
- bezpieczeństwa,
- regulacji,
- algorytmu pracy.

Home Assistant nie steruje bezpośrednio grzałką.

Home Assistant może jedynie zmieniać parametry pracy sterownika.

---

# Rola Home Assistant

Home Assistant pełni funkcję:

- panelu operatorskiego,
- panelu informacyjnego,
- panelu analitycznego,
- archiwizacji danych,
- zdalnej konfiguracji.

Home Assistant nie jest elementem odpowiedzialnym za bezpieczeństwo pracy sterownika.

---

# ControlPanel

ControlPanel jest lokalnym panelem sterowania sterownika.

Sterownik musi umożliwiać pełną konfigurację bez użycia Home Assistanta.

Za pomocą ControlPanel użytkownik może:

- zmieniać tryb pracy,
- ustawiać parametry modułów,
- odczytywać informacje diagnostyczne,
- uruchamiać oraz konfigurować sterownik.

---

# Moduły konfiguracyjne

Każdy moduł może posiadać własne parametry konfiguracyjne.

Przykład:

Guardian

- maksymalna moc falownika,
- maksymalny dopuszczalny skok mocy.

AutoController

- minimalny SOC,
- maksymalna moc grzałki,
- minimalna moc PV.

WiFiManager

- konfiguracja sieci,
- nazwa urządzenia.

DisplayManager

- kontrast,
- podświetlenie.

Każdy moduł udostępnia jedynie własne parametry.

---

# Diagnostyka

Po uruchomieniu sterownik wykonuje test wszystkich modułów.

Przykład:

ZeroCross ............ OK

BurstFire ............ OK

HeaterOutput ......... OK

DisplayManager ....... OK

ControlPanel ......... OK

ESPNowManager ........ OK

Guardian ............. OK

AutoController ....... OK

Po zakończeniu testów sterownik przechodzi do normalnej pracy.

---

# ESP-NOW

ESP-NOW jest podstawowym kanałem komunikacji z falownikiem.

Za jego pomocą przekazywane są dane niezbędne do pracy:

- moc falownika,
- moc odbiorników,
- moc PV,
- SOC,
- pozostałe dane pomiarowe.

Bez poprawnych danych z ESP-NOW tryb AUTO nie powinien rozpocząć regulacji.

---

# Guardian

Guardian posiada najwyższy priorytet.

Może natychmiast wyłączyć grzałkę niezależnie od pozostałych modułów.

Parametry Guardian ustawiane są lokalnie z poziomu ControlPanel.

Przykładowe parametry:

- maksymalna moc falownika,
- maksymalny dopuszczalny skok mocy.

---

# AutoController

AutoController wylicza moc grzałki na podstawie danych otrzymanych z ESP-NOW oraz parametrów zapisanych w sterowniku.

Nie steruje bezpośrednio triakiem.

Wynik działania przekazuje do modułu BurstFire.

---

# Home Assistant

Home Assistant może:

- zmieniać ustawienia,
- odczytywać dane,
- prezentować wykresy,
- prowadzić analizy.

Home Assistant nie może sterować bezpośrednio:

- triakiem,
- BurstFire,
- HeaterOutput,
- Guardian.

Cała logika bezpieczeństwa pozostaje w ESP32.

---

# Główna filozofia projektu

Sterownik Nadwyżki PV jest samodzielnym sterownikiem przemysłowym.

Home Assistant stanowi jedynie opcjonalne rozszerzenie jego możliwości.

Brak sieci WiFi, Home Assistanta lub Internetu nie może uniemożliwić poprawnej pracy sterownika.
