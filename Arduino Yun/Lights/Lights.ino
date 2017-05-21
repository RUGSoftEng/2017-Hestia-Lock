#include <QueueList.h>

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
#define LOOP_INTERVAL 50
#define SETUP_TIME 5000
#define BUT_PIN 10
#define SERVO_PIN 11
#define SETUP "setup\r\n"
#define ON "on"
#define LEDS "leds"
#define NUMBER_LEDS 6

BridgeServer server;
BridgeClient client;
int setupDuration = 0;
String key = "HASasdA092Qe43-adfASD";
int leds[NUMBER_LEDS];

/*
 * Code written for arduino devices works a little different than normal C code.
 * The compiler needs a setup and loop function, the setup function is called at every startup 
 * after this the loop function is called indefinetly.
 */
void setup() {
  for(int i = 0; i< NUMBER_LEDS; i++){
    pinMode(i, OUTPUT);
  }
  pinMode(BUT_PIN, INPUT_PULLUP);
  
  Serial.begin(BITS_PER_SECOND);
  Bridge.begin();

  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  client = server.accept();
  
  if(client){
    process(client);    
    client.stop();
  }   
  
  if(digitalRead(BUT_PIN)==LOW){
    setupDuration = SETUP_TIME;
  } else if(setupDuration > 0) {    
    setupDuration -= LOOP_INTERVAL;
    delay(LOOP_INTERVAL);
  }
}

/*
 * This methods determines what action the lock should perform based on the URL that was used to reach
 * the peripheral.
 */
void process(BridgeClient client){
  String command = client.readStringUntil('/');
  Serial.println(command);
  if(command == SETUP){
    setupConnection(client);
    return;
  }else if(command == key){
    command = client.readStringUntil('/');
    Serial.println(command);
    if(command == ON){
      onAction(client);
      return;
    } else if(command == LEDS){
      ledAction(client);
      return;
    } else{
      invalidArgument(client);
      return;
    }
  } else {
    invalidArgument(client);
    return;
  }
}

void setupConnection(BridgeClient client){
  if(setupDuration > 0){
    client.print(key);
    setupDuration = 0;    
    client.stop();
  } else {
    invalidArgument(client);
  }
  
    client.stop();
}

void onAction(BridgeClient client){
  int value = client.parseInt();
  Serial.println(value);
  if(value == 1){
    for(int i = 0; i < NUMBER_LEDS; i++){
      digitalWrite(i, LOW);      
      Serial.println("lights off");
    }
  } else {
    for(int i = 0; i < NUMBER_LEDS; i++){
      digitalWrite(i, HIGH);      
      Serial.println("lights on");
    }
  }
}

void ledAction(BridgeClient client){
    int value = client.parseInt();
    
    for(int i = 0; i < value; i++){
      digitalWrite(i, HIGH);      
    }

    for(int i = 0; i < NUMBER_LEDS ; i++){
      digitalWrite(i, LOW);  
    }
}

void invalidArgument(BridgeClient client){
  client.println(F("Usage:"));
  client.println(F("/setup"));
  client.println(F("/<string:id>/rotate/<int:rotation>"));
  client.println(F("/<string:id>/on/<int:on/off>"));
}

