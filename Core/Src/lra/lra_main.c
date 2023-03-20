/*
 * lra_main.c
 *
 *  Created on: Feb 8, 2023
 *      Author: Dennis
 */

/* includes */

// contrains user config header, should be the top of include files
#include "lra/lra_main.h"

#include "main.h"

// includes device related headers
#include "devices/adxl355.h"
#include "lra/lra_i2c_devices.h"

#include "lra/lra_error.h"
#include "lra/lra_usb.h"

#include "lra/lra_pwm.h"

/* external variables */



// from main.c
extern I2C_HandleTypeDef hi2c1;

extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim8;

/* public functions */

void LRA_Main_EnterPoint(void) {
  // LED flash *2: enter LRA Main EnterPoint
  LRA_LED_Flash_N(2, 500);

  LRA_Main_System_Init();

  // create error status variable
  uint8_t error = 0;

  /* I2C devices init */

  HAL_StatusTypeDef ret;

  // create devices' structs
  TCA9546_t tca = {
      .ch = 0,
      .dev_addr = tca9546_default_addr,
      .timeout_ms = tca9546_default_timeout_ms,
      .reset_pin = TCA_NRST_Pin,
      .reset_port = TCA_NRST_GPIO_Port,
      .hi2c = &hi2c1,
  };
  DRV2605L_t drv_x = {
      .dev_addr = drv2605l_default_addr,
      .timeout_ms = drv2605l_default_timeout_ms,
      .en_pin = 0,
      .en_port = NULL,
      .hi2c = &hi2c1,
  };
  DRV2605L_t drv_y = drv_x;
  DRV2605L_t drv_z = drv_x;

  // create TCA9546A and DRV2605L pair structs
  TCA_DRV_Pair_t tca_drv_x = {.located_ch = 1, .pDrv = &drv_x, .pTca = &tca};
  TCA_DRV_Pair_t tca_drv_y = {.located_ch = 2, .pDrv = &drv_y, .pTca = &tca};
  TCA_DRV_Pair_t tca_drv_z = {.located_ch = 3, .pDrv = &drv_z, .pTca = &tca};

  // create I2C_Devs struct
  LRA_I2C_Devs_t i2c_devs = {.pair_count = LRA_MOTOR_NUM,
                             .pDevPair = {&tca_drv_x, &tca_drv_y, &tca_drv_z}};

  // create PWM array
  LRA_PWM_t pwm_x = {
      .htim = &htim2,
      .ch = TIM_CHANNEL_1,
  };

  LRA_PWM_t pwm_y = {
      .htim = &htim2,
      .ch = TIM_CHANNEL_2,
  };

  LRA_PWM_t pwm_z = {
      .htim = &htim2,
      .ch = TIM_CHANNEL_3,
  };

  LRA_PWM_t* pwm_arr[] = {&pwm_x, &pwm_y, &pwm_z};

  /* create ADXL355 Instance */
  ADXL355_t adxl355 = {
      .hspi = &hspi3,
      .nss_pin = 0,
      .nss_port = NULL,
      .range = acc_2g,
      .timeout_ms = 1,

      // use temp default parser, change these parameters after adjustment
      .temp_intercept_lsb = 0,
      .temp_slope = 0.0,
      .temp_intercept_Celsius = 0.0,
  };

  /* I2C devs init */
  ret = LRA_I2C_Devs_Init(&i2c_devs);
  if (ret != HAL_OK)
    error |= (1 << LRA_INIT_ERR_I2C_DEVS);

  /* PWM init */
  for (int i = 0; i < LRA_MOTOR_NUM; i++) {
    ret = Lra_PWM_Init(pwm_arr[i], LRA_DEFAULT_PWM_FREQ, LRA_DEFAULT_PWM_DUTY);
    if (ret != HAL_OK)
      error |= (1 << LRA_INIT_ERR_PWM);
  }

  /* TODO: MPU6500 init */

  if (ret != HAL_OK)
    error |= (1 << LRA_INIT_ERR_MPU6500);

  /* TODO: ADXL355 init */
  ret = ADXL355_Init(&adxl355);
  if (ret != HAL_OK)
    error |= (1 << LRA_INIT_ERR_ADXL355);

  /* USB Init */
  ret = LRA_USB_Init(LRA_USB_CRTL_MODE);
  if (ret != HAL_OK)
    error |= (1 << LRA_INIT_ERR_USB);
    
  // you should check error code here
  error;

  // Flash *2: init end
  LRA_LED_Flash_N(2, 500);

  /* parameters used in main loop */

  /**
   * @brief pwm related variables \r\n
   *
   * pwm_flag
   *
   * val 0 0 0 0 0 0 0 0
   *     - - - - - - - -
   * bit 7 6 5 4 3 2 1 0
   *
   * 0th bit: update pwm freq
   * 1th bit: update pwm duty cycle
   *
   * check enum LRA_PWM_FLAG in lra_main.h also
   */
  uint8_t pwm_flag = 0;

  // CMD
  uint32_t pwm_freq_cmd[LRA_MOTOR_NUM] = {
      LRA_DEFAULT_PWM_FREQ, LRA_DEFAULT_PWM_FREQ, LRA_DEFAULT_PWM_FREQ};
  uint16_t pwm_duty_cmd[LRA_MOTOR_NUM] = {
      LRA_DEFAULT_PWM_DUTY, LRA_DEFAULT_PWM_DUTY, LRA_DEFAULT_PWM_DUTY};

  // TODO: usb buffers

  // TODO: acc buffers

  // System enable
  for (int i = 0; i < LRA_MOTOR_NUM; i++) {
    Lra_PWM_Enable(pwm_arr[i]);
  }

  // Test code

  while (1) {
    /* Loop update */

    /* usb parser */

    // usb receive flag & parser

    // usb tranmit flag

    // cmd update flag

    // internal update flag



    // --------------------------------

    /* feedback */

    /* calculate new pwm signal */

    /* update pwm singal */

    /* transport data to Rasp */
  }
}

/**
 * @brief
 *
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LRA_Main_System_Init(void) {
  // System interrupt related
  HAL_TIM_Base_Start_IT(&htim6);

  // DRV init
  DRV2605L_t drvx = {
      .dev_addr = drv2605l_default_addr,
      .timeout_ms = drv2605l_default_timeout_ms,
      .en_pin = 0,
      .en_port = NULL,
      .hi2c = &hi2c1,
  };

  uint8_t drv_buf[DRV2605L_Total_Reg_Num] = {0};
  // combine into lra_i2c_devices later
  DRV2605L_Read_All(&drvx, drv_buf);

  return HAL_OK;
}
