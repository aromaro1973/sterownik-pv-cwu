# Logger

Prosty logger UART dla firmware.

## Funkcje

- poziomy logowania: DEBUG/INFO/WARNING/ERROR,
- obsluga `String` i `F(...)` (Flash strings),
- globalne wlaczenie/wylaczenie logowania,
- prefiks czasu oparty na `millis()`.

## Cel

Ujednolicona diagnostyka bez rozrzucania `Serial.print` po module.
