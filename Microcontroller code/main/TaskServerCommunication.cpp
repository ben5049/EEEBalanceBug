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
#include "Arduino_JSON.h"

//-------------------------------- Global Variables -------------------------------------

const char* ssid = "";
const char* password = "";

String serverName = "http://3.86.226.249:5000/";
String hostname = "ESP32 Node";

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

  delay(1000);
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
    String tofleft = String(distanceLeft);
    String tofright = String(distanceRight);
    String mac = String(WiFi.macAddress());
    String rssi = String(WiFi.RSSI());
    String postData = "{\"diagnostics\": {\"battery\":100,\"connection\":"+rssi+"},\"MAC\":"+mac+",\"nickname\":\"MiWhip\",\"timestamp\":"+timestamp+",\"position\":["+position_x+","+position_y+"],\"whereat\":0,\"orientation\":"+orientation+",\"branches\":[],\"beaconangles\":[],\"tofleft\":"+tofleft+",\"tofright\":"+tofright+"}";
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
    return "{\"error\": \"Something really wrong...\"}";
  }
  return http.getString();
}

robotCommand* parsePayload(String payload, int& numCommands) {
  JSONVar data = JSON.parse(payload);
  robotCommand commands[10];
  numCommands = 0;

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(data) == "undefined") {
    SERIAL_PORT.println("Parsing input failed!");
    return commands;
  }

  SERIAL_PORT.print("JSON object = ");
  SERIAL_PORT.println(data);
  String actions[] = data["next_actions"];
  String clear  = data["clear_queue"];
  if (clear=="true"){
    xQueueReset(commandQueue);
    numCommands = 0;
  }
  else{
    for (int i=0; i<actions.size(); i++){
      switch(actions[i].substring(0, 2)){
        case "st":
          float dist = (float) actions[i].substring(5);
          SERIAL_PORT.println(dist);
          commands[i] = FORWARD;
          break;
        case "sp":
          SERIAL_PORT.println("spin");
          commands[i] = SPIN;
          break
        case "an":
          float ang = (float) actions[i].substring(6);
          SERIAL_PORT.println(ang);
          commands[i] = TURN;
          break;
        case "id":
          SERIAL_PORT.println("idle");
          commands[i] = IDLE;
          break;
        case "up":
          String newpos = actions[i].substring(16);
          int spaceindex = newpos.indexOf(" ");
          float newx = (float) newpos.substring(0, spaceindex);
          float newy = (float) newpos.substring(spaceindex+1);
          SERIAL_PORT.print(newx);
          SERIAL_PORT.println(newy);
          xPos = newx;
          yPos = newy;
          break;
        default:
          SERIAL_PORT.println("idle");
          commands[i] = IDLE;
      }
      numCommands = actions.size();
    }
  
  return commands;
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

      // Your Domain name with URL path or IP address with path
      http.begin(serverPath.c_str());

      // Send HTTP request
      httpResponseCode = makeRequest(requestType, http);

      // Manage response
      String payload = handleResponse(httpResponseCode, http);

      // Free resources
      http.end();

      // parse payload string
      robotCommand commands[] = parsePayload(payload, numCommands);
      for (int i=0; i<numCommands; i++){
        newCommand = commands[i];
        xQueueSend(commandQueue, newCommand, portMAX_DELAY);
      }
    }
  }
}

#endif