#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const char* ssid = "";
const char* password = "";

String serverName = "http://13.41.200.80:3001/";

String hostname = "ESP32 Node";

void setup() {
  Serial.begin(115200);
 

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  delay(1000);
 
}

void setEndpointAndRequest(String& endpoint, int& requestType){
  // endpoint = "tableData33";
  // requestType = 0;
  // endpoint = "pollServer";
  // requestType = 0;
  endpoint = "personQuery";
  requestType = 0;
  // endpoint = "TestPOST";
  // requestType = 1; // 0 is get 1 is post
}

int makeRequest(int requestType, HTTPClient& http){
  if (requestType == 0){
      return http.GET();
  }
  else if (requestType == 1){
    http.addHeader("Content-Type", "application/json");
    String postData = "{\"hello\":\"advik\"}";
    return http.POST(postData);
  }
}

String handleResponse(int httpResponseCode, HTTPClient& http){
  if (httpResponseCode>=200 && httpResponseCode<300) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      return http.getString();
  }
  else if (httpResponseCode >= 400 && httpResponseCode < 500){
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      return "{\"error\": \"Invalid Request\"}";
  }
  else if (httpResponseCode >= 500){
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    return "{\"error\": \"Internal Server Error\"}";
  }
  else{
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    return "{\"error\": \"Something really wrong...\"}";
  }
}

void parsePayload(String payload){
  JSONVar data = JSON.parse(payload);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(data) == "undefined") {
    Serial.println("Parsing input failed!");
    delay(1000);
    return;
  }

  Serial.print("JSON object = ");
  Serial.println(data);

  JSONVar keys = data.keys();

  for (int i = 0; i < keys.length(); i++) {
    JSONVar value = data[keys[i]];
    Serial.print(keys[i]);
    Serial.print(" = ");
    Serial.println(value);
  }
}

void loop() {
  String endpoint;
  int requestType;
  setEndpointAndRequest(endpoint, requestType);
  String serverPath = serverName + endpoint;
  
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP request
    int httpResponseCode = makeRequest(requestType, http);

    // Manage response
    String payload = handleResponse(httpResponseCode, http); 

    // Free resources
    http.end();

    // parse payload string
    parsePayload(payload);

    delay(5000);
  }

}
