String inString;
#include <Servo.h>
#include <Stepper.h>
Servo myservo;

#define STEPS 100

Stepper stepper(STEPS, 4, 5, 6, 7);

void setup()
{
  // start serial port at 9600 bps:
  Serial.begin(9600);
  pinMode(12, OUTPUT);
  myservo.attach(9);
  myservo.write(110);
  stepper.setSpeed(17);
}

void loop()
{
  
//inString = "BB\"!";
//myservo.write(131);
//Serial.print(inString);
  if (Serial.available() > 0) {//put everything in this loop?
    inString = Serial.readStringUntil('!');
    //Serial.print(inString);
  }

//blower
  if (inString.charAt(0) == 66){
    digitalWrite(12, HIGH);
    //myservo.write(constrain(map(int(inString.charAt(2)), 34, 84, 81, 131), 10, 131));
    myservo.write(constrain(map(int(inString.charAt(2)), 34, 122, 10, 131), 10, 131));
    //myservo.write(constrain(int(inString.charAt(2)), 34, 155) - 24);
    //Serial.println(constrain(map(int(inString.charAt(2)), 34, 122, 10, 131), 10, 131));
  }
  else{
    digitalWrite(12, LOW);
    myservo.write(70);
  }

//rotate
  if (inString.charAt(1) == 65){
    stepper.step(-1);
    //delay(50);
  }
  else if(inString.charAt(1) == 67){
    stepper.step(1);
    //delay(50);
  }
  else {
    stepper.step(0);
  }
//servo
//include wihtin blower?

 //Serial.flush();  
}
