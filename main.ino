int ledPin = 9;
byte count = 0;
boolean flag = true;
void setup() {  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  if (flag) {
    count++;
  } else {
    count--;
  }
  analogWrite(ledPin, count);
  if (count == 254) {
    flag = false;
    Serial.println(flag);
  }
  if (count == 1) {
    flag = true;
    Serial.println(flag);
  }
  delay(5);
  Serial.println(count);
}