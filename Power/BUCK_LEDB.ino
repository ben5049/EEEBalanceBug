//Based on Buck_OLCL.ino provided by power lab.

#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

float open_loop, closed_loop, delta;  // Duty Cycles
float vout, vin, vref = 3, vret, iL, dutyref, current_mA;                // Measurement Variables
unsigned int sensorValue0, sensorValue1, sensorValue2, sensorValue3;     // ADC sample values declaration
float current_limit = 300;
float iref = 50;
float cumError=0, prevError=0,Kp=0.0008,Ki=0.021,Kd=0.012;
const char* ssid = "Digo111";
const char* password = "88888888";
float loopLastTime = micros();
float starttime = micros();
bool sw_B=0;
String serverName = "http://54.165.59.37:5000/";



void setup() {

  //Basic pin setups


  pinMode(0, OUTPUT);       //pin0 is PWM pin
  pinMode(1, OUTPUT);       //pin1 is PWM enable pin
  digitalWrite(1,LOW);    //always enable PWM signal
  analogWriteFreq(100000);  //set the PWM frequency at 100kHz
  Serial.begin(115200);     //serial communication enable. Used for program debugging.
  delta = 0.01; 
  closed_loop = 0.4;  //initial value of dutycycle0
}

void loop() {

sampling();
if((micros()-starttime)>100){
if(iL<=current_limit){
  //adjust dutycycle according to measured vout, vref is set to be 3V
  
  closed_loop=PID(iref,iL,loopLastTime,cumError,prevError,Kp,Ki,Kd);
  loopLastTime = millis();
  starttime= micros();
}
else{
  closed_loop=closed_loop-20;
  starttime= micros();
}
}
// Serial.print("Input:");
// Serial.println(iL);
// Serial.print(",Setpoint:");
// Serial.println(iref);
// Serial.print(",DutyCycle:");
// Serial.println(closed_loop);
// Serial.print(",High:");
// Serial.println(0);
// Serial.print(",Low:");
// Serial.println(200);
analogWrite(0,(int)(closed_loop));

  //used to debug
  // Serial.print("Vout: ");
  // Serial.print(vout);
  // Serial.print("\t");

  // Serial.print("Vin: ");
  // Serial.print(vin);
  // Serial.print("\t");


  // Serial.print("closed_loop: ");
  // Serial.print(closed_loop);
  // Serial.print("\t");

  // Serial.print("Inductor Current: ");
  // Serial.print(iL);
  // Serial.print("\n");

  delay(1);
}

void setup1() {
  Serial.begin(115200);


  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(1000);
    Serial.print(".");

  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  delay(1000);
}

void loop1() {
  String endpoint = "led_driver/blue";
  int requestType = 1;

  String serverPath = serverName + endpoint;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    Serial.print(WiFi.status());
    Serial.print("\n");
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP request
    int httpResponseCode = makeRequest(requestType, http);

    // Manage response
    String payload = handleResponse(httpResponseCode, http);
    int flag=payload[36]-'0';
    Serial.print(payload);
    if (flag==1) {
      digitalWrite(1,HIGH);
      Serial.print("on!");
      // Serial.print("Inductor Current: ");
      // Serial.print(iL);
      // Serial.print("\t");
      Serial.print("\n");
    } else {
      digitalWrite(1,LOW);
      Serial.print("off!");
      // Serial.print("Inductor Current: ");
      // Serial.print(iL);
      // Serial.print("\t");
      Serial.print("\n");
    }
    // Free resources
    http.end();
    parsePayload(payload);
//  if(sw_B==0)
//     {
//       sw_B=1;
//     }
//     else
//     {
//       sw_B=0;
//     }
//     delay(5000);
      delay(1000);
  }
  else
  {

digitalWrite(1,LOW);
  }
}







// This subroutine processes all of the analogue samples, creating the required values for the main loop

void sampling() {

  // Make the initial sampling operations for the circuit measurements

  sensorValue2 = analogRead(28);  //sample Vout
  sensorValue0 = analogRead(26);  //sample Vret
  sensorValue1 = analogRead(27);  //sample Vin


  // Process the values so they are a bit more usable/readable
  // The analogRead process gives a value between 0 and 1023
  // representing a voltage between 0 and the analogue reference which is 3.3V

  vout = sensorValue2 * (12500 / 2500) * (3.43 / 1023.0);  // Convert the Vout sensor reading to volts
  vret = sensorValue0 * (3.43 / 1023.0);                   // Convert the Vret sensor reading to volts, Vret is used to measure the current
  vin = sensorValue1 * (12500 / 2500) * (3.43 / 1023.0);   // Convert the Vin sensor reading to volts

  iL = vret * 1000 / 1.02;  //measureing inductor current in mA
}

float saturation(float sat_input, float uplim, float lowlim) {  // Saturatio function
  if (sat_input > uplim) sat_input = uplim;
  else if (sat_input < lowlim) sat_input = lowlim;
  else
    ;
  return sat_input;
}

void pwm_modulate(float pwm_input) {  // PWM function
  analogWrite(0, (int)(pwm_input * 255));
}


int makeRequest(int requestType, HTTPClient& http) {
  if (requestType == 0) {
    return http.GET();
  } else {
    http.addHeader("Content-Type", "application/json");
   String VOUT = String(vout);
    String IOUT = String(iL);
    // String SWR = String(sw_R);
    String VIN = String(vin);
    String postData = "{\"vout\":" + VOUT + ",\"iout\":" + IOUT + ",\"vin\":"+ VIN+"}";
    return http.POST(postData);
  }
}


String handleResponse(int httpResponseCode, HTTPClient& http) {
  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else if (httpResponseCode >= 400 && httpResponseCode < 500) {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  } else if (httpResponseCode >= 500) {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    return "{\"error\": \"Something really wrong...\"}";
  }
  return http.getString();
}


void parsePayload(String payload) {
  JSONVar data = JSON.parse(payload);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(data) == "undefined") {
    Serial.println("Parsing input failed!");
    delay(1000);
    return;
  }
}

float PID(float setpoint, float input,float lastTime, float& cumError, float& prevError,  float Kp, float Ki,float Kd) {
  float error;
  float output;
  float DT = millis() - lastTime;
  error = setpoint - input;
  cumError += constrain(error, -1000, 1000);
  cumError = constrain(cumError, -10000, 10000);
  output = Kp * error + Ki * cumError + Kd*(error-prevError)/DT;
  prevError = error;
  return constrain(output,1,254);
}







/*end of the program.*/
