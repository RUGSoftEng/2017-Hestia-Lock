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
#define LED_PIN 13

BridgeServer server;

/*
 * Code written for arduino devices works a little different than normal C code.
 * The compiler needs a setup and loop function, the setup function is called at every startup 
 * after this the loop function is called indefinetly.
 */
void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(BITS_PER_SECOND);
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
  delay(50);
}

/*
 * This methods determines what action the lock should perform based on the URL that was used to reach
 * the peripheral.
 */
void process(BridgeClient client){
  String command = client.readStringUntil('/');
  command = command.substring(0,command.length()-2);
  if(command.equals("openlock")){
    openLock(client);
  }
  else if(command.equals("closelock")){
    closeLock(client);
  }
  else {
    invalidArgument(client, command);
  }
}

void openLock(BridgeClient client){
  digitalWrite(LED_PIN, HIGH);
  client.print(F("Lock open"));
}

void closeLock(BridgeClient client){
  digitalWrite(LED_PIN, LOW);
  client.print(F("Lock close"));
}

void invalidArgument(BridgeClient client, String command){
  client.print(F("Invalid Argument: "));
  client.print(command);
}

