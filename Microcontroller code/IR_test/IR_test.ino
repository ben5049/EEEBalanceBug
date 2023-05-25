#define IR_PIN 13
#define LED_PIN 35

void setup() {
  // put your setup code here, to run once:
  pinMode(IR_PIN, INPUT);
  // pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(IR_PIN) == LOW){
    Serial.println("1");
    // digitalWrite(LED_PIN, LOW);
  }
  else{
    Serial.println("0");
    // digitalWrite(LED_PIN, HIGH);
  
  }
  delay(100);
}
