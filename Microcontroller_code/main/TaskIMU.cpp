/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 06/06/23

IMU sampling and sensor fusion
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

/* Third party libraries */
#include "ICM_20948.h"
#include "Src/Fusion.h"

//-------------------------------- Global Variables -------------------------------------

/* IMU */
static ICM_20948_SPI myICM; /* Create an ICM_20948_SPI object */

/* IMU DMP */
volatile float pitch;
volatile float pitchRate;
volatile float yaw;
volatile float yawRate;

static float frequencyIMU;

/* Task handles */
TaskHandle_t taskIMUHandle = nullptr;

struct angleData IMUData;

//-------------------------------- Functions --------------------------------------------

/* Configure the IMU */
void configureIMU() {
  static bool initialized = false;
  while (!initialized) {
    myICM.begin(IMU_CS, SPI_PORT, SPI_FREQ);

    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    SERIAL_PORT.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok) {
      SERIAL_PORT.println("Trying again...");
      delay(500);
    } else {
      initialized = true;
      SERIAL_PORT.println("IMU connected");
    }
  }

  /* Reset and wake up */
  myICM.swReset();
  if (myICM.status != ICM_20948_Stat_Ok) {
    SERIAL_PORT.print(F("Software Reset returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

  delay(250);
  myICM.sleep(false);
  myICM.lowPower(false);

/* Initialise DMP if enabled*/
#if ENABLE_DMP == true

  frequencyIMU = IMU_SAMPLING_FREQUENCY_DMP;

  static bool success = true;

  success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);

#if ENABLE_DMP_MAGNETOMETER == true
  success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok);
  success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) == ICM_20948_Stat_Ok);  // Set to the maximum
#else
  success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR) == ICM_20948_Stat_Ok);
  success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat6, 0) == ICM_20948_Stat_Ok);                        // Set to the maximum
#endif
  success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);

  if (success) {
    SERIAL_PORT.println("DMP initialised");
  } else {
    while (true) {
      SERIAL_PORT.println(F("Enable DMP failed!"));
      SERIAL_PORT.println(F("Please check that you have uncommented line 29 (#define ICM_20948_USE_DMP) in ICM_20948_C.h. (This line is re-commented out each time you update the library)"));
      delay(1000);
    }
  }

/* If DMP is not initialised run the IMU faster */
#else

  frequencyIMU = IMU_SAMPLING_FREQUENCY_NO_DMP;

  /* Set to continuous sample mode */
  myICM.setSampleMode((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Continuous);
  if (myICM.status != ICM_20948_Stat_Ok) {
    SERIAL_PORT.print(F("setSampleMode returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

  /* Set the sample rate */
  ICM_20948_smplrt_t mySmplrt;
  mySmplrt.g = 1; /* 1.1 kHz/(1+GYRO_SMPLRT_DIV[7:0]), default = 19 */
  mySmplrt.a = 1; /* 1.125 kHz/(1+ACCEL_SMPLRT_DIV[11:0]), default = 19 */
  myICM.setSampleRate(ICM_20948_Internal_Gyr, mySmplrt);
  SERIAL_PORT.print(F("setSampleRate returned: "));
  SERIAL_PORT.println(myICM.statusString());

  // Set full scale ranges for both acc and gyr
  ICM_20948_fss_t myFSS;  // This uses a "Full Scale Settings" structure that can contain values for all configurable sensors

  myFSS.a = gpm4;  // (ICM_20948_ACCEL_CONFIG_FS_SEL_e)
                   // gpm2
                   // gpm4
                   // gpm8
                   // gpm16

  myFSS.g = dps500;  // (ICM_20948_GYRO_CONFIG_1_FS_SEL_e)
                     // dps250
                     // dps500
                     // dps1000
                     // dps2000

  myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);
  if (myICM.status != ICM_20948_Stat_Ok) {
    SERIAL_PORT.print(F("setFullScale returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

  // Set up Digital Low-Pass Filter configuration
  ICM_20948_dlpcfg_t myDLPcfg;     // Similar to FSS, this uses a configuration structure for the desired sensors
  myDLPcfg.a = acc_d473bw_n499bw;  // (ICM_20948_ACCEL_CONFIG_DLPCFG_e)
                                   // acc_d246bw_n265bw      - means 3db bandwidth is 246 hz and nyquist bandwidth is 265 hz
                                   // acc_d111bw4_n136bw
                                   // acc_d50bw4_n68bw8
                                   // acc_d23bw9_n34bw4
                                   // acc_d11bw5_n17bw
                                   // acc_d5bw7_n8bw3        - means 3 db bandwidth is 5.7 hz and nyquist bandwidth is 8.3 hz
                                   // acc_d473bw_n499bw

  myDLPcfg.g = gyr_d361bw4_n376bw5;  // (ICM_20948_GYRO_CONFIG_1_DLPCFG_e)
                                     // gyr_d196bw6_n229bw8
                                     // gyr_d151bw8_n187bw6
                                     // gyr_d119bw5_n154bw3
                                     // gyr_d51bw2_n73bw3
                                     // gyr_d23bw9_n35bw9
                                     // gyr_d11bw6_n17bw8
                                     // gyr_d5bw7_n8bw9
                                     // gyr_d361bw4_n376bw5

  myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg);
  if (myICM.status != ICM_20948_Stat_Ok) {
    SERIAL_PORT.print(F("setDLPcfg returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

#endif

#if ENABLE_MAGNETOMETER == true
  /* Start the magnetometer */
  myICM.startupMagnetometer();
  if (myICM.status != ICM_20948_Stat_Ok) {
    SERIAL_PORT.print(F("startupMagnetometer returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }
#endif

  /* Enable data ready interrupt */
  myICM.cfgIntActiveLow(true);
  myICM.cfgIntOpenDrain(false);
  myICM.cfgIntLatch(false);
  myICM.intEnableRawDataReady(true);
}

//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on IMU data ready interrupt and unblocks the IMU sampling task */
void IRAM_ATTR IMUDataReadyISR() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveIndexedFromISR(taskIMUHandle, 0, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to sample the IMU and process data */
void taskIMU(void *pvParameters) {

  (void)pvParameters;

  SERIAL_PORT.println("pog"); /* !!DO NOT REMOVE!! FOR SOME GODFORSAKEN REASON REMOVING THIS LINE CAUSES THE MCU TO BOOT LOOP */

  /* Create object to store DMP data */
  static icm_20948_DMP_data_t angle_data;

  /* This algorithm is based on the revised AHRS algorithm presented in chapter 7 of
    Madgwick's PhD thesis: https://ethos.bl.uk/OrderDetails.do?uin=uk.bl.ethos.681552

    The source code can be found in: https://github.com/xioTechnologies/Fusion */

  /* Initialise algorithms */
  FusionOffset offset;
  FusionAhrs ahrs;

  FusionOffsetInitialise(&offset, frequencyIMU);
  FusionAhrsInitialise(&ahrs);

  /* Set AHRS algorithm settings */
  const FusionAhrsSettings settings = {
    .convention = FusionConventionNwu,
    .gain = 0.5f,
    .accelerationRejection = 10.0f,
    .magneticRejection = 20.0f,
    .rejectionTimeout = 5 * frequencyIMU,
  };

  FusionAhrsSetSettings(&ahrs, &settings);

  /* Create timing variables */
  static unsigned long timestamp;         /* Timestamp of samples taken in microseconds */
  static unsigned long previousTimestamp; /* Time since last sample in microseconds */
  static float deltaTime;                 /* Time between samples in seconds */

  static uint32_t samples = 0;
  static float yawDrift = 0;
  static float totalYawDrift = 0;
  static float prevYaw;

  const TickType_t xFrequency = configTICK_RATE_HZ / IMU_SAMPLING_FREQUENCY_DMP;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* Wait for the data ready interrupt before sampling the IMU */
    // ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    /* Get data from the IMU and DMP and timestamp it */
    timestamp = micros();
    myICM.getAGMT();

#if ENABLE_DMP == true
    myICM.readDMPdataFromFIFO(&angle_data);
#endif

#if ENABLE_DMP == true
    /* Process the quaternion data from the DMP into Euler angles */

#if ENABLE_DMP_MAGNETOMETER == true
    if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) {  // Was valid data available?
      if ((angle_data.header & DMP_header_bitmap_Quat9) > 0) {
        float q1 = ((float)angle_data.Quat9.Data.Q1) / 1073741824.0;  // Convert to double. Divide by 2^30
        float q2 = ((float)angle_data.Quat9.Data.Q2) / 1073741824.0;  // Convert to double. Divide by 2^30
        float q3 = ((float)angle_data.Quat9.Data.Q3) / 1073741824.0;  // Convert to double. Divide by 2^30
#else
    if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) {  // Was valid data available?
      if ((angle_data.header & DMP_header_bitmap_Quat6) > 0) {
        float q1 = ((float)angle_data.Quat6.Data.Q1) / 1073741824.0;  // Convert to double. Divide by 2^30
        float q2 = ((float)angle_data.Quat6.Data.Q2) / 1073741824.0;  // Convert to double. Divide by 2^30
        float q3 = ((float)angle_data.Quat6.Data.Q3) / 1073741824.0;  // Convert to double. Divide by 2^30
#endif
        float q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
        float q2sqr = q2 * q2;
        /* pitch (x-axis rotation) */
        double t0 = +2.0 * (q0 * q1 + q2 * q3);
        double t1 = +1.0 - 2.0 * (q1 * q1 + q2sqr);
        IMUData.pitch = atan2(t0, t1) * 180.0 / PI;

        /* roll (y-axis rotation) don't need to calculate since rover cannot move in this axis */
        // float t2 = +2.0 * (q0 * q2 - q3 * q1);
        // t2 = t2 > 1.0 ? 1.0 : t2;
        // t2 = t2 < -1.0 ? -1.0 : t2;
        // roll = asin(t2) * 180.0 / PI;

        /* yaw (z-axis rotation) */
        float t3 = +2.0 * (q0 * q3 + q1 * q2);
        float t4 = +1.0 - 2.0 * (q2sqr + q3 * q3);
        IMUData.yaw = atan2(t3, t4) * 180.0 / PI;

        IMUData.pitchRate = myICM.gyrX();
        IMUData.yawRate = myICM.gyrZ();

#if ENABLE_IMU_DEBUG == true
        SERIAL_PORT.print("Pitch: ");
        SERIAL_PORT.print(IMUData.pitch);
        SERIAL_PORT.print(", Yaw: ");
        SERIAL_PORT.print(IMUData.yaw);
        samples++;
        totalYawDrift += IMUData.yaw - prevYaw;
        prevYaw = IMUData.yaw;
        yawDrift = totalYawDrift / samples;
        SERIAL_PORT.print(", Yaw drift: ");
        SERIAL_PORT.println(yawDrift, 10);
#endif

        /* Write the object that stores the IMU data to a single item queue to distribute to other tasks */
        xQueueOverwrite(IMUDataQueue, &IMUData);
      }
    }

#else

    /* Acquire latest sensor data */
    FusionVector gyroscope = { myICM.gyrX(), myICM.gyrY(), myICM.gyrZ() };
    FusionVector accelerometer = { myICM.accX() / 1000.0, myICM.accY() / 1000.0, myICM.accZ() / 1000.0 };
    // FusionVector magnetometer = { myICM.magX(), myICM.magY(), myICM.magZ() };

    /* Update gyroscope offset correction algorithm */
    // gyroscope = FusionOffsetUpdate(&offset, gyroscope);

    /* Calculate delta time (in seconds) to account for gyroscope sample clock error */
    deltaTime = (float)(timestamp - previousTimestamp) / (float)1000000.0;
    previousTimestamp = timestamp;

    /* Update gyroscope AHRS algorithm */
    // FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer, deltaTime);
    FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, deltaTime);

    /* Print algorithm outputs */
    const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    // const FusionVector linearAcceleration = FusionAhrsGetLinearAcceleration(&ahrs);

#endif
  }
}
