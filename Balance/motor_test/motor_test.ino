#include <PID_v1.h>

#define STEP_PIN 16
#define DIR_PIN 17
#define STEPS 1600
#define MIN_RPM 10
#define MAX_RPM 1000 // MOTOR CONFIG END

#include "ICM_20948.h" //START IMU CONFIG
#define SPI_PORT SPI     /* Desired SPI port */
#define SPI_FREQ 5000000 /* Override the default SPI frequency */
#define IMU_INT 4
#define IMU_MISO 19
#define IMU_MOSI 23
#define IMU_CS 5
#define IMU_SCK 18

#define KP_Pitch 1.5
#define KI_Pitch 0.5
#define KD_Pitch 0.0015

#define KP_Speed 1.5 
#define KI_Speed 0.5
#define KD_Speed 0.0015

ICM_20948_SPI myICM; // END IMU CONFIG
              
hw_timer_t *motor_timer = NULL;

double pitch_out;
double angle_setpoint = 0.8;
double pitch;
double x_speed;
double speed_out;
double speed_setpoint = 0;


PID pitch_control(&pitch,&pitch_out,&angle_setpoint,KP_Pitch,KI_Pitch,KD_Pitch,DIRECT);
//PID speed_control(&x_speed,&speed_out,&speed_setpoint,KP_Speed,KI_Speed,KD_Speed,DIRECT);

void IRAM_ATTR onTimer(){
  digitalWrite(STEP_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);
}


void setup() {
  motor_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(motor_timer, &onTimer, true);
  pinMode(STEP_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);
  pitch_control.SetMode(AUTOMATIC);
  pitch_control.SetOutputLimits(-256, 256);
  pitch_control.SetSampleTime(10);
  Serial.begin(9600);
  Serial.println("Meep morp robot online");

  SPI_PORT.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_INT);
  
   bool initialized = false;
  while (!initialized)
  {

    // Initialize the ICM-20948
    // If the DMP is enabled, .begin performs a minimal startup. We need to configure the sample mode etc. manually.
    myICM.begin(IMU_CS, SPI_PORT, SPI_FREQ);

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
  ICM_20948_AGMT_t agmt;
  icm_20948_DMP_data_t angle_data; //IMU START
  myICM.readDMPdataFromFIFO(&angle_data);
  myICM.getAGMT();

  float acc_x = myICM.accX();
  float acc_y = myICM.accY();
  float acc_z = myICM.accZ();

  if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) // Was valid data available?
  {
    if ((angle_data.header & DMP_header_bitmap_Quat6) > 0)
    {
      double q1 = ((double)angle_data.Quat6.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
      double q2 = ((double)angle_data.Quat6.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
      double q3 = ((double)angle_data.Quat6.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30
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

  
  
  pitch_control.Compute();
  //Serial.print("pid_out: ");
  //Serial.println(pid_out);  
  //Serial.println(pitch);
  // Serial.print(pitch);
  // Serial.print(",");
  // Serial.println(pitch_out);
  double pitch_contribution = (pitch_out/255)*(MAX_RPM);
  //double speed_contribution = (speed_out/255)*(MAX_RPM);
  motor_start(-(pitch_contribution));
  


  
  

}
