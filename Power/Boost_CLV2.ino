/*
 * Based upon code written by Yue Zhu (yue.zhu18@imperial.ac.uk) in July 2020.
 * pin6 is PWM output at 62.5kHz.
 * duty-cycle saturation is set as 2% - 98%
 * Control frequency is set as 1kHz. 
*/

#include <Wire.h>
#include <INA219_WE.h>

INA219_WE ina219; // this is the instantiation of the library for the current sensor

float open_loop, closed_loop; // Duty Cycles
float va,vb,vref,iL,dutyref,current_mA,dv,di,dp,current_mA_ap; // Measurement Variables
unsigned int sensorValue0,sensorValue1,sensorValue2,sensorValue3;  // ADC sample values declaration
float ev=0,cv=0,ei=0,oc=0; //internal signals
float Ts=0.001; //1 kHz control frequency. It's better to design the control period as integral multiple of switching period.
float kpv=0.05024,kiv=15.78,kdv=0; // voltage pid.
float u0v,u1v,delta_uv,e0v,e1v,e2v; // u->output; e->error; 0->this time; 1->last time; 2->last last time
float kpi=0.02512,kii=39.4,kdi=0; // float kpi=0.02512,kii=39.4,kdi=0; // current pid.
float u0i,u1i,delta_ui,e0i,e1i,e2i; // Internal values for the current controller
float uv_max=4, uv_min=0; //anti-windup limitation
float ui_max=50, ui_min=0; //anti-windup limitation
float current_limit = 1.0;
float vb_10,vbp=0,ibp=0;
float p,delta=0.005,delta_min=0.005,pp=0;
boolean Boost_mode = 0;
boolean CL_mode = 0;
unsigned int loopTrigger;
unsigned int com_count=0;   // a variables to count the interrupts. Used for program debugging.

void setup() {

  //Basic pin setups
  
  noInterrupts(); //disable all interrupts
  pinMode(13, OUTPUT);  //Pin13 is used to time the loops of the controller
  pinMode(3, INPUT_PULLUP); //Pin3 is the input from the Buck/Boost switch
  pinMode(2, INPUT_PULLUP); // Pin 2 is the input from the CL/OL switch
  analogReference(EXTERNAL); // We are using an external analogue reference for the ADC

  // TimerA0 initialization for control-loop interrupt.
  
  TCA0.SINGLE.PER = 999; //
  TCA0.SINGLE.CMP1 = 999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm; 

  // TimerB0 initialization for PWM output
  
  pinMode(6, OUTPUT);
  TCB0.CTRLA=TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz
  analogWrite(6,120); 

  Serial.begin(115200);   //serial communication enable. Used for program debugging.
  interrupts();  //enable interrupts.
  Wire.begin(); // We need this for the i2c comms for the current sensor
  ina219.init(); // this initiates the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  closed_loop=0.5;
  
}

 void loop() {
  if(loopTrigger) { // This loop is triggered, it wont run unless there is an interrupt
    
    digitalWrite(13, HIGH);   // set pin 13. Pin13 shows the time consumed by each control cycle. It's used for debugging.
    
    // Sample all of the measurements and check which control mode we are in
    sampling();
    CL_mode = digitalRead(3); // input from the OL_CL switch
    Boost_mode = digitalRead(2); // input from the Buck_Boost switch
    p=vb*current_mA_ap;
    if (Boost_mode){
      if (CL_mode) { //Closed Loop Boost
//P&O algorighm
      if (current_mA_ap==0)
      {
        current_mA_ap=ibp; //prevent not sensing current properly 
      }
//obtain difference and power
      p=vb*(current_mA_ap);
      dv=vb_10-vbp;
      di=ibp;
      dp=p-pp;
//varying dutycycle if the following condition meet
       if(dp>0)
      {
  
        if(dv>0)
        {
          closed_loop=closed_loop-delta; //at LHS of MPP, decrese dutycycle to increase voltage
        }
        else
        {
          closed_loop=closed_loop+delta;//at RHS of MPP
        }
      }
      else
      {
        if(dv>0)
        {
          closed_loop=closed_loop+delta; //at RHS of MPP
        }
        else
        {
          closed_loop=closed_loop-delta; //at LHS of MPP
        }
      }
//store the current value as the pervious value of next cycle
ibp=current_mA_ap;
vbp=vb_10;
pp=p;
closed_loop=saturation(closed_loop,0.99,0.02);
pwm_modulate(closed_loop);

}
      
     else{ // Open Loop Boost
          current_limit = 2; // 
          oc = iL-current_limit; // Calculate the difference between current measurement and current limit
          if ( oc > 0) {
            open_loop=open_loop+0.001; // We are above the current limit so less duty cycle
          } else {
            open_loop=open_loop-0.001; // We are below the current limit so more duty cycle
          }
          open_loop=saturation(open_loop,0.99,dutyref); // saturate the duty cycle at the reference or a min of 0.01
          pwm_modulate(open_loop); // and send it out
      }
    }else{      
      if (CL_mode) { // Closed Loop Buck
          pwm_modulate(1); // This disables the Buck as we are not using this mode
      }else{ // Open Loop Buck
          pwm_modulate(1); // This disables the Buck as we are not using this mode
      }
    }

    com_count++;              //used for debugging.
//    if (com_count >= 1000) {  //send out data every second.
      Serial.print("Va: ");
      Serial.print(va);
      Serial.print("\t");

      Serial.print("Vb: ");
      Serial.print(vb);
      Serial.print("\t");

      Serial.print("p: ");
      Serial.print(p);
      Serial.print("\t\t");

      Serial.print("CLduty: ");
      Serial.print(closed_loop);
      Serial.print("\t\t");

      Serial.print("current: ");
      Serial.print(current_mA_ap);
      Serial.print("\t\t");

      Serial.print("OLduty: ");
      Serial.print(1-open_loop);
      Serial.print("\t\t");


      Serial.print("CL Mode: ");
      Serial.print(CL_mode);
      Serial.print("\n");
      com_count = 0;
//    }

    digitalWrite(13, LOW);   // reset pin13.
    loopTrigger = 0;
  }
}


// Timer A CMP1 interrupt. Every 800us the program enters this interrupt. 
// This, clears the incoming interrupt flag and triggers the main loop.

ISR(TCA0_CMP1_vect){
  TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_CMP1_bm; //clear interrupt flag
  loopTrigger = 1;
}

// This subroutine processes all of the analogue samples, creating the required values for the main loop

void sampling(){

  // Make the initial sampling operations for the circuit measurements
  
  sensorValue0 = analogRead(A0); //sample Vb
  sensorValue2 = analogRead(A2); //sample Vref
  sensorValue3 = analogRead(A3); //sample Va
  current_mA = ina219.getCurrent_mA(); // sample the inductor current (via the sensor chip)

  // Process the values so they are a bit more usable/readable
  // The analogRead process gives a value between 0 and 1023 
  // representing a voltage between 0 and the analogue reference which is 4.096V
  
  vb = sensorValue0 * (12400/2400) * (4.096 / 1023.0); // Convert the Vb sensor reading to volts
  vref = sensorValue2 * (4.096 / 1023.0); // Convert the Vref sensor reading to volts
  va = sensorValue3 * (12400/2400) * (4.096 / 1023.0); // Convert the Va sensor reading to volts

  // The inductor current is in mA from the sensor so we need to convert to amps.
  // We want to treat it as an input current in the Boost, so its also inverted
  // For open loop control the duty cycle reference is calculated from the sensor
  // differently from the Vref, this time scaled between zero and 1.
  // The boost duty cycle needs to be saturated with a 0.33 minimum to prevent high output voltages
  
  if (Boost_mode == 1){
    iL = -current_mA/1000.0;
    vb_10=vb*10;// gives more digits of voltage to make the result more accurate
    current_mA_ap=round(-current_mA); //round the sensing current to avoid oversenstive.
    dutyref = saturation(sensorValue2 * (1.0 / 1023.0),0.99,0.2);
  }else{
    iL = current_mA/1000.0;
    dutyref = sensorValue2 * (1.0 / 1023.0);
  }
  
}

float saturation( float sat_input, float uplim, float lowlim){ // Saturatio function
  if (sat_input > uplim) sat_input=uplim;
  else if (sat_input < lowlim ) sat_input=lowlim;
  else;
  return sat_input;
}

void pwm_modulate(float pwm_input){ // PWM function

  if(CL_mode)
  {
     analogWrite(6,(int)(pwm_input*255)); 
  }
  else
  {
  analogWrite(6,(int)(255-pwm_input*255)); 
  }
}


/*end of the program.*/
