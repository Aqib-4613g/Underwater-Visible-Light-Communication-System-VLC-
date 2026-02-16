// === VLC Receiver – 5ms Bit Delay, Matched ACK Timing ===
const int photodiodePin = 34;
const int blueLedPin = 4;
const int gatePin = 18;
const int bitDelay = 5;
const String preamble = "10101010";

void setup() {
  Serial.begin(115200);
  pinMode(photodiodePin, INPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(gatePin, OUTPUT);
  digitalWrite(gatePin, HIGH);
  Serial.println("=== VLC Receiver Ready (5ms mode) ===");
}

char decodeByte(const String &bits) {
  return (char)strtol(bits.c_str(), nullptr, 2);
}

void sendBit(bool bit) {
  digitalWrite(blueLedPin, bit ? HIGH : LOW);
  delay(bitDelay);
}

void sendFrame(char c) {
  String frame = "1";
  sendBit(1);
  for (int b = 7; b >= 0; b--) {
    bool bit = (c >> b) & 1;
    frame += bit ? '1' : '0';
    sendBit(bit);
  }
  frame += '0';
  sendBit(0);
  Serial.print("ACK Frame: ");
  Serial.print(frame);
  Serial.print(" → '");
  Serial.print(c);
  Serial.println("'");
}

String waitForFrame() {
  while (digitalRead(photodiodePin) == 0) delay(1);
  delay(bitDelay / 2);
  String frame = "1";
  for (int b = 0; b < 8; b++) {
    delay(bitDelay);
    frame += (digitalRead(photodiodePin) ? '1' : '0');
  }
  delay(bitDelay);
  frame += '0';
  return frame;
}

bool waitForPreamble() {
  String preambleBits = "";
  unsigned long start = millis();
  while (millis() - start < bitDelay * 1000) {
    while (digitalRead(photodiodePin) == 0) delay(1);
    delay(bitDelay / 2);
    preambleBits = "1";
    for (int i = 1; i < preamble.length(); i++) {
      delay(bitDelay);
      preambleBits += (digitalRead(photodiodePin) ? '1' : '0');
    }
    if (preambleBits == preamble) return true;
  }
  return false;
}

void loop() {
  while (digitalRead(photodiodePin) == 1) delay(1);

  if (!waitForPreamble()) return;

  Serial.println("--- Reception Started ---");
  Serial.print("Preamble: ");
  Serial.println(preamble);

  String decodedMessage = "";
  while (true) {
    String frame = waitForFrame();
    if (frame == "1000000000") break;
    if (frame.charAt(0) == '1' && frame.charAt(9) == '0') {
      char c = decodeByte(frame.substring(1, 9));
      Serial.print("Frame: ");
      Serial.print(frame);
      Serial.print(" → '");
      Serial.print(c);
      Serial.println("'");
      decodedMessage += c;
    }
  }

  Serial.println("--- Message Received ---");
  Serial.print("Decoded Message: ");
  Serial.println(decodedMessage);

  delay(bitDelay * 150);  // match transmitter before ACK start

  Serial.println("--- Sending Acknowledgment ---");
  for (char b : preamble) sendBit(b == '1');
  delay(bitDelay * 20);  // reduced guard delay (~100 ms)
  String ack = "ACK";
  for (char c : ack) sendFrame(c);
  sendBit(0);
  sendBit(0);
  Serial.println("--- Acknowledgment Sent ---");

  while (digitalRead(photodiodePin) == 1) delay(1);
}