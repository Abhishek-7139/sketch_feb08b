int RELAY_PIN=13;
int SIGNAL_PIN=12;

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SIGNAL_PIN, INPUT);
}

void loop() {
  int sig = digitalRead(SIGNAL_PIN);
  if(sig){
    digitalWrite(RELAY_PIN, LOW);
  }else{
    digitalWrite(RELAY_PIN, HIGH);
  }
  delay(2000);
}
