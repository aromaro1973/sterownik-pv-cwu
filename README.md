# Sterownik PV-CWU / EMS Off-Grid

Documentation of the current off-grid energy management firmware for a resistive hot-water heater controller.

## 1. Project goal

The system receives telemetry from an off-grid inverter and safely directs surplus photovoltaic energy to the heater load while maintaining battery safety and inverter stability.

The current logic is intentionally off-grid oriented:
- heating is driven by inverter and battery telemetry,
- PV power is treated as diagnostic input rather than the main control signal,
- the system keeps a smooth ramp-up and only allows full power when the safety conditions are satisfied.

## 2. Current operating logic

### 2.1. Power rule

The present control law is:
- 0-47%: smooth power regulation,
- threshold at 47%: the system stops the smooth ramp and waits for surplus conditions,
- 100%: full power is only enabled when the controller confirms a safe surplus and the protection constraints allow it.

This preserves a soft start and prevents an abrupt jump from low power to full power.

### 2.2. Automatic mode

In AUTO mode the controller uses:
- inverter power,
- battery power balance,
- the configurable battery discharge limit stored by Guardian,
- the safety block state managed by Guardian.

In MANUAL mode the heater power is controlled directly by the user.

## 3. Module architecture

### 3.1. main.cpp

Coordinates the complete firmware:
- initializes all modules,
- maintains the system state machine,
- handles service-menu activity,
- drives the user interface loop.

### 3.2. ZeroCross

Detects zero crossing of the AC mains waveform.

It operates as the fast hardware path:
- uses attachInterrupt(...),
- filters false zero-cross pulses,
- provides timing synchronization to the triac driver.

### 3.3. PhaseController

Controls phase-angle firing of the triac.

It converts the requested heater power percentage into a microsecond delay for the triac gate and protects the system from unsafe intermediate-power regions through the current safety logic.

### 3.4. AutoController

Computes the target heater power based on:
- inverter power,
- battery power balance,
- the configured battery draw limit,
- the current Guardian block state.

### 3.5. Guardian

Acts as the safety supervisor.

It stores and enforces the main limits:
- maxPower,
- powerStep,
- maxBatteryDraw.

These values are persisted in NVS and reloaded on startup.

### 3.6. DisplayManager

Manages the LCD display screens and service-menu rendering.

### 3.7. ControlPanel

Handles user input and mode switching.

## 4. Service menu order

The service menu is organized in a practical order from input diagnostics to safety logic:

1. ZeroCross diagnostics,
2. PhaseController diagnostics,
3. Guardian max power,
4. Battery draw limit,
5. Guardian delta power,
6. ESP-NOW radio status,
7. AutoController status.

This keeps the flow diagnostic-first and safety-oriented.

## 5. Verified build status

The firmware was verified by running:

platformio run

Result: successful completion with exit code 0.

## 6. Summary

The current firmware is a local off-grid EMS controller for a resistive water-heater load. It uses inverter and battery telemetry, applies a soft-start ramp, and only allows full power when the available surplus and safety constraints are satisfied.