#include <ESP8266WiFi.h>

// wifi config
const char* ssid     = "D-Shop-Bucharest";// YOUR WIFI SSID
const char* password = "dshopbuh";        // YOUR WIFI PASSWORD 
#define DELAY 1                           // Delay to allow Wifi to work

// config specific to motor controller
#define NAME "Body"
#define MINSTEPS 0
#define MAXSTEPS 4400

// stepper motor config
#define LED 0
#define ENABLE D0
#define DIR D5
#define PULSE D6
#define STEPS 200
#define DELAYSTEP 500  // in micro seconds
#define STEPS_DIVIDER 3

// running vars
int position = 0;
WiFiServer server(80);

void setup() {
  
  Serial.begin(115200);
  delay(10);

  // prepare onboard LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("STEPPER: Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    blink(3000);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Blink onboard LED to signify its connected
  blink();
  blink();
  blink();
  blink();

   // Motor Config
   pinMode(ENABLE, OUTPUT);
   pinMode(DIR, OUTPUT);
   pinMode(PULSE, OUTPUT);
   
   digitalWrite(PULSE, LOW);
   digitalWrite(ENABLE, LOW);
}

void loop() {
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    blink();
    return;
  }

  String respMsg = "";    // HTTP Response Message
  
  // Wait until the client sends some data
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request 
  if (req.indexOf("/left/") != -1) {
    int steps = req.substring(10).toInt();
    int res = moveLeft(steps);
    respMsg = String(res);
    
    Serial.println(req.substring(10));
    Serial.println(String(steps));
    
    blink();
  } 
  else 
  if (req.indexOf("/right/") != -1) {
    
    int steps = req.substring(11).toInt();
    int res = moveRight(steps);
    respMsg = String(res);
    
    Serial.println(req.substring(11));
    Serial.println(String(steps));
    
    blink();
  }
  else
  if (req.indexOf("/origin") != -1) {

    if(position > 0){
      moveLeft(position);
    }
    else{
      moveRight(-1 * position);
    }
    
    position = 0;
    respMsg = "0";
    
    Serial.println("GO TO ORIGIN (0)");
    blink();
  }
  else
  if (req.indexOf("/position") != -1) {
    respMsg = String(position);
    Serial.println("Position : " + String(position));
    blink();
  }
  else
  if (req.indexOf("/reset") != -1) {
    respMsg = String(position);
    Serial.println("Reset requested!");
    blink();
    ESP.restart();
  }
  
  client.flush();

  String s = "";
  
  if (respMsg.length() > 0){
    s = getResponse("json", respMsg);
  }else{
    s = getResponse("html", printUsage());
  }

  // Send the response to the client
  client.print(s);
  delay(1);
  
  Serial.println("Client disconnected");
  blink();
}

String getResponse(String type, String message){
  
  String html = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nAccess-Control-Allow-Origin:*\r\n\r\n";
  String json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin:*\r\n\r\n";

  String s = "";
  
  if(type == "json"){
    s += json +  "{\"name\":\"" + NAME + "\",\"message\":\"" + message + "\"}";

  }else{
    s += html + NAME + "<br/>\r\n" + message;
  }
  
  return s;
}

String printUsage() {
  
  String ip = WiFi.localIP().toString();
  String s = "Stepper usage:<br/><br/>\n";
  
  s += "<a href=\"http://"+ ip + "/origin\">http://"+ ip + "/origin</a><br/>\n";
  s += "<a href=\"http://"+ ip + "/position\">http://"+ ip + "/position</a><br/>\n";
  s += "<a href=\"http://"+ ip + "/left/1\">http://"+ ip + "/left/{steps}</a><br/>\n";
  s += "<a href=\"http://"+ ip + "/right/1\">http://"+ ip + "/right/{steps}</a><br/>\n";
  
  return(s);
}

void blink(int delayMs) {
  
  digitalWrite(LED, HIGH);
  delay(1000);

  digitalWrite(LED, LOW);
  delay(delayMs);
}

void blink() {
  
  digitalWrite(LED, LOW);
  delay(100);
  
  digitalWrite(LED, HIGH);
  delay(100);
}

int getStepDelay(int step, int total){
 
  int delayT = DELAYSTEP;
  
  float rap = step / total;
    
  if(rap < 0.1){
    delayT = round(DELAYSTEP * 1.5);
  }else
  if(rap > 0.9){
    delayT = round(DELAYSTEP * 1.5);
  }else
  if(rap >= 0.2 && rap <= 0.8){
    delayT = delayT - round(delayT * 0.4);
  }

  return delayT;
}

int moveLeft(int steps){

  if(position - steps < MINSTEPS){
    Serial.println("Moving left : OUT OF RANGE"); 
    return(0);
  }

  Serial.println("Moving left : " + String(steps));  

  digitalWrite(ENABLE, HIGH);
  delayMicroseconds(DELAYSTEP);
  
  digitalWrite(DIR, HIGH);
  delayMicroseconds(DELAYSTEP);

  int total = steps * STEPS_DIVIDER;
  int i=1;
  int delayT = DELAYSTEP;
  
  for(i=1; i <= total; i++){

     if(i % 10 == 0){
        delayT = getStepDelay(i, total);
     }
     
     digitalWrite(PULSE, HIGH);
     delayMicroseconds(delayT);
     
     digitalWrite(PULSE, LOW);
     delayMicroseconds(delayT);

     yield();
  }

  position -= steps;

  digitalWrite(ENABLE, LOW);

  return steps;
}

int moveRight(int steps){

  if(position + steps > MAXSTEPS){
    Serial.println("Moving right : OUT OF RANGE"); 
    return(0);
  }

  Serial.println("Moving right : " + String(steps));

  digitalWrite(ENABLE, HIGH);
  delayMicroseconds(DELAYSTEP);

  digitalWrite(DIR, LOW);
  delayMicroseconds(DELAYSTEP);

  int total = steps * STEPS_DIVIDER;
  int i=1;
  int delayT = DELAYSTEP;
  
  for(i=1; i<= total; i++){

     if(i % 10 == 0){
        delayT = getStepDelay(i, total);
     }
    
     digitalWrite(PULSE, HIGH);
     delayMicroseconds(delayT);
     
     digitalWrite(PULSE, LOW);
     delayMicroseconds(delayT);

     yield();
  }

  position -= steps;

  digitalWrite(ENABLE, LOW);

  return steps;
}
