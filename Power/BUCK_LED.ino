/*
 * Based upon code written by Yue Zhu (yue.zhu18@imperial.ac.uk) in July 2020.
 * pin6 is PWM output at 62.5kHz.
 * duty-cycle saturation is set as 2% - 98%
 * Control frequency is set as 1kHz. 
*/

#include <Wire.h>

float open_loop, closed_loop,closed_loopdash,closed_loopb=0; // Duty Cycles
float vout,vin,vref=3,vret,iL,dutyref,current_mA; // Measurement Variables
unsigned int sensorValue0,sensorValue1,sensorValue2,sensorValue3;  // ADC sample values declaration
float ev=0,cv=0,ei=0,oc=0; //internal signals
float Ts=0.001; //1 kHz control frequency. It's better to design the control period as integral multiple of switching period.
float kpv=0.05024,kiv=15.78,kdv=0; // voltage pid.
float u0v,u1v,delta_uv,e0v,e1v,e2v; // u->output; e->error; 0->this time; 1->last time; 2->last last time
float kpi=0.02512,kii=39.4,kdi=0; // float kpi=0.02512,kii=39.4,kdi=0; // current pid.
float u0i,u1i,delta_ui,e0i,e1i,e2i; // Internal values for the current controller
float uv_max=4, uv_min=0; //anti-windup limitation
float ui_max=50, ui_min=0; //anti-windup limitation
float current_limit = 200;
unsigned int loopTrigger;
unsigned int com_count=0;   // a variables to count the interrupts. Used for program debugging.

void setup() {

  //Basic pin setups
  



  // TimerA0 initialization for control-loop interrupt.
//  

  // TimerB0 initialization for PWM output
  
  pinMode(0, OUTPUT); 
  pinMode(1, OUTPUT); 
  digitalWrite(1,HIGH);
  analogWriteFreq(100000);
  Serial.begin(115200);   //serial communication enable. Used for program debugging.
  closed_loop=0.4;
}

 void loop() {


    sampling();
 if(iL<current_limit)
 { 
  if(vout<vref)
  {
    closed_loop=closed_loop+0.01;
  }
  else
  {
    closed_loop=closed_loop-0.01;
  }
 }
 else
 {
  closed_loop=closed_loop-0.02;
 }
  closed_loop= saturation(closed_loop,0.99,0.01);
  pwm_modulate(closed_loop);

      Serial.print("Vout: ");
      Serial.print(vout);
      Serial.print("\t");

      Serial.print("Vin: ");
      Serial.print(vin);
      Serial.print("\t");

//      Serial.print("cv: ");
//      Serial.print(cv);
//      Serial.print("\t");
////
//      Serial.print("ei: ");
//      Serial.print(ei);
//      Serial.print("\t");
////
//      Serial.print("vret: ");
//      Serial.print(vret);
//      Serial.print("\t");
//
//       Serial.print("cd: ");
//      Serial.print(closed_loopdash);
//      Serial.print("\t");
//
      Serial.print("closed_loop: ");
      Serial.print(closed_loop);
      Serial.print("\t");
//      
//      Serial.print("closed_loopb: ");
//      Serial.print(closed_loopb);
//      Serial.print("\t");
//      
//      Serial.print("comcout: ");
//      Serial.print(com_count);
//      Serial.print("\t");

      Serial.print("Inductor Current: ");
      Serial.print(iL);
      Serial.print("\n");


//      com_count = 0;
//    }
delay(1);
}


// Timer A CMP1 interrupt. Every 800us the program enters this interrupt. 
// This, clears the incoming interrupt flag and z`triggers the main loop.


// This subroutine processes all of the analogue samples, creating the required values for the main loop

void sampling(){

  // Make the initial sampling operations for the circuit measurements
  
  sensorValue2 = analogRead(28); //sample Vout
  sensorValue0 = analogRead(26); //sample Vref
  sensorValue1 = analogRead(27); //sample Vin


  // Process the values so they are a bit more usable/readable
  // The analogRead process gives a value between 0 and 1023 
  // representing a voltage between 0 and the analogue reference which is 3.3V
  
  vout = sensorValue2 * (12500/2500) * (3.43 / 1023.0); // Convert the Vout sensor reading to volts
  vret = sensorValue0*(3.43/1023.0); // Convert the Vref sensor reading to volts
  vin = sensorValue1 * (12500/2500) *  (3.43 / 1023.0); // Convert the Vin sensor reading to volts

  // The inductor current is in mA from the sensor so we need to convert to amps.
  // We want to treat it as an input current in the Boost, so its also inverted
  // For open loop control the duty cycle reference is calculated from the sensor
  // differently from the Vref, this time scaled between zero and 1.
  // The boost duty cycle needs to be saturated with a 0.33 minimum to prevent high output voltages
  
 
    iL = vret*1000/1.02;
  
}

float saturation( float sat_input, float uplim, float lowlim){ // Saturatio function
  if (sat_input > uplim) sat_input=uplim;
  else if (sat_input < lowlim ) sat_input=lowlim;
  else;
  return sat_input;
}

void pwm_modulate(float pwm_input){ // PWM function
  analogWrite(0,(int)(pwm_input*255)); 
}

// This is a PID controller for the voltage

float pidv( float pid_input){
  float e_integration;
  e0v = pid_input;
  ;
 
  //anti-windup, if last-time pid output reaches the limitation, this time there won't be any intergrations.
  if(u1v >= uv_max) {
    e_integration = 0;
  } else if (u1v <= uv_min) {
    e_integration = 0;
  }

  delta_uv = kpv*(e0v-e1v) + kiv*Ts*e_integration + kdv/Ts*(e0v-2*e1v+e2v); //incremental PID programming avoids integrations.there is another PID program called positional PID.
  u0v = u1v + delta_uv;  //this time's control output

  //output limitation
  saturation(u0v,uv_max,uv_min);
  
  u1v = u0v; //update last time's control output
  e2v = e1v; //update last last time's error
  e1v = e0v; // update last time's error
  return u0v;
}

// This is a PID controller for the current

float pidi(float pid_input){
  float e_integration;
  e0i = pid_input;
  e_integration=e0i;
  
  //anti-windup
  if(u1i >= ui_max){
    e_integration = 0;
  } else if (u1i <= ui_min) {
    e_integration = 0;
  }
  
  delta_ui = kpi*(e0i-e1i) + kii*Ts*e_integration + kdi/Ts*(e0i-2*e1i+e2i); //incremental PID programming avoids integrations.
  u0i = u1i + delta_ui;  //this time's control output

  //output limitation
  saturation(u0i,ui_max,ui_min);
  
  u1i = u0i; //update last time's control output
  e2i = e1i; //update last last time's error
  e1i = e0i; // update last time's error
  return u0i;
}


/*end of the program.*/
