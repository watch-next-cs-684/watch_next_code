
int pulseval,oxyval;
long counter = 0;
int led = 7;
int ir = 8;

// the setup routine runs once when you press reset:
void setup() {
  
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  pinMode(ir, OUTPUT);
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

}

// the loop routine runs over and over again forever:
void loop() {
  Serial.write(254);
  counter=0;
  //Serial.println(15);
  digitalWrite(ir, HIGH);
  while(counter < 18000) {
    pulseval = analogRead(A0);
    if(pulseval > 1015)
      pulseval = 1015;
    oxyval = analogRead(A2);
    if(oxyval > 1015)
      oxyval = 1015;
    Serial.write(pulseval/4);    
    Serial.write(oxyval/4);
    counter++;
  }
  digitalWrite(ir, LOW);
  Serial.write(255);
  digitalWrite(led, HIGH);
  counter=0;
  while(counter < 18000) {
    pulseval = analogRead(A0);
    if(pulseval > 1015)
      pulseval = 1015;
    oxyval = analogRead(A2);
    if(oxyval > 1015)
      oxyval = 1015;
    Serial.write(pulseval/4);
    Serial.write(oxyval/4);
    counter++;
  }
  //Serial.println(60);
  digitalWrite(led, LOW);              
}
