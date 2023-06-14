//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

TaskHandle_t taskServerCommunicationHandle = nullptr;
#if ENABLE_SERVER_COMMUNICATION_TASK == true

/* Arduino headers */
#include "WiFi.h"
#include "HTTPClient.h"
#include <ArduinoJson.h>

//-------------------------------- Global Variables -------------------------------------

const char* ssid = "OnePlus 8";
const char* password = "abc123def";

String serverName = "http://54.165.59.37:5000/";
String hostname = "ESP32 Node";

float ang;
//-------------------------------- Functions --------------------------------------------

void configureWiFi() {

  WiFi.begin(ssid, password);
  SERIAL_PORT.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  SERIAL_PORT.println("");
  SERIAL_PORT.print("Connected to WiFi network with IP Address: ");
  SERIAL_PORT.println(WiFi.localIP());

  delay(500);
}

uint16_t makeRequest(uint16_t requestType, HTTPClient& http) {
  if (requestType == 0) {
    return http.GET();
  } else if (requestType == 1) {
    http.addHeader("Content-Type", "application/json");
    String timestamp = String(millis());
    String position_x = String(xPosition);
    String position_y = String(yPosition);
    String orientation = String(yaw);
    String tofleft = String(distanceLeftFiltered);
    String tofright = String(distanceRightFiltered);
    String mac = String(WiFi.macAddress());
    String rssi = String(WiFi.RSSI());
    String postData = "{\"diagnostics\": {\"battery\":100,\"connection\":"+rssi+"},\"MAC\":\""+mac+"\",\"nickname\":\"MiWhip\",\"timestamp\":"+timestamp+",\"position\":["+position_x+","+position_y+"],\"whereat\":0,\"orientation\":"+orientation+",\"branches\":[";
    
    float junctionAngle;
    float beaconAngle; 
    bool flag = false;
    while (uxQueueMessagesWaiting(junctionAngleQueue) > 0){
      if (xQueueReceive(junctionAngleQueue, &junctionAngle, 0) == pdTRUE){
        postData = postData + String(junctionAngle) + ",";
        flag = true;
      }
    }
    if (flag){
      postData = postData.substring(0, postData.length()-1);
    }
    postData = postData + "],\"beaconangles\":[";
    flag = false;
  
    while (uxQueueMessagesWaiting(beaconAngleQueue) > 0){
      if (xQueueReceive(beaconAngleQueue, &beaconAngle, 0) == pdTRUE){
        postData = postData + String(beaconAngle) + ",";
        flag = true;
      }
    }
    if (flag){
      postData = postData.substring(0, postData.length()-1);
    }
    postData = postData + "],\"tofleft\":"+tofleft+",\"tofright\":"+tofright+"}";
    SERIAL_PORT.println(postData);
    return http.POST(postData);
  }
}

String handleResponse(uint16_t httpResponseCode, HTTPClient& http) {
  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    SERIAL_PORT.print("HTTP Response code: ");
    SERIAL_PORT.println(httpResponseCode);
  } else if (httpResponseCode >= 400 && httpResponseCode < 500) {
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
  } else if (httpResponseCode >= 500) {
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
  } else {
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
    return "{\"error\": \"Something really wrong54.165.59.37\"}";
  }
  return http.getString();
}

int findCommandLength(String payload){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  return doc["next_actions"].size();
}

void parsePayload(String payload, robotCommand rc[]) {
    SERIAL_PORT.println(payload);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    int clearqueue = doc["clear_queue"];
    if (clearqueue==1){
      xQueueReset(commandQueue);
    }
    SERIAL_PORT.println(clearqueue);

    
    const int length = doc["next_actions"].size();

    int actions[length];
    for (int i=0; i<length; i++){
      actions[i] = doc["next_actions"][i];
    }    

    robotCommand commands[length];

    for (int i=0; i<length; i++){
      switch(actions[i]){
        case 0:
          SERIAL_PORT.println("FORWARD");
          commands[i] = FORWARD;
          break;
        case 1:
          SERIAL_PORT.println("SPIN");
          commands[i] = SPIN;
          break;
        case 2:
          ang = actions[i+1];
          SERIAL_PORT.println(ang);
          commands[i] = TURN;
          break;
        case 3:
          SERIAL_PORT.println("idle");
          doc["next_actions"][i] = IDLE;
          break;
        case 4:{
          xPosition = actions[i+1];
          yPosition = actions[i+2];
          break;
        }
        case 5:{
          SERIAL_PORT.println("DONE");
          commands[i] = IDLE;
          break;
        }
        default:
          break;
      }
    }
    rc = commands;
}


//-------------------------------- Task Functions ---------------------------------------

/* Task to record debug information */
void taskServerCommunication(void* pvParameters) {

  (void)pvParameters;

  static uint16_t requestType;
  static uint16_t httpResponseCode;
  static uint8_t numCommands;
  static robotCommand newCommand;

  const TickType_t xFrequency = configTICK_RATE_HZ / TASK_SERVER_COMMUNICATION_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* Run the task at a set frequency */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);


    String endpoint = "rover";
    requestType = 1;


    String serverPath = serverName + endpoint;

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());

      // Send HTTP request
      httpResponseCode = makeRequest(requestType, http);

      // Manage response
      String payload = handleResponse(httpResponseCode, http);

      // Free resources
      http.end();

      // parse payload string
      numCommands = findCommandLength(payload);
      robotCommand commands[numCommands];
      parsePayload(payload, commands);
      SERIAL_PORT.println("pog1");
      for (int i=0; i<numCommands; i++){
        SERIAL_PORT.println(commands[i]);
        newCommand = commands[i];
        xQueueSend(commandQueue, &newCommand, portMAX_DELAY);
      }
      SERIAL_PORT.println("pog2");
    }
  }
}

#endif