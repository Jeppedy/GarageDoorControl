#include <DS18B20.h>

const int ONE_WIRE_BUS= D4 ;
const int garageControlPin = D6 ;
const int garageIndicatorPin = D9 ;
const int boardLed = D7; // This is the LED that is already on your device.
const boolean debug = false ;

const int MAXRETRY = 3;

const char appVer[] = "v2.1-GarageDoorControl" ;

int iterCount;
int crcErrCount = 0;
int failedTempRead = 0;
char devShortID[4+1];
unsigned long old_time = 0;

DS18B20 tempSensor(ONE_WIRE_BUS, true);

void publishTemp( void ) ;

//---------------------------------------------------------

void setup() {
  Serial.begin(14400);

  pinMode(garageControlPin, OUTPUT);
  digitalWrite(garageControlPin, HIGH);

  pinMode(garageIndicatorPin, OUTPUT);
  digitalWrite(garageIndicatorPin, LOW);

  pinMode(boardLed, OUTPUT);
  digitalWrite(boardLed, LOW);
  
  pinMode(ONE_WIRE_BUS, INPUT);

  Particle.function("trigGarDoor", triggerGarageDoor) ;
  Particle.function("trigGarLite", triggerGarageLight) ;
  Particle.variable("iterCount", iterCount );
  Particle.variable("crcErrCount", crcErrCount );
  Particle.variable("badTempRead", failedTempRead );

  Particle.publish("AppVer", appVer, 60, PRIVATE);

  // Device Name Identification
  String devFullID = System.deviceID() ;
  Serial.printf("Full DevID: %s\n", devFullID.c_str() );
  devFullID.substring(strlen(devFullID)-4, strlen(devFullID)).toUpperCase().toCharArray(devShortID,4+1) ;
  //Serial.println( devShortID );

  tempSensor.setResolution(TEMP_11_BIT);   // max = 12
}

void loop() {
  if(millis() - old_time >= 5000){
    digitalWrite(boardLed, HIGH);
      publishTemp() ;
    digitalWrite(boardLed, LOW);
    old_time = millis();
  }
}

//---------------------------------------------------------

void publishTemp( void ) {
  float temp1=0;
  float temp2=0;
  float temp3=0;
  char outBuffer[32+1] ; 

  temp1 = (float)getTemp();
  temp2 = 0;
  temp3 = 0; //Voltage
  
  if( temp1 == 999 || temp2 == 999 || temp3 == 999) {
    char errMsg[128+1] ;
    sprintf(errMsg, "Invalid temp found! [%f] [%f] [%f]", temp1, temp2, temp3 ) ;
    Serial.println( errMsg );
    if( debug ) Particle.publish("Error", errMsg, 60, PRIVATE );
    failedTempRead++ ;
  }
  Particle.process() ;

  if ( ++iterCount > 999 ) iterCount = 0;

  sprintf(outBuffer, "%c%c,%03d,%04u,%04u,%04u",
    devShortID[2], devShortID[3],
    iterCount,
    (int16_t)(temp1*10),
    (int16_t)(temp2*10),
    (int16_t)(temp3*10)
    );

  Mesh.publish("temps", outBuffer);
  Serial.printf("outBuffer: %s len: %d \n",outBuffer, strlen(outBuffer));
  if( debug ) Particle.publish("tempDBG", outBuffer, 60, PRIVATE);
}

int triggerGarageDoor( String inString ) {
  flashRelay( garageControlPin, 150 ) ;
  Particle.publish("DOOR", "Triggered", 60, PRIVATE);
  return 0;
}

int triggerGarageLight( String inString ) {
  flashRelay( garageControlPin, 50 ) ;
  Particle.publish("LIGHT", "Triggered", 60, PRIVATE);
  return 0;
}

void flashRelay( int pin, int duration ) {
  digitalWrite(garageIndicatorPin, HIGH);

  digitalWrite(pin, LOW);
  delay(duration);
  digitalWrite(pin, HIGH);

  digitalWrite(garageIndicatorPin, LOW);
}

void blinkLED( int LEDPin, int times ) {
  for( int i=0; i < times; i++) {
    if( i > 0 )
      delay( 200 );
    digitalWrite(LEDPin, HIGH);
    delay( 150 );
    digitalWrite(LEDPin, LOW);
  }
}

double getTemp(){
  double tempOut = 999;
  float _temp;
  int   i = 0;

  do {
//    Serial.println("Obtaining reading");
    _temp = tempSensor.getTemperature();
    if(!tempSensor.crcCheck()) {
      crcErrCount++ ;
    } else {
      break ;
    }
  } while ( MAXRETRY > i++);

  if (i < MAXRETRY) {
    tempOut = tempSensor.convertToFahrenheit(_temp);
  }
  else {
    Serial.println("Invalid reading");
  }
  return tempOut ;
}
