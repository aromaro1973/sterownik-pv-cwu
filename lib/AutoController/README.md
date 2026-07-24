# AutoController

Modul odpowiada za automatyczne wyliczanie zadanej mocy grzalki w trybie AUTO.

## Co robi

- pracuje na wartosciach w watach,
- ogranicza wzrost mocy krokowo (`stepPowerW`),
- zwieksza moc dopiero po czasie zwloki (`pvHoldDelayMs`),
- redukuje moc natychmiast przy nadmiernym obciazeniu baterii lub falownika,
- zwraca wynik jako procent `0..100`.

## Wejscia algorytmu

- moc falownika,
- bilans baterii,
- limit rozladowania baterii,
- limit mocy falownika,
- krok mocy,
- zwloka PV.

## Uwagi eksploatacyjne

- Przy braku telemetrii logika AUTO jest podtrzymywana przez `main.cpp`, ale moc zostaje zredukowana do `0%`.
- Po powrocie telemetrii algorytm jest resetowany i kontynuuje regulacje.
