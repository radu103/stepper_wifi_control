#include <ESP8266WiFi.h>

// wifi config
const char* ssid     = "D-Shop-Bucharest";// YOUR WIFI SSID
const char* password = "dshopbuh";        // YOUR WIFI PASSWORD 

// config specific to motor controller
//#define NAME "Base"
//#define MINSTEPS -181
//#define MAXSTEPS 181
//#define STEPS_DIVIDER 3

#define NAME "Body"
#define MINSTEPS 0
#define MAXSTEPS 13201
#define STEPS_DIVIDER 1

// stepper motor config
#define LED 0
#define ENABLE D0
#define DIR D5
#define PULSE D6
#define STEPSPERCIRCLE 200
#define DELAYSTEP 1000  // in micro seconds
#define MINDELAYSTEP 1000  // in micro seconds

// running vars
int position = 0;
float angle = 0;
WiFiServer server(80);

// init function
void setup() {
  
  Serial.begin(115200);
  delay(10);

  // prepare onboard LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  String vers = ESP.getCoreVersion();
  Serial.print("ESP version : ");
  Serial.println(vers);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  
  Serial.print(NAME);
  Serial.print(" : Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    blink(3000);
  }
  
  Serial.println("");
  
  Serial.print(NAME);
  Serial.println(" : WiFi connected");
  
  // Start the server
  server.begin();

  Serial.print(NAME);
  Serial.println(" : Server started");

  // Print the IP address
  Serial.print(NAME);
  Serial.print(" IP : ");
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

  String respMsg = "";
  
  // Wait until the client sends some data
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  yield();

  boolean isGET = req.indexOf("GET") >= 0;
  Serial.println(req); 
  
  // Match the request
  if (isGET && req.indexOf("/left/") != -1) {
    respMsg = handleMoveStepsLeft(req);
  } 
  else
  if (isGET && req.indexOf("/right/") != -1) {
    respMsg = handleMoveStepsRight(req);
  }
  else
  if (isGET && req.indexOf("/origin/set") != -1) {
    respMsg = handleSetOrigin();
  }
  else  
  if (isGET && req.indexOf("/origin/go") != -1) {
    respMsg = handleGoToOrigin();
  }
  else
  if (isGET && req.indexOf("/position/get") != -1) {
    respMsg = handleGetPosition();
  }
  else
  if (isGET && req.indexOf("/position/set") != -1) {
   respMsg =  handleSetPosition(req);
  }
  else    
  if (isGET && req.indexOf("/position") != -1) {
    respMsg = handleGetPosition();
  }
  if (isGET && req.indexOf("/angle/get") != -1) {
    respMsg = handleGetAngle();
  }
  else
  if (isGET && req.indexOf("/angle/set") != -1) {
   respMsg =  handleSetAngle(req);
  }
  else    
  if (isGET && req.indexOf("/angle") != -1) {
    respMsg = handleGetAngle();
  }
  else
  if (isGET && req.indexOf("/enable") != -1) {
    respMsg = handleDriverEnable();
  }
  else
  if (isGET && req.indexOf("/disable") != -1) {
    respMsg = handleDriverDisable();
  }  
  if (isGET && req.indexOf("/reset") != -1) {
    handleSoftReset();
  }
  
  client.flush();

  yield();

  String s = "";
  
  if (respMsg.length() > 0)
  {
    s = getResponse("json", respMsg);
  }
  else
  {
    s = getResponse("html", printUsage());
  }

  // Send the response to the client
  client.print(s);
  delay(1);
  
  Serial.println("client disconnected");
  blink();
}

void handleSoftReset(){
    Serial.println("Reset requested!");
    blink();
    ESP.restart();
}

String handleDriverEnable(){
    String respMsg = "true";
    Serial.println("Stepper Driver enabled");
    digitalWrite(ENABLE, LOW);
    blink();
    return respMsg;
}

String handleDriverDisable(){
    String respMsg = "false";
    Serial.println("Stepper Driver disabled");
    digitalWrite(ENABLE, HIGH);
    blink();
    return respMsg;
}

String handleGetAngle(){
  
    String respMsg = String(angle);
    Serial.println("Position : " + String(position));
    blink();

    return respMsg;
}

String handleSetAngle(String req){
    String respMsg = "NOT IMPLEMENTED";
    Serial.println("NOT IMPLEMENTED : handleSetAngle");
    blink();

    return respMsg;
}

String handleGetPosition(){
  
    String respMsg = String(position);
    Serial.println("Position : " + String(position));
    blink();

    return respMsg;
}

String handleSetPosition(String req){

    String auxP = req.substring(19);
    String posString = auxP.substring(0, auxP.indexOf(" "));
    int pos = posString.toInt();
    
    int resP = 0;
    if(pos > position){
      resP = moveRight(position - pos);
    }
    else
    {
      resP = moveLeft(pos - position);
    }
    
    String respMsg = String(resP);
    
    Serial.println(String(resP));
    
    blink();

    return respMsg;
}

String handleSetOrigin(){
    position = 0;
    String respMsg = "0";
    
    Serial.println("ORIGIN SET HERE (0)");
    blink();

    return respMsg;
}

String handleGoToOrigin(){
  
    if(position > 0){
      moveLeft(position);
    }
    else{
      moveRight(-1 * position);
    }
    
    position = 0;
    String respMsg = "0";
    
    Serial.println("GO TO ORIGIN (0)");
    blink();

    return respMsg;
}

String handleMoveStepsLeft(String req){
    
    String auxL = req.substring(10);
    String leftString = auxL.substring(0, auxL.indexOf(" "));
    int stepsL = leftString.toInt();
    
    int resL = moveLeft(stepsL);
    String respMsg = String(resL);
    
    Serial.println(String(stepsL));
    
    blink();

    return respMsg;
}

String handleMoveStepsRight(String req){
  
    String auxR = req.substring(11);
    String rightString = auxR.substring(0, auxR.indexOf(" "));
    int stepsR = rightString.toInt();
    
    int resR = moveRight(stepsR);
    String respMsg = String(resR);
    
    Serial.println(String(stepsR));
    
    blink();

    return respMsg;
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
  delay(delayMs / 2);

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

  if(total - step >= STEPSPERCIRCLE){
    delayT = MINDELAYSTEP;
  }
  else  
  if(step < STEPSPERCIRCLE){
    delayT = DELAYSTEP * 2;
  }
  else
  if(step < STEPSPERCIRCLE / 2){
    delayT = DELAYSTEP * 4;
  }
  else
  if(step < STEPSPERCIRCLE / 4){
    delayT = DELAYSTEP * 6;
  }  
  else
  if(step < STEPSPERCIRCLE / 8){
    delayT = DELAYSTEP * 8;
  }    
  if(step < STEPSPERCIRCLE / 16){
    delayT = DELAYSTEP * 10;
  }
  else  
  if(total - step < STEPSPERCIRCLE){
    delayT = DELAYSTEP * 2;
  }
  else
  if(total - step < STEPSPERCIRCLE / 2){
    delayT = DELAYSTEP * 4;
  }
  else
  if(total - step < STEPSPERCIRCLE / 4){
    delayT = DELAYSTEP * 6;
  }  
  else
  if(total - step < STEPSPERCIRCLE / 8){
    delayT = DELAYSTEP * 8;
  }    
  if(total - step < STEPSPERCIRCLE / 16){
    delayT = DELAYSTEP * 10;
  }  

  return delayT;
}

int moveLeft(int steps){

  if(position - steps < MINSTEPS){
    Serial.println("Moving left : OUT OF RANGE"); 
    return(0);
  }

  Serial.println("Moving left : " + String(steps));
  
  digitalWrite(DIR, LOW);
  delayMicroseconds(DELAYSTEP);

  int total = steps * STEPS_DIVIDER;
  int i=1;
  int delayT = DELAYSTEP;
  
  for(i=1; i <= total; i++){

     if(i % 2 == 0){
        delayT = getStepDelay(i, total);
     }

     digitalWrite(DIR, LOW);
     
     digitalWrite(PULSE, LOW);
     delayMicroseconds(delayT);
     
     digitalWrite(PULSE, HIGH);
     delayMicroseconds(delayT);

     yield();
  }

  position -= steps;

  return steps;
}

int moveRight(int steps){

  if(position + steps > MAXSTEPS){
    Serial.println("Moving right : OUT OF RANGE"); 
    return(0);
  }

  Serial.println("Moving right : " + String(steps));

  digitalWrite(DIR, HIGH);
  delayMicroseconds(DELAYSTEP);

  int total = steps * STEPS_DIVIDER;
  int i=1;
  int delayT = DELAYSTEP;
  
  for(i=1; i<= total; i++){

     if(i % 2 == 0){
        delayT = getStepDelay(i, total);
     }

     digitalWrite(DIR, HIGH);
    
     digitalWrite(PULSE, LOW);
     delayMicroseconds(delayT);
     
     digitalWrite(PULSE, HIGH);
     delayMicroseconds(delayT);

     yield();
  }

  position += steps;

  return steps;
}
