#define IN1_BIT  1
#define IN2_BIT  2
#define LED_BIT  5

#define COIL_FWD()  { PORTB |=  (1 << IN1_BIT); PORTB &= ~(1 << IN2_BIT); }
#define COIL_REV()  { PORTB &= ~(1 << IN1_BIT); PORTB |=  (1 << IN2_BIT); }
#define COIL_OFF()  { PORTB &= ~(1 << IN1_BIT); PORTB &= ~(1 << IN2_BIT); }
#define LED_HIGH()  (PORTB |=  (1 << LED_BIT))
#define LED_LOW()   (PORTB &= ~(1 << LED_BIT))

char trackData[] = ";4111111111111111=29012010000000000000?";

bool bitStream[500];
int bitStreamLen = 0;

bool oddParity(int val) {
  int count = 0;
  for (int i = 0; i < 4; i++) if (val & (1 << i)) count++;
  return (count % 2 == 0) ? 1 : 0;
}

void encodeTrack2(const char* data, bool* stream, int& len) {
  len = 0;
  int lrc[5] = {0,0,0,0,0};

  auto appendChar = [&](int val, bool doLRC) {
    bool bits[5];
    bits[0] = (val >> 0) & 1;
    bits[1] = (val >> 1) & 1;
    bits[2] = (val >> 2) & 1;
    bits[3] = (val >> 3) & 1;
    bits[4] = oddParity(val);
    for (int j = 0; j < 5; j++) {
      stream[len++] = bits[j];
      if (doLRC) lrc[j] ^= bits[j];
    }
  };

  for (int i = 0; i < 20; i++) {
    stream[len++] = 0;
    stream[len++] = 0;
    stream[len++] = 0;
    stream[len++] = 0;
    stream[len++] = 1;
  }

  for (int i = 0; data[i] != '\0'; i++) {
    char c = data[i];
    int val;
    if      (c >= '0' && c <= '9') val = c - '0';
    else if (c == '=')             val = 13;
    else if (c == ';')             val = 11;
    else if (c == '?')             val = 15;
    else                           continue;
    appendChar(val, true);
  }

  int lrcVal = lrc[0]|(lrc[1]<<1)|(lrc[2]<<2)|(lrc[3]<<3);
  stream[len++] = lrc[0];
  stream[len++] = lrc[1];
  stream[len++] = lrc[2];
  stream[len++] = lrc[3];
  stream[len++] = oddParity(lrcVal);
}

void sendNoIntContinuous(bool* bits, int numBits, uint16_t halfCycles, int repeats) {
  TCCR1A = 0;
  TCCR1B = (1 << CS10);
  TCNT1 = 0;

  noInterrupts();

  bool state = false;
  COIL_FWD();

  for (int r = 0; r < repeats; r++) {
    for (int i = 0; i < numBits; i++) {
      state = !state;
      if (state) { COIL_FWD(); } else { COIL_REV(); }

      if (bits[i]) {
        TCNT1 = 0; while (TCNT1 < halfCycles);
        state = !state;
        if (state) { COIL_FWD(); } else { COIL_REV(); }
        TCNT1 = 0; while (TCNT1 < halfCycles);
      } else {
        TCNT1 = 0; while (TCNT1 < halfCycles);
        TCNT1 = 0; while (TCNT1 < halfCycles);
      }
    }
  }

  interrupts();
  COIL_OFF();
}

void setup() {
  DDRB |= (1 << IN1_BIT) | (1 << IN2_BIT) | (1 << LED_BIT);
  COIL_OFF();
  LED_LOW();

  for (int i = 0; i < 3; i++) {
    LED_HIGH(); delay(100);
    LED_LOW();  delay(100);
  }
  delay(500);

  encodeTrack2(trackData, bitStream, bitStreamLen);
}

uint16_t halfCyclesTable[] = { 2672, 2128, 1776, 1328, 1072, 896 };
int numSpeeds = 6;

void loop() {
  for (int s = 0; s < numSpeeds; s++) {
    for (int b = 0; b <= s; b++) {
      LED_HIGH(); delay(120);
      LED_LOW();  delay(120);
    }
    delay(300);

    LED_HIGH();
    sendNoIntContinuous(bitStream, bitStreamLen, halfCyclesTable[s], 5);
    LED_LOW();

    delay(3000);
  }
}
