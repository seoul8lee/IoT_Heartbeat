#include <LiquidCrystal.h>
#include <IRremote.h>

#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>



int selectMode = 0;

/* Remote Preprocess */
int RECV_PIN = A2; //define input pin on Arduino
IRrecv irrecv(RECV_PIN);
decode_results results;
/* Remote Preprocess END */


/* CPR Preprocess */
int pressurePin = A1; //pressure reading pin
int pressureRead; //for reading pressure pin
const int buttonPin = 8;  // pin for CPR count
unsigned long time; //to display time for compressions
const int pressGood = 10;
const int pressBad =  9;

int BPM_LCD; //final output calculated beats per min
float bpmCount; // to calculate BPM
float cprCount = 0;   // counter for the number of button presses, changed from int, changed from float
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int cprTotal = 0; //total of the 5 stored values
int cprCycle = 0; //keeps track of CPR cycle

int  timeSec, timeCPR, BPM_1; //calculating cpr time
float totalTime;
unsigned long timeBegin, timeEnd;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //LCD pins
/* CPR Preprocess END */



/* Pulse Preprocess */
//  Variables
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle
byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;

PulseSensorPlayground pulseSensor;

/* Pulse Preprocess END*/




 
void setup() {

  /* Remote setup */
  irrecv.enableIRIn(); // Start the receiver
  /* Remote setup END */

  /* CPR setup */
  pinMode(buttonPin, INPUT); //CPR button
  pinMode(pressGood, OUTPUT); //for good release, led on Green
  pinMode(pressBad, OUTPUT); // for bad release, led on Red
  lcd.home();
  /* CPR setup END */

  /* Pulse setup */
 // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  // Skip the first SAMPLES_PER_SERIAL_SAMPLE in the loop().
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    /*
       PulseSensor initialization failed,
       likely because our Arduino platform interrupts
       aren't supported yet.

       If your Sketch hangs here, try changing USE_PS_INTERRUPT to false.
    */
    /*
    for(;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
    */
  }
    /* Pulse setup END */


  Serial.begin(115200); // start serial communication / uncomment to debug


  lcd.setCursor(0, 0);  
  lcd.print("Select Mode");
  Serial.println("Select Mode");
  lcd.setCursor(0, 1);
  lcd.println("1. CPR 2. Pulse");
  Serial.println("1. CPR 2. Pulse");
}


void loop() {

  if(selectMode == 0){
    /* Remote */
    if (irrecv.decode(&results)) {
      Serial.println(results.value, HEX);
      switch (results.value) {
        case 0xFF30CF: selectMode = 1;  break;//1버튼이 눌리면 CPR 
        case 0xFF18E7: selectMode = 2; break;//2버튼이 눌리면 Pulse
    }
     irrecv.resume(); // Receive the next value
    }
  }
  /* CPR START */
  else if(selectMode == 1){
    if(cprCount == 0){
      lcd.setCursor(0,0);
      lcd.print("Begin CPR");
      Serial.println("Begin CPR");
     
    }
     pressureRead = analogRead(pressurePin);
      time = millis(); //start time
      buttonState = digitalRead(buttonPin);
      
      if (buttonState != lastButtonState) {
           
        if (buttonState == HIGH) { //depth of at least 2in per AHA guidelines
          lcd.clear();
          lcd.home();
          cprCount++; //add 1 to the running total
          Serial.println(cprCount);
          lcd.setCursor(0,0);
          lcd.print("Count: ");
          lcd.setCursor(7,0);
          lcd.print(cprCount);
          lcd.setCursor(10,0);
          lcd.print("R: ");     //rate BPM
          
            if(cprCount == 1){
               timeBegin = millis(); //beginning time
            }
          
          }
          
           //pressureRead = analogRead(pressurePin);
          if (pressureRead < 500){
              digitalWrite(pressGood, HIGH);
              digitalWrite(pressBad, LOW);
              //Serial.println(pressureRead);
          }
          else if (pressureRead > 501){
              digitalWrite(pressBad, HIGH);
              digitalWrite(pressGood, LOW);
              //Serial.println(pressureRead);
          }
        
        }
        
       
       



         if (cprCount == 30){ // 30:2 ratio for CPR, 30 total
                cprCount = 0; //restart count
                timeEnd = millis(); //capture end time for set
                //Serial.println(timeEnd);
                totalTime = (timeEnd - timeBegin) / 1000; //convert Ms to seconds
                Serial.print("Total Time: ");
                Serial.println(totalTime);
                bpmCount = ( 30.0 / totalTime); //bpmCount = float
                Serial.print("BPM Count: ");
                Serial.println(bpmCount);
                BPM_LCD = (bpmCount * 60.0);
                Serial.print("Total time in sec: ");
                Serial.println(totalTime);
                lcd.home();
                lcd.print("Rate (BPM): ");
                lcd.setCursor(12,0);
                lcd.print(BPM_LCD);
                Serial.print("BPM is: ");
                Serial.println(BPM_LCD);
                lcd.setCursor(0,1);
                lcd.print("Total Time: ");
                lcd.setCursor(12,1);
                lcd.print(totalTime);
                cprCycle++;
                Serial.print("CPR Cycle #: ");
                Serial.println(cprCycle);
                if (cprCycle == 5){
                    cprCycle = 0;
                }
                
         }
         
         if (cprCount > 5 && cprCount <= 30) {
             float rateTime = (millis() - timeBegin) / 1000; //changed int rateTime to unsigned long
             //Serial.println(rateTime);
             int rate = (cprCount / rateTime) * 60; //changed int to unsigned int
             //Serial.print("BPM is: ");
             //Serial.println(rate);
             
             if (rate >= 100 && rate <= 120 ){ //AHA guidelines 100-120 CPM
                 lcd.setCursor(12,0);
                 lcd.print("Good");
             }
             else {
                 lcd.setCursor(12,0);
                 lcd.print("----");
             }
         }
         
        // Delay
        delay(25); //changed from 50
        lastButtonState = buttonState;
     
       
  }
  /* CPR END */

  /* Pulse START */
  else if(selectMode==2){
     lcd.setCursor(0,0);
      lcd.print("Begin Pulse Check");
      
    if (pulseSensor.sawNewSample()) {
        /*
           Every so often, send the latest Sample.
           We don't print every sample, because our baud rate
           won't support that much I/O.
        */
        if (--samplesUntilReport == (byte) 0) {
          samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

          pulseSensor.outputSample();

          /*
             At about the beginning of every heartbeat,
             report the heart rate and inter-beat-interval.
          */
          if (pulseSensor.sawStartOfBeat()) {
            pulseSensor.outputBeat();
          }
        }

        /*******
          Here is a good place to add code that could take up
          to a millisecond or so to run.
        *******/
      }

  }
  /* Pulse END */
  
} //loop end
