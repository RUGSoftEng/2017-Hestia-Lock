#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <Servo.h>

BridgeServer server;

void setup() {
  pinMode(13, OUTPUT);
  
  Serial.begin(9600);
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
  digitalWrite(13, HIGH);
  client.print(F("Lock open"));
}

void closeLock(BridgeClient client){
  digitalWrite(13, LOW);
  client.print(F("Lock close"));
}

void invalidArgument(BridgeClient client, String command){
  client.print(F("Invalid Argument: "));
  client.print(command);
}

