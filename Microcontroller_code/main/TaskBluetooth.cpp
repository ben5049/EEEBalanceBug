/*
Authors: David Cai
Date created: 21/05/23
Date updated: 21/06/23

Bluetooth Xbox controller task
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"
#include "BluetoothSerial.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"
BluetoothSerial SerialBT;

//-------------------------------- Global Variables -------------------------------------

TaskHandle_t taskBluetoothHandle = nullptr;

//-------------------------------- Task Functions ---------------------------------------

void initBluetooth(){
  SerialBT.begin("dwayne"); //Bluetooth device name
  SerialBT.setTimeout(10);
}

/* Task to record debug information */
void taskBluetooth(void *pvParameters) {

  (void)pvParameters;

  /* Start the loop */
  while (true) {
    static char serialData; 
    digitalWrite(LED_BUILTIN,HIGH);
    serialData = SerialBT.read();
    enableDirectionControl=false;
    enableAngRateControl=true;
    if(SerialBT.available()){
        if (serialData == 'x') {
          speedSetpoint = SerialBT.parseFloat();
        } else if (serialData == 'y') {
          angRateSetpoint = SerialBT.parseFloat();
        }
        SerialBT.flush();
      }else{
        speedSetpoint = 0;
        angRateSetpoint = 0;
      }
    vTaskDelay(100);
  }
}