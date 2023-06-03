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
static ICM_20948_SPI myICM;   /* Create an ICM_20948_SPI object */
volatile static float acc[3]; /* x, y ,z */
volatile static float gyr[3]; /* x, y ,z */
volatile static float mag[3]; /* x, y ,z */

/* IMU DMP */
volatile float pitch;
volatile float yaw;
volatile float roll;

/* Task handles */
TaskHandle_t taskIMUHandle = nullptr;

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

  /* Initialise DMP*/
  static bool success = true;
  success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR) == ICM_20948_Stat_Ok);
  success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat6, 0) == ICM_20948_Stat_Ok);  // Set to the maximum
  success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);

  if (success) {
    SERIAL_PORT.println("DMP initialised");
  } else {
    while (true) {
      SERIAL_PORT.println(F("Enable DMP failed!"));
      SERIAL_PORT.println(F("Please check that you have uncommented line 29 (#define ICM_20948_USE_DMP) in ICM_20948_C.h..."));
      delay(1000);
    }
  }

  /* Start the magnetometer */
  myICM.startupMagnetometer();
  if (myICM.status != ICM_20948_Stat_Ok) {
    SERIAL_PORT.print(F("startupMagnetometer returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

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

/* Task to sample the IMU */
void taskIMU(void *pvParameters) {

  (void)pvParameters;

  SERIAL_PORT.println("pog"); /* !!DO NOT REMOVE!! FOR SOME GODFORSAKEN REASON REMOVING THIS LINE CAUSES THE MCU TO BOOT LOOP */

  /* This algorithm is based on the revised AHRS algorithm presented in chapter 7 of
    Madgwick's PhD thesis: https://ethos.bl.uk/OrderDetails.do?uin=uk.bl.ethos.681552

    The source code can be found in: https://github.com/xioTechnologies/Fusion */

  /* Initialise algorithms */
  FusionOffset offset;
  FusionAhrs ahrs;

  FusionOffsetInitialise(&offset, IMU_SAMPLING_FREQUENCY);
  FusionAhrsInitialise(&ahrs);

  /* Set AHRS algorithm settings */
  const FusionAhrsSettings settings = {
    .convention = FusionConventionNwu,
    .gain = 0.5f,
    .accelerationRejection = 10.0f,
    .magneticRejection = 20.0f,
    .rejectionTimeout = 1 * IMU_SAMPLING_FREQUENCY, /* 1 seconds */
  };

  FusionAhrsSetSettings(&ahrs, &settings);

  /* Create timing variables */
  static unsigned long timestamp;         /* Timestamp of samples taken in microseconds */
  static unsigned long previousTimestamp; /* Time since last sample in microseconds */
  static float deltaTime;                 /* Time between samples in seconds */

  /* Create object to store DMP data */
  static icm_20948_DMP_data_t angle_data;

  /* Start the loop */
  while (true) {
    
    /* Wait for the data ready interrupt before sampling the IMU */
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

    /* Get data from the IMU and timestamp */
    if (xSemaphoreTake(mutexSPI, portMAX_DELAY) == pdTRUE) {
      timestamp = micros();
      myICM.getAGMT();
      myICM.readDMPdataFromFIFO(&angle_data);
      xSemaphoreGive(mutexSPI);

      /* Process the quaternion data from the DMP into Euler angles */
      if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) {  // Was valid data available?
        if ((angle_data.header & DMP_header_bitmap_Quat6) > 0) {
          float q1 = ((float)angle_data.Quat6.Data.Q1) / 1073741824.0;  // Convert to double. Divide by 2^30
          float q2 = ((float)angle_data.Quat6.Data.Q2) / 1073741824.0;  // Convert to double. Divide by 2^30
          float q3 = ((float)angle_data.Quat6.Data.Q3) / 1073741824.0;  // Convert to double. Divide by 2^30
          float q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
          float q2sqr = q2 * q2;

          /* roll (x-axis rotation) don't need to calculate since rover cannot move in this axis */
          // double t0 = +2.0 * (q0 * q1 + q2 * q3);
          // double t1 = +1.0 - 2.0 * (q1 * q1 + q2sqr);
          // roll = atan2(t0, t1) * 180.0 / PI;

          /* pitch (y-axis rotation) */
          float t2 = +2.0 * (q0 * q2 - q3 * q1);
          t2 = t2 > 1.0 ? 1.0 : t2;
          t2 = t2 < -1.0 ? -1.0 : t2;
          pitch = asin(t2) * 180.0 / PI;

          /* yaw (z-axis rotation) */
          float t3 = +2.0 * (q0 * q3 + q1 * q2);
          float t4 = +1.0 - 2.0 * (q2sqr + q3 * q3);
          yaw = atan2(t3, t4) * 180.0 / PI;
        }
      }

      // Acquire latest sensor data
      FusionVector gyroscope = { myICM.gyrX(), myICM.gyrY(), myICM.gyrZ() };
      FusionVector accelerometer = { myICM.accX() / 1000.0, myICM.accY() / 1000.0, myICM.accZ() / 1000.0 };
      FusionVector magnetometer = { myICM.magX(), myICM.magY(), myICM.magZ() };

      // Update gyroscope offset correction algorithm
      gyroscope = FusionOffsetUpdate(&offset, gyroscope);

      // Calculate delta time (in seconds) to account for gyroscope sample clock error
      deltaTime = (float)(timestamp - previousTimestamp) / (float)1000000.0;
      previousTimestamp = timestamp;

      // Update gyroscope AHRS algorithm
      FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer, deltaTime);

      // Print algorithm outputs
      // const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
      const FusionVector linearAcceleration = FusionAhrsGetLinearAcceleration(&ahrs);

      /* Store linear acceleration in volatile array so it can be accessed outside of this task */
      // acceleration[0] = linearAcceleration.axis.x;
      // acceleration[1] = linearAcceleration.axis.y;
      // acceleration[2] = linearAcceleration.axis.z;
    }
  }
}
