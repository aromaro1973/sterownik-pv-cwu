# ControlPanel

Modul obsluguje przyciski PLUS, MODE i MINUS.

## Funkcje

- debounce 50 ms,
- rozroznienie klikniecia MODE i przytrzymania MODE (2 s),
- zdarzenia impulsowe (`was...Pressed`) z auto-resetem,
- przechowywanie aktualnego trybu (`WorkMode`) i nastawy mocy manualnej.

## Integracja

`main.cpp` odpowiada za mapowanie klikniec na akcje UI i logike trybow.
