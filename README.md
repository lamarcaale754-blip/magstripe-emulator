# Magstripe Emulator

Arduino firmware that emulates a magnetic-stripe card swipe. It encodes Track 2 data using F2F (Aiken biphase) encoding with odd parity and an LRC check character, then drives a hand-wound enameled-wire coil through an H-bridge so a standard magstripe reader reads it as a real swipe.

This was the first bench step of the DynamiStripe card project: proving the read side before working on rewriting a stripe.

## Hardware

- Arduino Uno or Nano (ATmega328P, 16 MHz)
- H-bridge driver chip: IN1 on D9, IN2 on D10
- Hand-wound enameled-wire coil across the H-bridge outputs
- Onboard LED on D13 shows which swipe speed is playing

## How it works

- Track 2 characters are encoded LSB-first as 5-bit groups (4 data bits + odd parity), followed by the LRC.
- 20 leading zero characters give the reader time to lock onto the clock before the start sentinel.
- Bit timing uses Timer1 with interrupts disabled, and the coil is switched with direct port writes for consistent timing.
- The sketch cycles through six simulated swipe speeds (40 to 120 in/s). The LED blinks once for the slowest speed up to six times for the fastest.

## Testing

Bench-tested with a commercial magstripe reader using fictional test data only. The included track uses the standard Visa test number 4111 1111 1111 1111. Never used with real card data or a live payment terminal.
