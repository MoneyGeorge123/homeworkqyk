const int ledPin =2;
unsigned long previous =0;
void setup() {
  pinMode(ledPin,OUTPUT);// put your setup code here, to run once:
  Serial.begin(115200);
  previous=millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long current=millis();
  unsigned long elapsed=current-previous;
  if(elapsed<500){digitalWrite(ledPin,HIGH);}
  else if(elapsed<1000){digitalWrite(ledPin,LOW);}
  else {previous=current;}
}
