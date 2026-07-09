Oto kompletne, uporządkowane podsumowanie architektoniczne Twojego projektu. Stworzyłem z tego gotową notatkę techniczną, którą możesz zapisać w swojej dokumentacji projektu.
DOKUMENTACJA PROJEKTU: STEROWNIK PV OFF-GRID (HYBRYDOWY EMS)
Główny cel: Efektywne zagospodarowanie nadwyżek energii z instalacji PV do podgrzewania CWU (zasobnik 140L, 2 osoby) przy jednoczesnej maksymalnej ochronie falownika (4.2 kW) oraz akumulatora (magazyn 24 V).
I. Główne Moduły Logiczne
1. Autocontroller (Zarządzanie Mocą)
	Zadanie: Śledzenie nadwyżek energii i dynamiczne dopasowywanie mocy grzałki (2 kW).
	Logika działania: Pobiera dane o produkcji z paneli PV i stanie naładowania baterii. Podbija moc grzałki, aby wymusić maksymalną produkcję z paneli, jednocześnie pilnując, aby pobór z akumulatora nie przekroczył 400 W (ok. 16-17 A dla instalacji 24 V), co chroni żywotność magazynu w czasie chmur.
2. Guardian (Moduł Bezpieczeństwa)
	Zadanie: Natychmiastowa ochrona falownika przed przeciążeniem i szokiem termicznym.
	Logika działania: * Limit stały: Wyłącza grzałkę, jeśli całkowite obciążenie falownika przekroczy ustawiony bezpieczny próg (np. 3500 W).
	Limit dynamiczny (ΔP): Wykrywa nagły skok mocy wywołany przez domowników (np. włączenie czajnika > 1 kW) i w czasie liczonym w milisekundach odłącza grzałkę na czas pracy drugiego urządzenia.
II. Metoda Sterowania Grzałką (Hybrydowa)
Rezygnujemy z klasycznego algorytmu Bresenhama (który szarpie falownikiem pełną mocą 2 kW na poziomie pojedynczych półokresów) na rzecz sterowania fazowego na zboczu opadającym sinusoidy połączonego z trybem pełnej mocy. Wymaga to wymiany optotriaka na model bez detekcji zera (np. MOC3021 / MOC3023).
	Zakres 0% – 40% mocy (0 – 800 W): Sterowanie czasem załączenia triaka wyłącznie na zboczu opadającym (opóźnienie od 9 ms płynnie schodzące do 6 ms od punktu przejścia przez zero). Falownik widzi płynny, stały prąd o niskiej wartości. Kondensatory DC nie są obciążone ripple current, a wentylatory chłodzące pracują stabilnie.
	Zakres 40% – 99% mocy: Zakres zablokowany programowo, aby uniknąć załączania triaka w szczycie sinusoidy (co generowałoby potężne udary prądowe i harmoniczne niszczące filtry falownika).
	Tryb 100% mocy (2000 W): Przy pełnym słońcu i braku innych obciążeń, triak zostaje załączony na stałe. Falownik pracuje w optymalnym, liniowym trybie czystego sinusa.
III. Architektura Oprogramowania ESP32 (Podział na Rdzenie)
Aby zapobiec zapychaniu się procesora i zagwarantować precyzję zegara mikrosekundowego, zadania zostają rozdzielone na dwa niezależne rdzenie systemu FreeRTOS:
                      [ H11AA1 (Zero Cross) ]
                                 |
                                 v  (Przerwanie zewnętrzne)
+-------------------------------------------------------------------+
| CORE 0: SEKCJA KRYTYCZNA CZASOWO (Hardware Timers)               |
|                                                                   |
|  1. Obsługa przerwania z detektora zera.                         |
|  2. Uruchomienie sprzętowego timera z wyliczonym opóźnieniem.     |
|  3. Wygenerowanie impulsu na bramkę triaka (MOC3021).             |
+-------------------------------------------------------------------+
                                 ^
                                 | (Bezpieczna kolejka / volatile)
+-------------------------------------------------------------------+
| CORE 1: SEKCJA BIZNESOWA & AUTOMATYKA (Pętla loop)                |
|                                                                   |
|  1. Komunikacja z falownikiem (odczyt mocy PV i baterii).         |
|  2. Algorytm AUTOCONTROLLER (wyliczanie optymalnej mocy/czasu).   |
|  3. Algorytm GUARDIAN (wykrywanie czajnika, ochrona przed przeciążeniem).|
|  4. Obsługa peryferiów (wyświetlacz, Wi-Fi, diagnostyka).        |
+-------------------------------------------------------------------+
Szczegóły implementacji programowej:
	H11AA1 podłączony do pinu ESP32 wywołuje przerwanie zewnętrzne (attachInterrupt).
	Funkcja obsługi przerwania (ISR) na CORE 0 nie zawiera żadnych blokad delay(). Jej jedynym zadaniem jest zresetowanie i wystartowanie sprzętowego timera z wartością opóźnienia przesłaną z CORE 1.
	Po odliczeniu czasu przez timer, generowane jest drugie przerwanie, które wystawia stan wysoki na MOCa na ok. 15–20 μ"s"  i natychmiast wraca do pracy.
	Wszystkie powolne procesy (np. zapytania do falownika, kalkulacje, obsługa ekranu) realizowane są w pętli loop() na CORE 1. Wyliczona wartość opóźnienia przekazywana jest do CORE 0 jako zmienna volatile uint32_t delayMicros.
	Nad całością czuwa włączony sprzętowy Watchdog Timer (WDT) – w razie jakiegokolwiek zawieszenia pętli, natychmiast resetuje ESP32 i bezpiecznie gasi triak.
IV. Korzyści z Przyjętej Architektury
	Dla falownika: Brak udarów prądowych w szczycie sinusoidy i brak szarpania paczkami energii. Wydłużenie żywotności kondensatorów DC i optymalna praca układu chłodzenia.
	Dla baterii: Eliminacja ryzyka poboru prądów rzędu 90 A przy 24 V dzięki sztywnemu programowemu ograniczeniu do 400 W w warunkach braku słońca.
	Dla Guardiana: Błyskawiczny czas reakcji na czajnik (poniżej 10 ms), mierzony na stabilnym profilu prądowym, bez fałszywych alarmów wywoływanych przez sterowanie grupowe.



