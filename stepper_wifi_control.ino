#include <ESP8266WiFi.h>

const char* ssid     = "D-Shop-Bucharest";// YOUR WIFI SSID
const char* password = "dshopbuh";        // YOUR WIFI PASSWORD 
#define DELAY 1                           // Delay to allow Wifi to work

#define LED 0
#define ENABLE D0
#define DIR D5
#define PULSE D6
#define STEPS 200
#define DELAYSTEP 1000  // in micro seconds
#define STEPS_DIVIDER 3

int position = 0;
int posMin = -200;
int posMax = 200;

// Create an instance of the server
// specify the port to listen on as an argument
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
   digitalWrite(ENABLE, HIGH);
}

void loop() {
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
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
    moveLeft(steps);
    respMsg = String(steps);
    
    Serial.println(req.substring(10));
    Serial.println(String(steps));
    
    blink();
  } 
  else 
  if (req.indexOf("/right/") != -1) {
    
    int steps = req.substring(11).toInt();
    moveRight(steps);
    respMsg = String(steps);
    
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
  
  client.flush();

  String s = "";
  
  if (respMsg.length() > 0){
    s = getResponse("json", respMsg);
  }else{
    s = getResponse("json", printUsage());
  }

  // Send the response to the client
  client.print(s);
  delay(1);
  
  Serial.println("Client disconnected");
  blink();
}

String getResponse(String type, String message){
  
  String html = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  String json = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";

  String s = "";
  
  if(type == "json"){
    s += json +  "{\"message\":\"" + message + "\"}";

  }else{
    s += html + message;
  }

  s += "\n";
  
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

void blink() {
  
  digitalWrite(LED, LOW);
  delay(100);
  
  digitalWrite(LED, HIGH);
  delay(100);
}

int getStepDelay(int step, int total){
  
  float rap = step / total;
  
  int delayT = DELAYSTEP;
  
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

void moveLeft(int steps){

  Serial.println("Moving left : " + String(steps));  
  
  digitalWrite(DIR, HIGH);

  int total = steps * STEPS_DIVIDER;
  
  for(int i=1; i <= total; i++){

     int delayT = getStepDelay(i, total);
     
     digitalWrite(PULSE, HIGH);
     delayMicroseconds(delayT);
     
     digitalWrite(PULSE, LOW);
     delayMicroseconds(delayT);
  }
  
  position -= steps;
}

void moveRight(int steps){

  Serial.println("Moving right : " + String(steps));

  digitalWrite(DIR, LOW);

  int total = steps * STEPS_DIVIDER;

  for(int i=1; i<= total; i++){

     int delayT = getStepDelay(i, total);
    
     digitalWrite(PULSE, HIGH);
     delayMicroseconds(delayT);
     
     digitalWrite(PULSE, LOW);
     delayMicroseconds(delayT);
  }
  
  position += steps;
}
