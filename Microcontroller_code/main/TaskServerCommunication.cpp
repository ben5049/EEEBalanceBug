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
volatile whereAt currentwhereAt = PASSAGE;
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
    String cwa = String(currentwhereAt);
    String battery = String(analogRead(VBAT)*4*3.3*1.1/4096);
    String postData = "{\"diagnostics\": {\"battery\":"+battery+",\"connection\":"+rssi+"},\"MAC\":\""+mac+"\",\"nickname\":\"MiWhip\",\"timestamp\":"+timestamp+",\"position\":["+position_x+","+position_y+"],\"whereat\":"+cwa+",\"orientation\":"+orientation+",\"branches\":[";
    
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
    // SERIAL_PORT.println("I'm outside");
    // SERIAL_PORT.println(uxQueueMessagesWaiting(beaconAngleQueue));
    // SERIAL_PORT.println(uxQueueSpacesAvailable(beaconAngleQueue));
    
    if (uxQueueMessagesWaiting(beaconAngleQueue)==3){
      SERIAL_PORT.println("I'm inside");
      while (uxQueueMessagesWaiting(beaconAngleQueue) > 0){
        if (xQueueReceive(beaconAngleQueue, &beaconAngle, 0) == pdTRUE){
          postData = postData + String(beaconAngle) + ",";
          flag = true;
          SERIAL_PORT.println(beaconAngle);
        }
      }
    }

    if (flag){
      postData = postData.substring(0, postData.length()-1);
    }
    postData = postData + "],\"tofleft\":"+tofleft+",\"tofright\":"+tofright+"}";
    return http.POST(postData);
  }
}

String handleResponse(uint16_t httpResponseCode, HTTPClient& http) {
  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    SERIAL_PORT.println(httpResponseCode);
  } else if (httpResponseCode >= 400 && httpResponseCode < 500) {
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
  } else if (httpResponseCode >= 500 && httpResponseCode<600) {
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
  } else {
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
    return "{\"error\": \"Server not up\"}";
  }
  return http.getString();
}
/* Get size of response sent */
int findCommandLength(String payload){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  return doc["next_actions"].size();
}

/* Convert JSON to array of commands */
void parsePayload(String payload, robotCommand rc[], int httpResponseCode) {
    /* Check to ensure data has been received */
    if (httpResponseCode >= 200 && httpResponseCode < 300){
      /* Extract JSON information */
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      
      /*  Get number of commands and create array with that size */
      const int length = doc["next_actions"].size();
      int actions[length];

      /* 
        Can't directly use next_actions array as it is a String& instead 
        of an int[], but next_actions[i] is an int (???), so we iterate 
      ' and store in a new array
      */

      for (int i=0; i<length; i++){
        actions[i] = doc["next_actions"][i];
      }
      
      /* Convert next actions to robot commands */
      robotCommand commands[length];

      for (int i=0; i<length; i++){
        switch(actions[i]){
          case 0:
            commands[i] = FORWARD;
            break;
          case 1:
            commands[i] = SPIN;
            break;
          case 2:
            commands[i] = TURN;
            ang = actions[i+1];
            xQueueSend(angleSetpointQueue, &ang, portMAX_DELAY);
            i++;
            break;
          case 3:
            doc["next_actions"][i] = IDLE;
            break;
          case 4:{
            xPosition = actions[i+1];
            yPosition = actions[i+2];
            i+=2;
            break;
          }
          case 5:{
            commands[i] = IDLE;
            break;
          }
          default:
            break;
        }
      }
      /* Clear queue and replace all commands with idle if necessary */
      int clearqueue = doc["clear_queue"];
      if (clearqueue==1){
        xQueueReset(commandQueue);
        for (int i=0; i<length; i++){
          commands[i] = IDLE;
        }
      }

      rc = commands;
    
    }
    /* If data has not been received, simply idle */
    else{
      robotCommand commands[1] = {IDLE};
      rc = commands;
    }

    
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

      /* Your Domain name with URL path or IP address with path */
      http.begin(serverPath.c_str());

      /* Send HTTP request */
      httpResponseCode = makeRequest(requestType, http);

      /* Manage response */
      String payload = handleResponse(httpResponseCode, http);

      /* Free resources */
      http.end();

      /* Parse payload string, and convert it to commands */
      numCommands = findCommandLength(payload);
      robotCommand commands[numCommands];
      parsePayload(payload, commands, httpResponseCode);
      
      for (int i=0; i<numCommands; i++){
        newCommand = commands[i];
        xQueueSend(commandQueue, &newCommand, portMAX_DELAY);
      }
    }
  }
}

#endif