#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
/*
Authors: Aranya Gupta
Date created: 05/05/23
Date updated: 22/06/23

Communicate with a server using JSONs
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

/* Arduino headers */
#include "WiFi.h"
#include "HTTPClient.h"
#include <ArduinoJson.h>

/* Personal libraries */
#include "src/FIRFilter.h"

//-------------------------------- Global Variables -------------------------------------

const char* ssid = "osa";
const char* password = "password";

String serverName = "http://54.165.59.37:5000/";
String hostname = "ESP32 Node";

TaskHandle_t taskServerCommunicationHandle = nullptr;

/* Create battery FIR filter */
static FIRFilter2 batteryFIR;

static struct ToFDistanceData ToFData;
static struct angleData IMUData;

float ang;
volatile bool wifiInitialised = false;

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
    xQueuePeek(ToFDataQueue, &ToFData, 0);
    xQueuePeek(IMUDataQueue, &IMUData, 0);
    String timestamp = String(millis());
    String position_x = String(xPosition);
    String position_y = String(yPosition);
    String orientation = String(IMUData.yaw);
    String tofleft = String(ToFData.left);
    String tofright = String(ToFData.right);
    String mac = String(WiFi.macAddress());
    String rssi = String(WiFi.RSSI());
    String battery = String(FIRFilterUpdate2(&batteryFIR, analogRead(VBAT) * 4 * 3.3 * 1.1 / 4096));
    String cwa = String(currentWhereAt);
    String postData = "{\"diagnostics\": {\"battery\":" + battery + ",\"connection\":" + rssi + "},\"MAC\":\"" + mac + "\",\"nickname\":\"MiWhip\",\"timestamp\":" + timestamp + ",\"position\":[" + position_x + "," + position_y + "],\"whereat\":" + cwa + ",\"orientation\":" + orientation + ",\"branches\":[";

    float junctionAngle;
    float beaconAngle;
    bool flag = false;


    /* If the SPIN task isn't running process the results */
    if (xSemaphoreTake(mutexSpin, 0) == pdTRUE) {

      /* While there are junction angles waiting in the queue, send them to the server */
      while (uxQueueMessagesWaiting(junctionAngleQueue) > 0) {
        if (xQueueReceive(junctionAngleQueue, &junctionAngle, 0) == pdTRUE) {
          postData = postData + String(junctionAngle) + ",";
          flag = true;
        }
      }

      if (flag) {
        postData = postData.substring(0, postData.length() - 1);
      }
      postData = postData + "],\"beaconangles\":[";
      flag = false;
      // SERIAL_PORT.println("I'm outside");
      // SERIAL_PORT.println(uxQueueMessagesWaiting(beaconAngleQueue));
      // SERIAL_PORT.println(uxQueueSpacesAvailable(beaconAngleQueue));

      /* If there are 3 beacon angles waiting in the queue, send them to the server */
      if (uxQueueMessagesWaiting(beaconAngleQueue) == NUMBER_OF_BEACONS) {
        while (uxQueueMessagesWaiting(beaconAngleQueue) > 0) {
          if (xQueueReceive(beaconAngleQueue, &beaconAngle, 0) == pdTRUE) {
            postData = postData + String(beaconAngle) + ",";
            flag = true;
            SERIAL_PORT.println(beaconAngle);
          }
        }
      }

      xSemaphoreGive(mutexSpin);
    } 
    /* If SPIN task running, send empty JSON keys#[] */
    else {

      postData = postData + "],\"beaconangles\":[";
      flag = false;
    }

    if (flag) {
      postData = postData.substring(0, postData.length() - 1);
    }
    postData = postData + "],\"tofleft\":" + tofleft + ",\"tofright\":" + tofright + "}";
    return http.POST(postData);
  }
}


String handleResponse(uint16_t httpResponseCode, HTTPClient& http) {

  if (httpResponseCode >= 200 && httpResponseCode < 300) {
#if ENABLE_SERVER_COMMUNICATION_DEBUG == true
    SERIAL_PORT.println(httpResponseCode);
#endif
  } else if (httpResponseCode >= 400 && httpResponseCode < 500) {
#if ENABLE_SERVER_COMMUNICATION_DEBUG == true
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
#endif
  } else if (httpResponseCode >= 500 && httpResponseCode < 600) {
#if ENABLE_SERVER_COMMUNICATION_DEBUG == true
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
#endif
  } else {
#if ENABLE_SERVER_COMMUNICATION_DEBUG == true
    SERIAL_PORT.print("Error code: ");
    SERIAL_PORT.println(httpResponseCode);
#endif
    return "{\"error\": \"Server not up\"}";
  }

  return http.getString();
}
/* Get size of response sent */
int findCommandLength(String payload) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  return doc["next_actions"].size();
}

/* Convert JSON to array of commands */
/* Convert JSON to array of commands */
void parsePayload(String payload, robotCommand rc[], uint16_t httpResponseCode, uint8_t& commandsLength) {
  /* Check to ensure data has been received */
  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    /* Extract JSON information */
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    /*  Get number of commands and create array with that size */
    float actions[commandsLength];

    /* 
        Can't directly use next_actions array as it is a String& instead 
        of a float[], but next_actions[i] is a float (???), so we iterate 
        and store in a new array
      */

    for (int i = 0; i < commandsLength; i++) {
      actions[i] = doc["next_actions"][i];
    }

    /* Convert next actions to robot commands */
    robotCommand commands[commandsLength];
    for (int i = 0; i < commandsLength; i++) {
      commands[i] = IDLE;
    }

    for (int i = 0; i < commandsLength; i++) {
      int a = static_cast<int>(actions[i]);
      switch (a) {
        case 0:
          commands[i] = FORWARD;
          break;
        case 1:
          commands[i] = SPIN;
          break;
        case 2:
          commands[i] = TURN;
          ang = actions[i + 1];
          xQueueSend(angleSetpointQueue, &ang, portMAX_DELAY);
          i++;
          break;
        case 3:
          commands[i] = IDLE;
          break;
        case 4:
          {
            xPosition = actions[i + 1];
            yPosition = actions[i + 2];
            i += 2;
            break;
          }
        case 5:
          {
            commands[i] = IDLE;
            break;
          }
        default:
          break;
      }
    }
    /* Clear queue and replace all commands with idle if necessary */
    int clearqueue = doc["clear_queue"];
    if (clearqueue == 1) {
      xQueueReset(commandQueue);
      xQueueReset(angleSetpointQueue);
      for (int i = 0; i < commandsLength; i++) {
        commands[i] = IDLE;
      }
    }
    for (int i = 0; i < commandsLength; i++) {
      rc[i] = commands[i];
    }
  }
  /* If data has not been received, continue as before */
  else {
    commandsLength = 0;
  }
}



//-------------------------------- Task Functions ---------------------------------------

/* Task to communicate with server via JSON */
void taskServerCommunication(void* pvParameters) {

  (void)pvParameters;

  static uint16_t requestType;
  static uint16_t httpResponseCode;
  static uint8_t numCommands;
  static robotCommand newCommand;

  /* Initialise battery filter */
  FIRFilterInit2(&batteryFIR);
  
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
      if (numCommands != 0) {
        robotCommand commands[numCommands];
        parsePayload(payload, commands, httpResponseCode, numCommands);

        for (int i = 0; i < numCommands; i++) {

          /* Flash LED when a command is received */
          // digitalWrite(LED_BUILTIN, LOW);
          vTaskDelay(pdMS_TO_TICKS(10));

          /* Send new command to the command queue*/
          newCommand = commands[i];
          xQueueSend(commandQueue, &newCommand, portMAX_DELAY);

          /* Flash LED when a command is received */
          // digitalWrite(LED_BUILTIN, HIGH);
          vTaskDelay(pdMS_TO_TICKS(10));
          // digitalWrite(LED_BUILTIN, LOW);
        }
      }
    }
  }
}