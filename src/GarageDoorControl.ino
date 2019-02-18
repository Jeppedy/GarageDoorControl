int controlPin = D6 ;

void setup() {
  pinMode(controlPin, OUTPUT);
  digitalWrite(controlPin, HIGH);

  Particle.function("flashRelay", flashRelayCloud) ;
}

int flashRelayCloud( String inString ) {
  flashRelay( inString.toInt() ) ;
  return 0 ;
}

void flashRelay( int duration ) {
  digitalWrite(controlPin, LOW);
  delay(duration);
  digitalWrite(controlPin, HIGH);
}

void loop() {
}