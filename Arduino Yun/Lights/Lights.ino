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
#define LED_PIN 9
#define BUT_PIN 10
#define SERVO_PIN 11
#define SETUP "setup\r\n"
#define ON "on"
#define ROTATE "rotate"

BridgeServer server;
BridgeClient client;
Servo servo;
int setupDuration = 0;
String key = "HASasdA092Qe43-adfASD";

/*
 * Code written for arduino devices works a little different than normal C code.
 * The compiler needs a setup and loop function, the setup function is called at every startup 
 * after this the loop function is called indefinetly.
 */
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUT_PIN, INPUT_PULLUP);
  
  Serial.begin(BITS_PER_SECOND);
  Bridge.begin();

  server.listenOnLocalhost();
  server.begin();
  servo.attach(SERVO_PIN);
  servo.write(90);
}

void loop() {
   client = server.accept();
  if(client){
    process(client);
    client.stop();
  }
  delay(LOOP_INTERVAL);
  if(digitalRead(BUT_PIN)==LOW){
    setupDuration = SETUP_TIME;
  } else if(setupDuration > 0) {
    setupDuration -= LOOP_INTERVAL;
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
  }else if(command == key){
    command = client.readStringUntil('/');
    Serial.println(command);
    if(command == ON){
      on(client);
    } else if(command == ROTATE){
      rotate(client);
    } else{
      invalidArgument(client);
    }
  } else {
    invalidArgument(client);
  }
}

void setupConnection(BridgeClient client){
  if(setupDuration > 0){
    client.print(key);
    setupDuration = 0;
  } else {
    invalidArgument(client);
  }
}

void on(BridgeClient client){
  int value = client.parseInt();
  Serial.println(value);
  if(value == 1){
    digitalWrite(LED_PIN, HIGH);
    Serial.println("light on");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("light off");
  }
}

void rotate(BridgeClient client){
    int value = client.parseInt();
    Serial.println(value);
    if(88<=value && value<=92){
      servo.write(90);
      delay(15);
    } else{      
      servo.write(value);
      delay(15);
      Serial.println("Servo running");
    }
}

void invalidArgument(BridgeClient client){
  client.println(F("Usage:"));
  client.println(F("/setup"));
  client.println(F("/<string:id>/rotate/<int:rotation>"));
  client.println(F("/<string:id>/on/<int:on/off>"));
}

