#include <PID_v1.h>

#define STEP_PIN 18
#define DIR_PIN 19
#define STEPS 200
#define MIN_RPM 10
#define MAX_RPM 1000 // MOTOR CONFIG END

#include "ICM_20948.h" //START IMU CONFIG
#define WIRE_PORT Wire
#define AD0_VAL 1
#define K_P 0.3
#define K_I 0.0
#define K_D 0.0
ICM_20948_I2C myICM; // END IMU CONFIG
              
hw_timer_t *motor_timer = NULL;

unsigned long prevStepMillis = 0;
double pid_out;
double setpoint = 0;
double pitch;
static const uint16_t controllerFrequency = 100;
PID control(&pitch,&pid_out,&setpoint,K_P,K_I,K_D,DIRECT);

void IRAM_ATTR onTimer(){
  digitalWrite(STEP_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);
}


void setup() {
  motor_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(motor_timer, &onTimer, true);
  pinMode(STEP_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);
  control.SetMode(AUTOMATIC);
  control.SetOutputLimits(-256, 256);
  Serial.begin(9600);
  Serial.println("Meep morp robot online");

  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000);
   bool initialized = false;
  while (!initialized)
  {

    // Initialize the ICM-20948
    // If the DMP is enabled, .begin performs a minimal startup. We need to configure the sample mode etc. manually.
    myICM.begin(WIRE_PORT, AD0_VAL);

    if (myICM.status != ICM_20948_Stat_Ok)
    {
      delay(500);
      Serial.println("IMU initialising...");
    }
    else
    {
      initialized = true;
      Serial.println("IMU initialised");
    }
  }
  bool success = true;
  success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR) == ICM_20948_Stat_Ok);
  success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat6, 0) == ICM_20948_Stat_Ok); // Set to the maximum
  success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);
  if (success)
    {
      Serial.println("DMP initialised");
    }
    else
    {
      Serial.println(F("Enable DMP failed!"));
      Serial.println(F("Please check that you have uncommented line 29 (#define ICM_20948_USE_DMP) in ICM_20948_C.h..."));
      while (1)
        ; // Do nothing more
    }

}

void motor_start(double RPM) {
    double millisBetweenSteps = 60000/(STEPS*abs(RPM)); // milliseconds
    if(RPM > 0){
      digitalWrite(DIR_PIN,HIGH);
    }else if(RPM<0){
      digitalWrite(DIR_PIN,LOW);
    }else{
      return;
    }
    timerAlarmWrite(motor_timer, millisBetweenSteps*1000, true);
    timerAlarmEnable(motor_timer);
}


void loop() {
  
  icm_20948_DMP_data_t data; //IMU START
  myICM.readDMPdataFromFIFO(&data);

  if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) // Was valid data available?
  {
    if ((data.header & DMP_header_bitmap_Quat6) > 0)
    {
      double q1 = ((double)data.Quat6.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
      double q2 = ((double)data.Quat6.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
      double q3 = ((double)data.Quat6.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30
      double q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
      double q2sqr = q2 * q2;
      // roll (x-axis rotation)
      double t0 = +2.0 * (q0 * q1 + q2 * q3);
      double t1 = +1.0 - 2.0 * (q1 * q1 + q2sqr);
      double roll = atan2(t0, t1) * 180.0 / PI;
      // pitch (y-axis rotation)
      double t2 = +2.0 * (q0 * q2 - q3 * q1);
      t2 = t2 > 1.0 ? 1.0 : t2;
      t2 = t2 < -1.0 ? -1.0 : t2;
      pitch = asin(t2) * 180.0 / PI;
      // yaw (z-axis rotation)
      double t3 = +2.0 * (q0 * q3 + q1 * q2);
      double t4 = +1.0 - 2.0 * (q2sqr + q3 * q3);
      double yaw = atan2(t3, t4) * 180.0 / PI;
    }
  }

  if (myICM.status != ICM_20948_Stat_FIFOMoreDataAvail) // If more data is available then we should read it right away - and not delay
  {
    delay(10);
  }//IMU END

  

  control.Compute();
  Serial.print("pid_out: ");
  Serial.println(pid_out);  
  double motorspeed = (pid_out/255)*(MAX_RPM);
  if(abs(motorspeed)>MIN_RPM){
    motor_start(-motorspeed);
  }else{
    motor_start(0);
  }
  


  
  

}
