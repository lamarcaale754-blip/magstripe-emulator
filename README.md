# Magstripe Emulator

Arduino firmware for a bench magnetic-stripe waveform emulator. It encodes Track 2 data using F2F (Aiken biphase), odd parity, and an LRC character, then drives a coil through an L293D H-bridge.

This was an early bench step in the DynamiStripe project. The published sketch is the **owner-confirmed working edition**, imported from `WORKINGCODEMAGEMULATOR.ino` on October 21, 2026. It replaces the earlier Timer1/direct-port speed-sweep edition.

## Hardware and pin mapping

- Arduino Uno/Nano-class board used for the bench prototype.
- L293D H-bridge: IN1 on **D9**, IN2 on **D10**.
- Status LED on **D11** (an external LED with an appropriate resistor; not the Uno's D13 onboard LED).
- Coil connected through the H-bridge, with a shared ground and a supply/coil combination within the driver's electrical ratings. The Arduino pins drive logic inputs, not the coil directly.

## Working edition behavior

- 22 raw zero preamble bits, followed by Track 2 characters sent least-significant bit first with odd parity.
- Four-column XOR LRC plus odd parity, followed by 30 zero postamble bits.
- `digitalWrite` switches coil polarity with interrupts disabled during transmission.
- Nominal bit-cell delay is **2666 µs**, corresponding approximately to 5 in/s at 75 bits/in. Function-call overhead affects actual timing; this is not a precision waveform measurement.
- Both drive pins go low after transmission; the loop pauses for four seconds before repeating.

## Build and upload

Open `magstripe_emulator/magstripe_emulator.ino` in Arduino IDE, select the board and port that match your hardware, and build/upload. No external Arduino libraries are required.

```sh
arduino-cli compile --fqbn arduino:avr:uno magstripe_emulator
```

## Provenance and testing

The owner identified `WORKINGCODEMAGEMULATOR.ino` as the edition last confirmed working with the reader. This update preserves that sketch's firmware logic, pin assignments, and timing. The **only source substitution** is the hardcoded track payload, replaced with a fictional test payload using the standard test number `4111111111111111`.

The published edition compiles for Arduino Uno with Arduino AVR Boards 1.8.7: 1,524 bytes of program storage and 651 bytes of global-variable memory. No new physical reader test was performed as part of this publication update. The sanitized payload should be validated on the same authorized bench setup. The fixed-size encoding buffer is intended for the included fixed payload; this is not a general untrusted-input parser.

Use only with test data and equipment you own or are authorized to test. No private source-history archive or original track payload is included in this update.
