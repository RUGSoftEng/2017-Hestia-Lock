#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <Servo.h>

/* Author:  Lars Holdijk
 * Date:    12-3-2017
 * 
 * The lock peripheral runs on Arduino Yun. It uses the bridgeServer and bridgeClient libraries
 * to make a connection between the operating system on the Yun and the controler that controls the 
 * in-/output pins.
 * 
 * https://www.arduino.cc/en/Reference/YunBridgeLibrary
 */

#define BITS_PER_SECOND 9600
#define GREEN_PIN 3
#define RED_PIN 2
#define ENCA_PIN 7
#define ENCB_PIN 6
#define SERVO_PIN 8
#define BUT_PIN 10

BridgeServer server;
Servo servo;

int encoderPosCount = 0;
int pinALast;
int aVal;
int buttonCount = 0;
int openPosition;
/*
 * Code written for arduino devices works a little different than normal C code.
 * The compiler needs a setup and loop function, the setup function is called at every startup 
 * after this the loop function is called indefinetly.
 */
void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(ENCA_PIN, INPUT);
  pinMode(ENCB_PIN, INPUT);
  pinMode(BUT_PIN, INPUT_PULLUP);

  pinALast = digitalRead(ENCA_PIN);
  openPosition = 10;
  
  Serial.begin(BITS_PER_SECOND);
  Serial.println("Starting");
  Bridge.begin();

  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  BridgeClient client = server.accept();
  
  if(client){
    process(client);
    client.stop();
  }
  
  checkRotation();
  
  checkButtonPress();
  
  setLights();
   
  delay(5);
}

void setLights(){
  if(isLockOpen()){
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, LOW);
   } else {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);    
   }
}

void checkRotation(){
  aVal = digitalRead(ENCA_PIN);
   if (aVal != pinALast){ // Means the knob is rotating
     // if the knob is rotating, we need to determine direction
     // We do that by reading pin B.
     if (digitalRead(ENCB_PIN) != aVal) {  // Means pin A Changed first - We're Rotating Clockwise
       encoderPosCount ++;
     } else {// Otherwise B changed first and we're moving CCW
       encoderPosCount--;
     }
     Serial.println(encoderPosCount);
   } 
   pinALast = aVal;
}

void checkButtonPress(){
  if(digitalRead(BUT_PIN) == LOW){
    buttonCount++;
  } else {
    if(buttonCount >20 && buttonCount < 100){
      if(isLockOpen()){
        closeLock();
      } else {
        openLock();
      }
    } else if(buttonCount > 500){
      reset();
    }
    buttonCount = 0;
  }
}

void reset(){
  Serial.println("Resetting...");
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(RED_PIN, HIGH);
  delay(1000);
  
  encoderPosCount = 0;
  servo.attach(SERVO_PIN);
  while(digitalRead(BUT_PIN)!=LOW){
    servo.write(80);
    checkRotation();
  }
  openPosition = encoderPosCount-5;
  servo.write(90);
  servo.detach();
  Serial.write("NEW OPEN POSITION: ");
  Serial.println(openPosition);
  delay(1000);
}

boolean isLockOpen(){
  if(encoderPosCount > openPosition){
    return true;
  } else {
    return false;
  }
}

boolean isLockClosed(){
  if(encoderPosCount <= 0){
    return true;
  } else {
    return false;
  }
}

/*
 * This methods determines what action the lock should perform based on the URL that was used to reach
 * the peripheral.
 */
void process(BridgeClient client){
  String command = client.readStringUntil('/');
  command = command.substring(0,command.length()-2);
  if(command.equals("openlock")){
    openLock();
  }
  else if(command.equals("closelock")){
    closeLock();
  } else if(command.equals("info")){
    printInfo(client);
  }
  else {
    invalidArgument(client, command);
  }
}

void openLock(){
  servo.attach(SERVO_PIN);
  while(!isLockOpen()){
    servo.write(70);
    checkRotation();
  }
  servo.write(90);
  servo.detach();
}

void closeLock(){
  servo.attach(SERVO_PIN);
  while(!isLockClosed()){
    servo.write(110);
    checkRotation();
  }
  servo.write(90);
  servo.detach();
}

void printInfo(BridgeClient client){
  client.print(F("Lock is currently: "));
  if(isLockOpen()){
    client.println(F("open"));
  } else {
    client.println(F("closed"));
  }
  client.print(F("The lock is is position: "));
  client.println(encoderPosCount);
}

void invalidArgument(BridgeClient client, String command){
  client.print(F("Invalid Argument: "));
  client.print(command);
}

