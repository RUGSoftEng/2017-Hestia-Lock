#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <Servo.h>

#define LEDPIN 13;
#define BITS 9600;

BridgeServer server;

void setup() {
  pinMode(LEDPIN, OUTPUT);

  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  BridgeClient client = server.accept();
  if(client){
    process(client);
    client.stop();
  }
  delay(50);
}

void process(BridgeClient client){
  String turnDirection = client.readStringUntil('/');
  turnDirection = turnDirection.substring(0, turnDirection.length()-2);
  int durationInMicrosec = Integer.parseInt(client.readStringUntil('/'));
  int turnSpeed = Integer.parseInt(client.readStringUntil('/');

  if(!checkTurnSpeed(turnSpeed)){
    client.println(F("TurnSpeed is not valid));
  }

  int motorControl = 0;
  if(turnDirection.equals("ccw"){
    motorControl = 90 + (turnSpeed * 10);
  } if(turnDirection.equals("cw"){
    motorControl = 90 - (turnSpeed * 10);
  } else {
    client.println(F("Direction is not valid));
  }
  
  turnMotor(motorControl, durationInMicrosec);
}

void turnMotor(int motorControl, int duration){
  int elapsed = 0;
  while(elapsed < duration){
    // Turn motor
    elapsed += 10;
    delay(10);
  }
}

bool checkTurnSpeed(int turnSpeed){
  if(turnSpeed < 0 || turnSpeed > 10 ){
    return false;
  } else {
    return true;
  }
}

