// Pin your LED is connected to
const int ledPin = 2; // change this to the pin your LED is on

// Timing for Morse code
const int dotTime = 250;   // duration of a dot
const int dashTime = dotTime * 3; // duration of a dash
const int symbolSpace = dotTime;  // space between symbols
const int letterSpace = dotTime * 3; // space between letters
const int wordSpace = dotTime * 7;   // space between words

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("ESP32 SOS LED Program Started");
}

void loop() {
  // SOS: ... --- ...
  blinkSOS();
  delay(wordSpace); // pause before repeating
}

void blinkDot() {
  digitalWrite(ledPin, HIGH);
  delay(dotTime);
  digitalWrite(ledPin, LOW);
  delay(symbolSpace);
}

void blinkDash() {
  digitalWrite(ledPin, HIGH);
  delay(dashTime);
  digitalWrite(ledPin, LOW);
  delay(symbolSpace);
}

void blinkSOS() {
  Serial.println("Incoming transmission from the RMS Titanic...");
  // S: ...
  blinkDot();
  blinkDot();
  blinkDot();
  Serial.println("...");
  
  delay(letterSpace);
  
  // O: ---
  blinkDash();
  blinkDash();
  blinkDash();
  Serial.println("---");
  
  delay(letterSpace);
  
  // S: ...
  blinkDot();
  blinkDot();
  blinkDot();
  Serial.println("...");
  Serial.println("We've dressed in our best, and we shall go down like gentlemen.");
}
