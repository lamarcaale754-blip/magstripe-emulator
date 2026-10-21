#define IN1_PIN 9
#define IN2_PIN 10
#define LED_PIN 11

char trackData[] = ";4111111111111111=29012010000000000000?"; 

bool bitStream[600]; 
int bitStreamLen = 0;

void encodeTrack2Data(const char* data, bool* bits, int& num) {
  num = 0;

  // 1. RAW PREAMBLE: 22 raw zero bits (essential for reader clock lock)
  for (int i = 0; i < 22; i++) {
    bits[num++] = 0;
  }

  int lrcAccumulator[4] = {0, 0, 0, 0};

  // 2. DATA: 5-bit characters (4 data + 1 odd parity)
  for (int i = 0; data[i] != '\0'; i++) {
    int val;
    char c = data[i];
    
    if (c >= '0' && c <= '9') val = c - '0';
    else if (c == ';') val = 11; // Start Sentinel
    else if (c == '=') val = 13; // Field Separator
    else if (c == '?') val = 15; // End Sentinel
    else continue;

    int onesCount = 0;
    for (int j = 0; j < 4; j++) {
      bool b = (val >> j) & 1; // LSB first
      bits[num++] = b;
      lrcAccumulator[j] ^= b;
      if (b) onesCount++;
    }
    // Odd Parity Bit
    bits[num++] = (onesCount % 2 == 0) ? 1 : 0;
  }

  // 3. LRC: Error checking block
  int lrcOnes = 0;
  for (int j = 0; j < 4; j++) {
    bits[num++] = lrcAccumulator[j];
    if (lrcAccumulator[j]) lrcOnes++;
  }
  bits[num++] = (lrcOnes % 2 == 0) ? 1 : 0;

  // 4. POSTAMBLE: Buffers the end of the swipe
  for (int i = 0; i < 30; i++) {
    bits[num++] = 0;
  }
}

// ============================================
// SEND: Optimized for 9V Battery & L293D
// ============================================
void sendStable(bool* bits, int num, unsigned int bit_us) {
  unsigned int half_us = bit_us / 2;
  bool state = false;

  // Disable interrupts to prevent timing jitter from background tasks
  noInterrupts(); 

  for (int i = 0; i < num; i++) {
    // F2F ALWAYS transitions at the start of a bit cell
    state = !state;
    digitalWrite(IN1_PIN, state);
    digitalWrite(IN2_PIN, !state);

    if (bits[i]) {
      // "1" bit: transition again at the halfway point
      delayMicroseconds(half_us);
      state = !state;
      digitalWrite(IN1_PIN, state);
      digitalWrite(IN2_PIN, !state);
      delayMicroseconds(half_us);
    } else {
      // "0" bit: hold the state for the full bit duration
      delayMicroseconds(bit_us);
    }
  }

  // Set pins to LOW to stop battery drain after swipe
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);

  interrupts(); 
}

void setup() {
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Flash LED to signal ready
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);

  encodeTrack2Data(trackData, bitStream, bitStreamLen);

  // --- 9V SURVIVAL SETTINGS ---
  // Density: 75 BPI
  // Speed: 5.0 in/s (Slower speed = cleaner transitions for the L293D)
  // bit_us = 1,000,000 / (75 * 5) = 2666
  sendStable(bitStream, bitStreamLen, 2666);

  digitalWrite(LED_PIN, LOW);
  
  // CRITICAL: 4 second pause to let the 9V battery voltage recover
  delay(4000); 
}
