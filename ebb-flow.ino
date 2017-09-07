#include <RTCZero.h>

/* Create an rtc object */
RTCZero rtc;     //this is the real time clock for the time control

/* Definition of Solenoid Valves */
int ValveOpenPin = 3;    //Open valve (i.e. next to the tank) is controlled by Analog pin 3
int ValveClosePin = 4;  //Close valve (i.e. under the tray) is controlled by Analog pin 4

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 25;
const byte hours = 9;

/* Change these values to set the current initial date */
const byte day = 5;
const byte month = 9;
const byte year = 17;
unsigned long irrigation_time=300000; // 5 mins
unsigned long check_time=3600000; //one hour


void setup()
{
  Serial.begin(9600);

  rtc.begin(); // initialize RTC
  pinMode(ValveOpenPin, OUTPUT);           //Sets the pin as an output
  pinMode(ValveClosePin, OUTPUT); 
  // Set the time
  rtc.setHours(hours);
  rtc.setMinutes(minutes);
  rtc.setSeconds(seconds);

  // Set the date
  rtc.setDay(day);
  rtc.setMonth(month);
  rtc.setYear(year);

}

void loop()
{
//get hour
  int hour=rtc.getHours();
//get humidity levels from the water level sensor, connected to Analog pin 1
  int humidity = analogRead(A1);
  Serial.println(humidity); // printing the values to check if is working correctly
 
  humidity = 0;

  if(hour==9){ //it is going to irrigate at 9 am
    
    while(humidity<=900){ //if the water hasn't reached the desired level, keep the valve open. (900 is the aprox threshold value when water is 2.5 cm deep. 
      digitalWrite(ValveOpenPin, HIGH); //open water valve from the tank, starts flooding
      humidity = analogRead(A1); //each cycle update the new humidity value
      Serial.println(humidity);
      delay(2000);
    }
    digitalWrite(ValveOpenPin, LOW); //once the humidty value is 900 or more, the cycle stops and this line closes the valve
    delay(irrigation_time); // hold the program for 5 minutes, flooding time for the soil and plants to absorb the water
  }
  digitalWrite(ValveClosePin, HIGH); //once irrigation time is done, we open the second valve (or activate pump) to flush the water contained
  while(humidity>=600){ //this cycle keeps the exit valve open until the humidity value is very low, that means the water has been flushed out of the tray
     digitalWrite(ValveClosePin, HIGH);
     humidity = analogRead(A1); //keep updating the humidty value
    delay(2000);
  }
  digitalWrite(ValveClosePin, LOW); //once the water level sensor measures close to no water in the tray, we can turn the exit valve off


  delay(check_time); //the program checks every hour if it is time to irrigate or not. you can control this in the definitions.
}


