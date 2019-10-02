#ifndef _HAL_
#define _HAL_



#define DMA_Channel_0                     ((uint32_t)0x00000000)
#define DMA_Channel_1                     ((uint32_t)0x02000000)
#define DMA_Channel_2                     ((uint32_t)0x04000000)
#define DMA_Channel_3                     ((uint32_t)0x06000000)
#define DMA_Channel_4                     ((uint32_t)0x08000000)
#define DMA_Channel_5                     ((uint32_t)0x0A000000)
#define DMA_Channel_6                     ((uint32_t)0x0C000000)
#define DMA_Channel_7                     ((uint32_t)0x0E000000)

/*
 * GPIOS
 */

#define RCC_AHB1Periph_GPIOBUTTON       (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE)

// Keys
#ifdef PCBX7
 #ifdef PCBT12
  #define PIN_BUTTON_MENU		        GPIO_Pin_10	// PE.10
	#define	PIN_BUTTON_EXIT           GPIO_Pin_2	// PD.02
  #define	PIN_BUTTON_UP		          GPIO_Pin_9	// PE.09
  #define	PIN_BUTTON_DOWN		        GPIO_Pin_11	// PE.11
  #define	PIN_BUTTON_LEFT	          GPIO_Pin_7	// PD.07
  #define	PIN_BUTTON_RIGHT          GPIO_Pin_3	// PD.03
 #else
  #define PIN_BUTTON_MENU		        GPIO_Pin_7	// PD.07
  #define	PIN_BUTTON_EXIT           GPIO_Pin_2	// PD.02
  #define PIN_BUTTON_PAGE           GPIO_Pin_3  // PD.03
  #define PIN_BUTTON_ENCODER        GPIO_Pin_10 // PE.10
 #endif
#else // now not X7
 #ifdef REV9E
  #define PIN_BUTTON_MENU		        GPIO_Pin_7	//SW4 PD.07
  #define	PIN_BUTTON_EXIT           GPIO_Pin_2	//SW5 PD.02
  #define PIN_BUTTON_PAGE           GPIO_Pin_3      //SW6 PD.03
  #define PIN_BUTTON_ENCODER        GPIO_Pin_0      // PF.00
 #else
  #if defined(PCBXLITE)
   #define PIN_BUTTON_MENU		        GPIO_Pin_11	// PE.11
   #define	PIN_BUTTON_EXIT           GPIO_Pin_7	// PE.07
   #define	PIN_BUTTON_UP		          GPIO_Pin_10	// PE.10
   #define	PIN_BUTTON_DOWN		        GPIO_Pin_14	// PE.14
   #define	PIN_BUTTON_LEFT	          GPIO_Pin_12	// PE.12
   #define	PIN_BUTTON_RIGHT          GPIO_Pin_13	// PE.13
   #define	PIN_BUTTON_SHIFT          GPIO_Pin_08	// PE.08
  #else
   #ifdef PCBX9LITE
    #define PIN_BUTTON_MENU		        GPIO_Pin_7	// PE.07
    #define	PIN_BUTTON_EXIT           GPIO_Pin_9	// PE.09
    #define PIN_BUTTON_PAGE           GPIO_Pin_8  // PE.08
	  #define PIN_BUTTON_ENCODER        GPIO_Pin_11 // PE.11
   #else

#if defined(REV19)
    #define PIN_BUTTON_MENU		        GPIO_Pin_2	//SW4 PD.02
    #define	PIN_BUTTON_EXIT           GPIO_Pin_7	//SW5 PD.07
    #define PIN_BUTTON_PAGE           GPIO_Pin_3  //SW6 PD.03
	  #define PIN_BUTTON_ENCODER        GPIO_Pin_11 // PE.11
#else    
		#define	PIN_BUTTON_PLUS		        GPIO_Pin_10	//SW3 PE.10
    #define	PIN_BUTTON_MINUS	        GPIO_Pin_11	//SW2 PE.11
    #define	PIN_BUTTON_ENTER	        GPIO_Pin_12	//SW1 PE.12
    #define PIN_BUTTON_MENU		        GPIO_Pin_7	//SW4 PD.07
    #define	PIN_BUTTON_EXIT           GPIO_Pin_2	//SW5 PD.02
    #define PIN_BUTTON_PAGE           GPIO_Pin_3  //SW6 PD.03
#endif // REV19
	 
	 #endif // X3
  #endif // XLITE
 #endif // REV9E
#endif // PCBX7

#ifdef PCBX7
#define LED_GREEN_GPIO                  GPIOC
#define LED_GREEN_GPIO_PIN              GPIO_Pin_4  // PC.04
#define LED_RED_GPIO                    GPIOC
#define LED_RED_GPIO_PIN                GPIO_Pin_5  // PC.05
#define LED_BLUE_GPIO                   GPIOB
#define LED_BLUE_GPIO_PIN               GPIO_Pin_1  // PB.01
#endif // PCBX7

#ifdef PCBX9LITE
#define LED_GREEN_GPIO                  GPIOE
#define LED_GREEN_GPIO_PIN              GPIO_Pin_5  // PE.05
#define LED_RED_GPIO                    GPIOE
#define LED_RED_GPIO_PIN                GPIO_Pin_4  // PE.04
#define LED_BLUE_GPIO                   GPIOE
#define LED_BLUE_GPIO_PIN               GPIO_Pin_6  // PE.06
#endif // PCBX9LITE

#if defined(REV19)
#define LED_GPIO                  GPIOA
#define LED_GPIO_PIN              GPIO_Pin_13  // PA.13
#endif // REV19


#ifdef PCBX7
#define GPIOENCODER             GPIOE
#define PIN_ENC1				        GPIO_Pin_9      //  PE.09
#define PIN_ENC2				        GPIO_Pin_11     //  PE.11
#endif // PCBX7
#ifdef REV9E
// X9E rev0.2
#define GPIOENCODER             GPIOD
#define PIN_ENC1				        GPIO_Pin_12      //  PD.12
#define PIN_ENC2				        GPIO_Pin_13      //  PD.13

#endif // REV9E

#ifdef PCBX9LITE
#define GPIOENCODER             GPIOE
#define PIN_ENC1				        GPIO_Pin_10     //  PE.10
#define PIN_ENC2				        GPIO_Pin_12     //  PE.12
#endif // X3

#ifdef REV19
#define GPIOENCODER             GPIOE
#define PIN_ENC1				        GPIO_Pin_10     //  PE.10
#define PIN_ENC2				        GPIO_Pin_12     //  PE.12
#endif // X3


// Trims
#ifdef REV9E
#define	PIN_TRIMLV_DN		        GPIO_Pin_4	//PE.04
#define PIN_TRIMLV_UP           GPIO_Pin_3  //PE.03
#define PIN_TRIMRH_UP		        GPIO_Pin_13	//PC.13
#define	PIN_TRIMRH_DN		        GPIO_Pin_1	//PC.01
#define	PIN_TRIMRV_DN		        GPIO_Pin_3	//PC.03
#define	PIN_TRIMRV_UP	          GPIO_Pin_2	//PC.02
#define PIN_TRIMLH_UP           GPIO_Pin_0  //PG.00
#define PIN_TRIMLH_DN           GPIO_Pin_1  //PG.01
#endif // REV9E

#ifdef PCBX7
 #define PIN_TRIMLV_DN		        GPIO_Pin_6	//PE.06
 #define PIN_TRIMLV_UP           GPIO_Pin_5  //PE.05
 #define PIN_TRIMRV_DN		        GPIO_Pin_3	//PC.03
 #define PIN_TRIMRV_UP	          GPIO_Pin_2	//PC.02
 #define PIN_TRIMRH_UP		        GPIO_Pin_4	//PE.04
 #define PIN_TRIMRH_DN		        GPIO_Pin_3	//PE.03
 #define PIN_TRIMLH_UP           GPIO_Pin_1  //PC.01
 #define PIN_TRIMLH_DN           GPIO_Pin_15 //PD.15
#endif // PCBX7

#if defined(PCBXLITE)
 #define	PIN_TRIMLH_DN		        GPIO_Pin_4	//PC.04
 #define PIN_TRIMLH_UP           GPIO_Pin_5  //PC.05
 #define PIN_TRIMLV_UP           GPIO_Pin_0  //PB.00
 #define PIN_TRIMLV_DN           GPIO_Pin_1  //PB.01
#endif // XLITE
   
#ifdef PCBX9LITE
 #define PIN_TRIMLV_DN		        GPIO_Pin_1	//PB.01
 #define PIN_TRIMLV_UP           GPIO_Pin_0  //PB.00
 #define PIN_TRIMRV_DN		        GPIO_Pin_14	//PE.14
 #define PIN_TRIMRV_UP	          GPIO_Pin_13	//PE.13
 #define PIN_TRIMRH_UP		        GPIO_Pin_8	//PD.08
 #define PIN_TRIMRH_DN		        GPIO_Pin_9	//PD.09
 #define PIN_TRIMLH_UP           GPIO_Pin_4  //PC.04
 #define PIN_TRIMLH_DN           GPIO_Pin_5	//PC.05
#endif // PCBX9LITE

#if defined(REVNORM) || defined(REVPLUS) || defined(REV19)
 #define PIN_TRIMLH_DN		       GPIO_Pin_4	//PE.04
 #define PIN_TRIMLH_UP           GPIO_Pin_3  //PE.03
 #define PIN_TRIMRV_UP	         GPIO_Pin_2	//PC.02
 #define PIN_TRIMRV_DN		       GPIO_Pin_3	//PC.03
 #define PIN_TRIMRH_UP		       GPIO_Pin_13	//PC.13
 #define PIN_TRIMRH_DN		       GPIO_Pin_1	//PC.01
 #define PIN_TRIMLV_UP           GPIO_Pin_5  //PE.05
 #define PIN_TRIMLV_DN           GPIO_Pin_6  //PE.06
#endif // NORM || PLUS || 19

// Switches
#ifdef PCBX7
 #define	PIN_SW_A_L		        GPIO_Pin_7	//PE.07
 #define	PIN_SW_A_H		        GPIO_Pin_13	//PE.13
 #define	PIN_SW_B_L		        GPIO_Pin_15 //PE.15
 #define	PIN_SW_B_H		        GPIO_Pin_5  //PA.05
 #define	PIN_SW_C_L		        GPIO_Pin_11 //PD.11
 #define	PIN_SW_C_H		        GPIO_Pin_0  //PE.00
 #define	PIN_SW_D_L		        GPIO_Pin_1  //PE.01
 #define	PIN_SW_D_H		        GPIO_Pin_2  //PE.02
 #define	PIN_SW_F			        GPIO_Pin_14	//PE.14
 #define	PIN_SW_H			        GPIO_Pin_14 //PD.14
#endif // PCBX7

#if defined(PCBXLITE)
 #define	PIN_SW_A_L		        GPIO_Pin_0	//PE.00
 #define	PIN_SW_A_H		        GPIO_Pin_1	//PE.01
 #define	PIN_SW_B_L		        GPIO_Pin_6	//PA.06
 #define	PIN_SW_B_H		        GPIO_Pin_5	//PA.05
 #define	PIN_SW_C_L		        GPIO_Pin_2	//PE.02
 #define	PIN_SW_C_H		        GPIO_Pin_3	//PE.03
 #define	PIN_SW_D_L		        GPIO_Pin_4	//PB.04
 #define	PIN_SW_D_H		        GPIO_Pin_5	//PB.05
#endif  // PCBXLITE
  
#ifdef PCBX9LITE
 #define	PIN_SW_A_H		        GPIO_Pin_1	//PE.01
 #define	PIN_SW_A_L		        GPIO_Pin_0	//PE.00
 #define	PIN_SW_B_H		        GPIO_Pin_5	//PB.05
 #define	PIN_SW_B_L		        GPIO_Pin_4  //PB.04
 #define	PIN_SW_C_H		        GPIO_Pin_3	//PE.03
 #define	PIN_SW_C_L		        GPIO_Pin_2  //PE.02
 #define	PIN_SW_F			        GPIO_Pin_13	//PC.13
 #define	PIN_SW_H			        GPIO_Pin_5	//PA.05
#endif // PCBX9LITE

#if defined(REV3)
 #define	PIN_SW_A_L		        GPIO_Pin_14	//PE.14
 #define	PIN_SW_B_H		        GPIO_Pin_3	//PB.03
 #define	PIN_SW_B_L		        GPIO_Pin_4	//PB.04
 #define	PIN_SW_C_H		        GPIO_Pin_5	//PB.05
 #define	PIN_SW_C_L		        GPIO_Pin_6	//PB.06
 #define	PIN_SW_D_H		        GPIO_Pin_7	//PB.07
 #define	PIN_SW_D_L		        GPIO_Pin_2	//PE.02
 #define	PIN_SW_E_H		        GPIO_Pin_0	//PB.00
 #define	PIN_SW_E_L		        GPIO_Pin_5	//PA.05
 #define	PIN_SW_F_H		        GPIO_Pin_7	//PE.07
 #define	PIN_SW_F_L		        GPIO_Pin_1	//PB.01
 #define	PIN_SW_G_H		        GPIO_Pin_9	//PE.09
 #define	PIN_SW_G_L		        GPIO_Pin_8	//PE.08
 #define	PIN_SW_H_L		        GPIO_Pin_13	//PE.13
#endif // REV3
     
#if defined(REVNORM) || defined(REVPLUS)
 #define	PIN_SW_A_L		        GPIO_Pin_0	//PE.00
 #define	PIN_SW_A_H		        GPIO_Pin_5	//PB.05
 #define	PIN_SW_B_L		        GPIO_Pin_2	//PE.02
 #define	PIN_SW_B_H		        GPIO_Pin_1	//PE.01
 #define	PIN_SW_C_L		        GPIO_Pin_5	//PA.05
 #define	PIN_SW_C_H		        GPIO_Pin_15	//PE.15
  #if defined(REVPLUS)
   #define	PIN_SW_D_L		        GPIO_Pin_13	//PE.13
  #else // REVPLUS
   #define	PIN_SW_D_L		        GPIO_Pin_1	//PB.01
  #endif // REVPLUS
 #define	PIN_SW_D_H		        GPIO_Pin_7	//PE.07
 #define	PIN_SW_E_L		        GPIO_Pin_4	//PB.04
 #define	PIN_SW_E_H		        GPIO_Pin_3	//PB.03
 #define	PIN_SW_F			        GPIO_Pin_14	//PE.14
 #define	PIN_SW_G_L		        GPIO_Pin_8	//PE.08
 #define	PIN_SW_G_H		        GPIO_Pin_9	//PE.09
 #ifdef REVPLUS
  #define	PIN_SW_H			        GPIO_Pin_14	//PD.14
 #else // REVPLUS
  #define	PIN_SW_H			        GPIO_Pin_13	//PE.13
 #endif // REVPLUS
#endif // REVPLUS || REVNORM

#if defined(REV19)
 #define	PIN_SW_A_L		      GPIO_Pin_0	//PE.00
 #define	PIN_SW_A_H		      GPIO_Pin_5	//PB.05
 #define	PIN_SW_B_L		      GPIO_Pin_2	//PE.02
 #define	PIN_SW_B_H		      GPIO_Pin_1	//PE.01
 #define	PIN_SW_C_L		      GPIO_Pin_5	//PA.05
 #define	PIN_SW_C_H		      GPIO_Pin_15	//PE.15
 #define	PIN_SW_D_L		      GPIO_Pin_13	//PE.13
 #define	PIN_SW_D_H		      GPIO_Pin_7	//PE.07
 #define	PIN_SW_E_L		      GPIO_Pin_4	//PB.04
 #define	PIN_SW_E_H		      GPIO_Pin_3	//PB.03
 #define	PIN_SW_F			      GPIO_Pin_14	//PE.14
 #define	PIN_SW_G_L		      GPIO_Pin_8	//PE.08
 #define	PIN_SW_G_H		      GPIO_Pin_9	//PE.09
 #define	PIN_SW_H			      GPIO_Pin_13	//PD.14

 #define	PIN_SW_KEYL		      GPIO_Pin_10	//PD.10
 #define	PIN_SW_KEYR		      GPIO_Pin_11	//PD.11

#endif // REV19


#ifdef REV9E

#define	PIN_SW_A_L		        GPIO_Pin_13	//PE.13
#define	PIN_SW_A_H		        GPIO_Pin_7	//PE.07
#define	PIN_SW_B_H		        GPIO_Pin_10	//PD.10
#define	PIN_SW_B_L		        GPIO_Pin_14	//PD.14
#define	PIN_SW_C_H		        GPIO_Pin_11	//PG.11
#define	PIN_SW_C_L		        GPIO_Pin_10	//PG.10
//#define	PIN_SW_D_H		        
#define	PIN_SW_D_L		        GPIO_Pin_11	//PE.11
#define	PIN_SW_E_L		        GPIO_Pin_4	//PF.04
#define	PIN_SW_E_H		        GPIO_Pin_3	//PF.03
#define	PIN_SW_F_L		        GPIO_Pin_2	//PE.02
#define	PIN_SW_F_H		        GPIO_Pin_1	//PE.01
#define	PIN_SW_G_L		        GPIO_Pin_14	//PF.14
#define	PIN_SW_G_H		        GPIO_Pin_13	//PF.13
#define	PIN_SW_H_L		        GPIO_Pin_1	//PF.01

#define	PIN_SW_I_L		        GPIO_Pin_14	//PE.14
#define	PIN_SW_I_H		        GPIO_Pin_15	//PF.15
#define	PIN_SW_J_L		        GPIO_Pin_8	//PG.08
#define	PIN_SW_J_H		        GPIO_Pin_7	//PG.07
#define	PIN_SW_K_L		        GPIO_Pin_6	//PF.06
#define	PIN_SW_K_H		        GPIO_Pin_5	//PF.05
#define	PIN_SW_L_L		        GPIO_Pin_8	//PE.08
#define	PIN_SW_L_H		        GPIO_Pin_9	//PE.09
#define	PIN_SW_M_L		        GPIO_Pin_5	//PA.05
#define	PIN_SW_M_H		        GPIO_Pin_15	//PE.15
#define	PIN_SW_N_L		        GPIO_Pin_4	//PB.04
#define	PIN_SW_N_H		        GPIO_Pin_3	//PB.03
#define	PIN_SW_O_L		        GPIO_Pin_10	//PE.10
#define	PIN_SW_O_H		        GPIO_Pin_7	//PF.07
#define	PIN_SW_P_L		        GPIO_Pin_12	//PF.12
#define	PIN_SW_P_H		        GPIO_Pin_11	//PF.11
#define	PIN_SW_Q_L		        GPIO_Pin_6	//PF.06
#define	PIN_SW_Q_H		        GPIO_Pin_5	//PF.05
#define	PIN_SW_R_L		        GPIO_Pin_0	//PE.00
#define	PIN_SW_R_H		        GPIO_Pin_5	//PB.05

#endif

#ifdef PCBX7
#define	PIN_SW_EXT1		        GPIO_Pin_13	//PC.13
#define	PIN_SW_EXT2		        GPIO_Pin_10	//PD.10
#endif


// ADC
#ifndef PCBXLITE
#ifdef PCBX9LITE
	#define RCC_AHB1Periph_GPIOADC        RCC_AHB1Periph_GPIOC
	
	#define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            RCC_APB1Periph_TIM5
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_3  // PA.03
  #define PWM_TIMER                     TIM5
  #define PWM_GPIO                      GPIOA
  #define PWM_GPIO_AF                   GPIO_AF_TIM5
  #define PWM_IRQHandler                TIM5_IRQHandler
  #define PWM_IRQn                      TIM5_IRQn
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_1  // PC.01
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define STICKS_GPIOA_PINS             (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
  #define PWM_GPIOA_PINS                STICKS_GPIOA_PINS
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_POT1 | (ANALOGS_PWM_ENABLED() ? 0 : STICKS_GPIOA_PINS))
  #define ADC_GPIOB_PINS                ADC_GPIO_PIN_POT2
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              ADC_Channel_6
  #define ADC_CHANNEL_POT2              ADC_Channel_8
  #define ADC_CHANNEL_BATT              ADC_Channel_10

#define PIN_FLP_J1                      GPIO_Pin_1  //PC.01
#define PIN_MVOLT                       GPIO_Pin_0  //PC.00  
#define PIN_STICK_RV						        GPIO_Pin_0  // PA.00
#define PIN_STICK_RH						        GPIO_Pin_1  // PA.01
#define PIN_STICK_LV        						GPIO_Pin_2  // PA.02
#define PIN_STICK_LH        						GPIO_Pin_3  // PA.03

#define PIN_STK_J1                      GPIO_Pin_0  //PA.00              
#define PIN_STK_J2                      GPIO_Pin_1  //PA.01
#define PIN_STK_J3                      GPIO_Pin_2  //PA.02
#define PIN_STK_J4                      GPIO_Pin_3  //PA.03

#define PIN_SLD_J1                      GPIO_Pin_4  //PC.04
#define PIN_SLD_J2                      GPIO_Pin_5  //PC.05
#define PIN_FLP_J2                      GPIO_Pin_0  //PB.00


#else // PCBX9LITE
#ifdef PCBX7
#define PIN_STK_J1                      GPIO_Pin_0  //PA.00              
#define PIN_STK_J2                      GPIO_Pin_1  //PA.01
#define PIN_STK_J3                      GPIO_Pin_2  //PA.02
#define PIN_STK_J4                      GPIO_Pin_3  //PA.03
#define PIN_FLP_J1                      GPIO_Pin_6  //PA.06
#define PIN_FLP_J2                      GPIO_Pin_0  //PB.00
#define PIN_MVOLT                       GPIO_Pin_0  //PC.00  
#define RCC_AHB1Periph_GPIOADC          RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC
#else // PCBX7
#ifndef PCB9XT
// ADC OK for REV19
#define PIN_STK_J1                      GPIO_Pin_0  //PA.00              
#define PIN_STK_J2                      GPIO_Pin_1  //PA.01
#define PIN_STK_J3                      GPIO_Pin_2  //PA.02
#define PIN_STK_J4                      GPIO_Pin_3  //PA.03
#define PIN_SLD_J1                      GPIO_Pin_4  //PC.04
#define PIN_SLD_J2                      GPIO_Pin_5  //PC.05
#define PIN_FLP_J1                      GPIO_Pin_6  //PA.06
#define PIN_FLP_J2                      GPIO_Pin_0  //PB.00
#define PIN_MVOLT                       GPIO_Pin_0  //PC.00  
#if defined(REVPLUS) || defined(REV9E)
#define PIN_FLP_J3                      GPIO_Pin_1  //PB.01
#define RCC_AHB1Periph_GPIOADC          RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC
#else
#define RCC_AHB1Periph_GPIOADC          RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC
#endif
#ifdef REV9E
#define PIN_FLAP3                      GPIO_Pin_8  //PF.08
#define PIN_FLAP4                      GPIO_Pin_9  //PF.09
#define PIN_FLAP5                      GPIO_Pin_10  //PF.10
#define PIN_FLAP6                      GPIO_Pin_1  //PB.01
#endif
#endif // nPCB9XT
#endif // PCBX9LITE
#endif // PCBX7
#endif // nPCBXLITE



// DAC
#define PIN_AUDIO_DAC                   GPIO_Pin_4  //PA.04

// Power_OFF Delay and LED
#ifndef PCBXLITE
#ifndef PCB9XT
#ifndef PCBX9LITE
#ifndef REV19
 #define PIN_PWR_LED                     GPIO_Pin_6  //PC.06
#endif
#define PIN_PWR_STATUS                  GPIO_Pin_1  //PD.01
#define PIN_MCU_PWR                     GPIO_Pin_0  //PD.00
#endif

#ifdef PCBX9LITE
	#define GPIOPWRINT                      GPIOA
	#define PIN_INT_RF_PWR                  GPIO_Pin_15	// PA.15
	#define GPIOPWREXT                      GPIOA
	#define PIN_EXT_RF_PWR                  GPIO_Pin_8  // PA.08

	#define GPIOPWRSPORT                    GPIOE
	#define PIN_SPORT_PWR                   GPIO_Pin_15   // PE.15
	
	#define GPIOPWR		                    GPIOA
	#define PIN_MCU_PWR                   GPIO_Pin_6  // PA.06

	#define GPIOPWRSENSE                  GPIOA
	#define PIN_PWR_STATUS                GPIO_Pin_7  // PA.07


#else
#define RCC_AHB1Periph_GPIOPWR          RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD
#if defined(REVPLUS) || defined(REV9E)
#define GPIOPWRINT                      GPIOC
#else
 #ifdef PCBX7
  #define GPIOPWRINT                      GPIOC
 #else
  #define GPIOPWRINT                      GPIOD
 #endif
#endif
#define GPIOPWREXT                      GPIOD
#define GPIOPWR		                      GPIOD
#define GPIOPWRSENSE                    GPIOD

#if defined(REVPLUS) || defined(REV9E)
#define PIN_INT_RF_PWR                  GPIO_Pin_6	// PC6
#else
 #ifdef PCBX7
  #define PIN_INT_RF_PWR                  GPIO_Pin_6	// PC6
 #else
  #define PIN_INT_RF_PWR                  GPIO_Pin_15	// PD15
 #endif
#endif
#define PIN_EXT_RF_PWR                  GPIO_Pin_8  // PD.08
#endif // nPCB9XT
#endif // PCBXLITE
#endif // X3


#ifdef REV19
 #define GPIOPWRSPORT                    GPIOA
 #define PIN_SPORT_PWR                   GPIO_Pin_14   // PA.14
#endif // REV19



#ifdef REV19
 #define GPIOPWRINT                      GPIOA
 #define PIN_INT_RF_PWR                  GPIO_Pin_7	// PA7

 #define GPIOPWREXT                      GPIOD
 #define PIN_EXT_RF_PWR                  GPIO_Pin_8  // PD.08
#endif // REV19

// Smart-Port
#define PIN_SPORT_ON                    GPIO_Pin_4  //PD.04
#define PIN_SPORT_TX                    GPIO_Pin_5  //PD.05
#define PIN_SPORT_RX                    GPIO_Pin_6  //PD.06

// Trainer Port

#ifdef PCBX9LITE

#define GPIO_TR_INOUT                   GPIOD
#define PIN_TR_PPM_IN                   GPIO_Pin_13  //PD.13
#define PIN_TR_PPM_OUT                  GPIO_Pin_12  //PD.12
#define GPIOTRNDET                      GPIOD
#define PIN_TRNDET                      GPIO_Pin_11
#define GPIOBOOTCMD											GPIOC
#define PORT_BOOTCMD										PIN_PORTC
#define PIN_BOOTCMD                     GPIO_Pin_8	// PC.08


#else // X3
#define GPIO_TR_INOUT                   GPIOC
#define PIN_TR_PPM_IN                   GPIO_Pin_8  //PC.08
#define PIN_TR_PPM_OUT                  GPIO_Pin_9  //PC.09
#define GPIOTRNDET                      GPIOA
#define PIN_TRNDET                      GPIO_Pin_8
#endif // X3


// Cppm
#ifdef PCBX9LITE
//#define RCC_AHB1Periph_GPIOCPPM         RCC_AHB1Periph_GPIOA
#define RCC_AHB1Periph_GPIO_INTPPM      RCC_AHB1Periph_GPIOB
#define PORT_INTPPM                     PIN_PORTB
#define GPIO_INTPPM                    	GPIOB
#define PIN_INTPPM_OUT                  GPIO_Pin_6  //PB.06
//#define PIN_CPPM_OUT                    GPIO_Pin_8  //PA.08
//#define GPIOCPPM                        GPIOA
//#define GPIO_PinSource_CPPM             GPIO_PinSource8
#define GPIO_EXTPPM                    	GPIOC
#define PIN_EXTPPM_OUT                  GPIO_Pin_6  //PC.06
#define PORT_EXTPPM                     PIN_PORTC
//#define GPIO_PinSource_EXTPPM           GPIO_PinSource7

#else // X3
#define RCC_AHB1Periph_GPIOCPPM         RCC_AHB1Periph_GPIOA
#define RCC_AHB1Periph_GPIO_INTPPM      RCC_AHB1Periph_GPIOA
#define PORT_INTPPM                     PIN_PORTA
#define GPIO_INTPPM                    	GPIOA
#define PIN_INTPPM_OUT                  GPIO_Pin_10  //PA.10
#define PIN_CPPM_OUT                    GPIO_Pin_8  //PA.08
#define GPIOCPPM                        GPIOA
#define GPIO_PinSource_CPPM             GPIO_PinSource8
#ifndef PCBXLITE
#define PIN_EXTPPM_OUT                  GPIO_Pin_7  //PA.07
#define PORT_EXTPPM                     PIN_PORTA
#endif
#define GPIO_PinSource_EXTPPM           GPIO_PinSource7
#endif // X3

// Heart Beat
#ifdef PCBX9LITE
 #define PIN_HEART_BEAT                  GPIO_Pin_15  //PD.15
#else // X3
 #ifdef REV19
  #define PIN_HEART_BEAT                  GPIO_Pin_1  //PB.01
 #else // REV19
  #define PIN_HEART_BEAT                  GPIO_Pin_7  //PC.07
 #endif // REV19
#endif // X3

// Update UART----RS232 ---UART3
#define PIN_UPDATE_UART_TX              GPIO_Pin_10 //PB.10
#define PIN_UPDATE_UART_RX              GPIO_Pin_11 //PB.11

// USB_OTG
#define PIN_FS_VBUS                     GPIO_Pin_9  //PA.09
#define PIN_FS_DM                       GPIO_Pin_11 //PA.11
#define PIN_FS_DP                       GPIO_Pin_12 //PA.12
#define USB_RCC_AHB1Periph_GPIO         RCC_AHB1Periph_GPIOA
#define USB_GPIO_PIN_DM                 GPIO_Pin_11 //PA.11
#define USB_GPIO_PIN_DP                 GPIO_Pin_12 //PA.12
#define USB_GPIO_PinSource_DM           GPIO_PinSource11
#define USB_GPIO_PinSource_DP           GPIO_PinSource12
#define USB_GPIO_AF                     GPIO_AF_OTG1_FS
#define USB_GPIO                        GPIOA
#define USB_GPIO_PIN_VBUS               GPIO_Pin_9  // PA.09

#ifndef REV19
#define PIN_FS_ID                       GPIO_Pin_10 //PA.10
#endif

#ifdef PCBX7
#define RCC_AHB1Periph_GPIOHAPTIC       RCC_AHB1Periph_GPIOB
#define GPIO_Pin_HAPTIC                 GPIO_Pin_8  //PB.08
#define GPIOHAPTIC                      GPIOB
#define GPIO_PinSource_HAPTIC           GPIO_PinSource8
#endif

#if defined(REVPLUS) || defined(REV9E)
// Haptic PB8
#define RCC_AHB1Periph_GPIOHAPTIC       RCC_AHB1Periph_GPIOB
#define GPIO_Pin_HAPTIC                 GPIO_Pin_8  //PB.08
#define GPIOHAPTIC                      GPIOB
#define GPIO_PinSource_HAPTIC           GPIO_PinSource8

 #ifdef REV9E
// BackLight BLUE PE06
#define RCC_AHB1Periph_GPIOBL           RCC_AHB1Periph_GPIOE
#define GPIO_Pin_BL                     GPIO_Pin_6  //PE.06
#define GPIOBL                          GPIOE
#define GPIO_PinSource_BL               GPIO_PinSource6

// BackLight WHITE PE05
#define RCC_AHB1Periph_GPIOBLW          RCC_AHB1Periph_GPIOE
#define GPIO_Pin_BLW                    GPIO_Pin_5  //PE.05
#define GPIOBLW                         GPIOE
#define GPIO_PinSource_BLW              GPIO_PinSource5

 #else
// BackLight BLUE PD13
#define RCC_AHB1Periph_GPIOBL           RCC_AHB1Periph_GPIOD
#define GPIO_Pin_BL                     GPIO_Pin_15  //PD.15
#define GPIOBL                          GPIOD
#define GPIO_PinSource_BL               GPIO_PinSource15

// BackLight WHITE PD15
#define RCC_AHB1Periph_GPIOBLW          RCC_AHB1Periph_GPIOD
#define GPIO_Pin_BLW                    GPIO_Pin_13  //PD.13
#define GPIOBLW                         GPIOD
#define GPIO_PinSource_BLW              GPIO_PinSource13
 #endif

#else

 #ifdef REV19
 
#define RCC_AHB1Periph_GPIOHAPTIC       RCC_AHB1Periph_GPIOA
#define GPIO_Pin_HAPTIC                 GPIO_Pin_10  //PA.10
#define GPIOHAPTIC                      GPIOA
#define GPIO_PinSource_HAPTIC           GPIO_PinSource10
// Will need to use Timer 1

#define RCC_AHB1Periph_GPIOBL           RCC_AHB1Periph_GPIOD
#define GPIO_Pin_BL                     GPIO_Pin_15  //PD.15
#define GPIOBL                          GPIOD
#define GPIO_PinSource_BL               GPIO_PinSource15

// BackLight WHITE PD15
#define RCC_AHB1Periph_GPIOBLW          RCC_AHB1Periph_GPIOD
#define GPIO_Pin_BLW                    GPIO_Pin_13  //PD.13
#define GPIOBLW                         GPIOD
#define GPIO_PinSource_BLW              GPIO_PinSource13
 
 
 #else

// Haptic
#ifndef PCBX7
#ifndef PCBX9LITE
#define RCC_AHB1Periph_GPIOHAPTIC       RCC_AHB1Periph_GPIOC
#define GPIO_Pin_HAPTIC                 GPIO_Pin_12  //PC.12
#define GPIOHAPTIC                      GPIOC
#endif
#endif

// BackLight PB8
#define RCC_AHB1Periph_GPIOBL           RCC_AHB1Periph_GPIOB
#define GPIO_Pin_BL                     GPIO_Pin_8  //PB.08
#define GPIOBL                          GPIOB
#define GPIO_PinSource_BL               GPIO_PinSource8
 #endif
#endif


#if defined(REVPLUS) || defined(REV9E) || defined(REV19)

// LCD GPIOD 10-14
#define RCC_AHB1Periph_LCD              RCC_AHB1Periph_GPIOC
#define RCC_AHB1Periph_LCD_RST          RCC_AHB1Periph_GPIOD
#define RCC_AHB1Periph_LCD_NCS          RCC_AHB1Periph_GPIOA
#define GPIO_LCD                        GPIOC
#define GPIO_LCD_RST                    GPIOD
#define GPIO_LCD_NCS                    GPIOA
#define PIN_LCD_MOSI                    GPIO_Pin_12 //PC.12
#define PIN_LCD_CLK                     GPIO_Pin_10 //PC.10
#define PIN_LCD_NCS                     GPIO_Pin_15 //PA.15
#define PIN_LCD_A0                      GPIO_Pin_11 //PC.11
#define PIN_LCD_RST                     GPIO_Pin_12  //pd12 test //RESET occurs when powered up,but should delay before initialize

#else

// LCD GPIOD 10-14
#define RCC_AHB1Periph_LCD              RCC_AHB1Periph_GPIOD
#define GPIO_LCD                        GPIOD
#define PIN_LCD_MOSI                    GPIO_Pin_10 //PD.10
#define PIN_LCD_CLK                     GPIO_Pin_11 //PD.11
#define PIN_LCD_NCS                     GPIO_Pin_14 //PD.14
#define PIN_LCD_A0                      GPIO_Pin_13 //PD.13
#define PIN_LCD_RST                     GPIO_Pin_12  //pd12 test //RESET occurs when powered up,but should delay before initialize

#endif

#ifdef PCBX7
	#define LCD_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
  #define LCD_RCC_APB1Periph            RCC_APB1Periph_SPI3
  #define LCD_SPI_GPIO                  GPIOC
  #define LCD_MOSI_GPIO_PIN             GPIO_Pin_12 // PC.12
  #define LCD_MOSI_GPIO_PinSource       GPIO_PinSource12
  #define LCD_CLK_GPIO_PIN              GPIO_Pin_10 // PC.10
  #define LCD_CLK_GPIO_PinSource        GPIO_PinSource10
  #define LCD_A0_GPIO_PIN               GPIO_Pin_11 // PC.11
  #define LCD_NCS_GPIO                  GPIOA
  #define LCD_NCS_GPIO_PIN              GPIO_Pin_15 // PA.15
  #define LCD_RST_GPIO                  GPIOD
  #define LCD_RST_GPIO_PIN              GPIO_Pin_12 // PD.12
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF_SPI3
#endif

#ifdef PCBX9LITE
	#define LCD_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
  #define LCD_RCC_APB1Periph            RCC_APB1Periph_SPI3
  #define LCD_SPI_GPIO                  GPIOC
  #define LCD_MOSI_GPIO_PIN             GPIO_Pin_12 // PC.12
  #define LCD_MOSI_GPIO_PinSource       GPIO_PinSource12
  #define LCD_CLK_GPIO_PIN              GPIO_Pin_10 // PC.10
  #define LCD_CLK_GPIO_PinSource        GPIO_PinSource10
  #define LCD_A0_GPIO_PIN               GPIO_Pin_11 // PC.11
  #define LCD_NCS_GPIO                  GPIOD
  #define LCD_NCS_GPIO_PIN              GPIO_Pin_3 // PD.03
  #define LCD_RST_GPIO                  GPIOD
  #define LCD_RST_GPIO_PIN              GPIO_Pin_2 // PD.02
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF_SPI3
#endif

#ifdef REV9E
// TOP LCD
#define RCC_AHB1Periph_TOPLCD            RCC_AHB1Periph_GPIOG
#define GPIO_TOPLCD                      GPIOG
#define PIN_TOPLCD_LED                   GPIO_Pin_2 //PG.02
#define PIN_TOPLCD_CS1                   GPIO_Pin_3 //PG.03
#define PIN_TOPLCD_CS2                   GPIO_Pin_15 //PG.15
#define PIN_TOPLCD_WR		                 GPIO_Pin_4 //PG.04
#define PIN_TOPLCD_DATA                  GPIO_Pin_5 //PG.05
#endif

// Audio----I2S3
//#define CODEC_I2S_ADDRESS              0x4000380C
#define CODEC_I2S                       SPI3
#define CODEC_I2S_CLK                   RCC_APB1Periph_SPI3
#define CODEC_I2S_GPIO_AF               GPIO_AF_SPI3
#define CODEC_I2S_IRQ                   SPI3_IRQn
#define CODEC_I2S_GPIO_CLOCK            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC )//| RCC_AHB1Periph_GPIOE)
#define CODEC_I2S_WS_PIN                GPIO_Pin_15 //PA.15
#define CODEC_I2S_SCK_PIN               GPIO_Pin_10 //PC.10
#define CODEC_I2S_SD_PIN                GPIO_Pin_12 //PC.12
#define CODEC_I2S_MCK_PIN               GPIO_Pin_6  //DISABLED
#define CODEC_I2S_WS_PINSRC             GPIO_PinSource15
#define CODEC_I2S_SCK_PINSRC            GPIO_PinSource10
#define CODEC_I2S_SD_PINSRC             GPIO_PinSource12
#define CODEC_I2S_MCK_PINSRC            GPIO_PinSource6
#define CODEC_I2S_GPIO                  GPIOC
#define CODEC_I2S_MCK_GPIO              GPIOC
#define CODEC_I2S_WS_GPIO               GPIOA
#define CODEC_I2S_MUTE                  GPIO_Pin_11 //PC.11
//#define CODEC_I2S_FLT                  GPIO_Pin_  //IIR/FIR
//#define CODEC_I2S_MODE                 GPIO_Pin_  //Connect to GND=I2S_STANDARD

// Eeprom 5137
#ifdef PCB9XT
#define I2C_EE_GPIO                     GPIOB
#define I2C_EE_GPIO_CLK                 RCC_AHB1Periph_GPIOB
#define I2C_EE_SCL                      GPIO_Pin_10  //PB10
#define I2C_EE_SDA                      GPIO_Pin_11  //PB11
#else
#ifdef PCBXLITE
#define I2C_EE_GPIO                     GPIOB
#define I2C_EE_WP_GPIO                  GPIOD
#define I2C_EE_GPIO_CLK                 RCC_AHB1Periph_GPIOB
#define I2C_EE_SCL                      GPIO_Pin_8  //PB8
#define I2C_EE_SDA                      GPIO_Pin_9  //PB9
#define I2C_EE_WP                       GPIO_Pin_7  //PD9
#else
#ifdef PCBX9LITE
#define I2C_EE_GPIO                     GPIOB
#define I2C_EE_WP_GPIO                  GPIOB
#define I2C_EE_GPIO_CLK                 RCC_AHB1Periph_GPIOB
#define I2C_EE_SCL                      GPIO_Pin_8  //PB8
#define I2C_EE_SDA                      GPIO_Pin_9  //PB9
#define I2C_EE_WP                       GPIO_Pin_7  //PD7
#else
 #ifdef REV19
#define I2C_RCC_AHB1Periph              RCC_AHB1Periph_GPIOB
#define I2C_RCC_APB1Periph              RCC_APB1Periph_I2C1
#define I2C                             I2C1
#define I2C_GPIO                        GPIOB
#define I2C_GPIO_PIN_SCL                GPIO_Pin_8  // PB.08
#define I2C_GPIO_PIN_SDA                GPIO_Pin_9  // PB.09
#define I2C_GPIO_AF                     GPIO_AF_I2C1
#define I2C_GPIO_PinSource_SCL          GPIO_PinSource8
#define I2C_GPIO_PinSource_SDA          GPIO_PinSource9
#define I2C_SPEED                       400000

#define I2C_EE_GPIO                     GPIOB
#define I2C_EE_GPIO_CLK                 RCC_AHB1Periph_GPIOB
#define I2C_EE_SCL                      GPIO_Pin_8  //PB8
#define I2C_EE_SDA                      GPIO_Pin_9  //PB9
 #else
#define I2C_EE_GPIO                     GPIOB
#define I2C_EE_WP_GPIO                  GPIOB
#define I2C_EE_GPIO_CLK                 RCC_AHB1Periph_GPIOB
#define I2C_EE_SCL                      GPIO_Pin_6  //PB6
#define I2C_EE_SDA                      GPIO_Pin_7  //PB7
#define I2C_EE_WP                       GPIO_Pin_9  //PB9
 #endif
#endif
#endif
#endif

// SD---spi2
#define SPI_SD                          SPI2
#define GPIO_AF_SD                      GPIO_AF_SPI2
#define RCC_AHB1Periph_GPIO_CS          RCC_AHB1Periph_GPIOB
#define GPIO_SPI_SD                     GPIOB
#define GPIO_Pin_SPI_SD_CS              GPIO_Pin_12 //PB.12
#define GPIO_Pin_SPI_SD_SCK             GPIO_Pin_13 //PB.13
#define GPIO_Pin_SPI_SD_MISO            GPIO_Pin_14 //PB.14
#define GPIO_Pin_SPI_SD_MOSI            GPIO_Pin_15 //PB.15
#define GPIO_PinSource_CS               GPIO_PinSource12
#define GPIO_PinSource_SCK              GPIO_PinSource13
#define GPIO_PinSource_MISO             GPIO_PinSource14
#define GPIO_PinSource_MOSI             GPIO_PinSource15
#define RCC_APBPeriphClockCmd_SPI_SD    RCC_APB1PeriphClockCmd
#define RCC_APBPeriph_SPI_SD            RCC_APB1Periph_SPI2
#define RCC_AHBxPeriph_GPIO_CP          RCC_AHB1Periph_GPIOD

#ifndef REV19
 #define RCC_AHBxPeriph_GPIO_WP          RCC_AHB1Periph_GPIOD
 #define GPIO_Mode_WP                    GPIO_Mode_OUT//lock?
 #define GPIO_Mode_CP                    GPIO_Mode_IN //
 #define GPIO_Pin_WP                     GPIO_Pin_8  //PD.08
#endif

#ifndef PCB9XT
 #if defined(PCBXLITE) || defined(PCBX9LITE)
 #define GPIO_Pin_CP                     GPIO_Pin_10  //PD.10
 #else
 #define GPIO_Pin_CP                     GPIO_Pin_9  //PD.09
 #endif
 #define GPIO_CTL_SD                     GPIOD
#else
#define GPIO_Pin_CP                     GPIO_Pin_11  //PC.11
#define GPIO_CTL_SD                     GPIOC
#endif
// Audio----I2S3-----SPI3
#define CODEC_MCLK_DISABLED
#define CODEC_USE_INT
//#define CODEC_USE_DMA 
#define AudioFreq                       I2S_AudioFreq_44k 


#ifdef PCB9XT
#else
// EEPROM and CAT5137
//#define EE_M24C08                       /* Support the device: M24C08. */
//#define EE_M24C64_32                  /* Support the devices: M24C32 and M24C64 */
#define I2C_Speed                       200000
#define I2C_FLASH_PAGESIZE              64
#define I2C_EEPROM_ADDRESS              0xA2
#define I2C_CAT5137_ADDRESS             0x5C //0101110
#endif

//#if defined (EE_M24C08)
// #define I2C_FLASH_PAGESIZE             (16)
//#elif defined (EE_M24C64_32)
// #define I2C_FLASH_PAGESIZE             (32)
//#endif

//#ifdef EE_M24C64_32
// #define EEPROM_HW_ADDRESS              0xA0   /* E0 = E1 = E2 = 0 */
//#elif defined (EE_M24C08)
////#define EEPROM_Block0_ADDRESS            0xA0   /* 000*/
//#define EEPROM_Block1_ADDRESS           0xA2 /* E3 E2 E1= 001 */
////#define EEPROM_Block2_ADDRESS            0xA4 /* 010 */
////#define EEPROM_Block3_ADDRESS            0xA6 /* 011 */
//#endif 

//#define USE_CAT5137 /*Support the Volume Control*/

//#ifdef USE_CAT5137
//#define I2C_CAT5137_ADDRESS             0x5C//0101110
//#endif

// SD card----SPI2
// demo uses a command line option to define this (see Makefile):
// #define STM32_SD_USE_DMA
#ifdef STM32_SD_USE_DMA
// #warning "Information only: using DMA"
#pragma message "*** Using DMA ***"
#endif

/* set to 1 to provide a disk_ioctrl function even if not needed by the FatFs */
#define STM32_SD_DISK_IOCTRL_FORCE      (0)

#define CARD_SUPPLY_SWITCHABLE          (0) //0£¬power on not depend any IO
#define SOCKET_WP_CONNECTED             (0)
#define SOCKET_CP_CONNECTED             (0)

#if (PERI1_FREQUENCY<40000000)
//#define SPI_BaudRatePrescaler_SPI_SD    SPI_BaudRatePrescaler_2 // - for SPI1 and half-speed APB2: 30MHz/2 =15MHZ < 20MHZ
#define SPI_BaudRatePrescaler_SPI_SD    SPI_BaudRatePrescaler_2 // - for SPI2 and half-speed APB1: 15MHz/2 = 7.5MHZ < 20MHZ
#else
#define SPI_BaudRatePrescaler_SPI_SD    SPI_BaudRatePrescaler_2 // - for SPI1 and full-speed APB2: 60MHz/4 =15MHZ < 20MHZ
#endif

// Selectable
// DMA
#define DMA_Channel_SPI_SD_RX           DMA1_Channel2
#define DMA_Channel_SPI_SD_TX           DMA1_Channel3
#define DMA_FLAG_SPI_SD_TC_RX           DMA1_FLAG_TC2
#define DMA_FLAG_SPI_SD_TC_TX           DMA1_FLAG_TC3


#ifdef PCB9XT
#define PIN_STK_J1                      GPIO_Pin_0  //PC.00              
#define PIN_STK_J2                      GPIO_Pin_1  //PC.01
#define PIN_STK_J3                      GPIO_Pin_2  //PC.02
#define PIN_STK_J4                      GPIO_Pin_3  //PC.03
#define PIN_MVOLT                       GPIO_Pin_0  //PB.00
#define PIN_SW1		                      GPIO_Pin_6  //PA.06
#define PIN_SW2		                      GPIO_Pin_1  //PB.01
#define PIN_SW3		                      GPIO_Pin_4  //PC.04
  
#define RCC_AHB1Periph_GPIOADC          RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC

#define PIN_AUDIO_DAC1                  GPIO_Pin_5  //PA.05

#define RCC_AHB1Periph_GPIOPWR          RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD
#define GPIOPWR		                      GPIOC
#define PIN_PWR_STATUS                  GPIO_Pin_6  //PC.06
#define PIN_MCU_PWR                     GPIO_Pin_5  //PC.05

#define GPIOPWRINT                      GPIOC
#define GPIOPWREXT                      GPIOD
#define PIN_INT_RF_PWR                  GPIO_Pin_12	// PC12
#define PIN_EXT_RF_PWR                  GPIO_Pin_2  // PD2

// EEPROM---spi1
#define GPIO_Pin_SPI_EE_CS							GPIO_Pin_15	// PA.15
#define GPIO_Pin_SPI_EE_SCK							GPIO_Pin_3	// PB.03
#define GPIO_Pin_SPI_EE_MOSI						GPIO_Pin_5	// PB.05
#define GPIO_Pin_SPI_EE_MISO						GPIO_Pin_4	// PB.04

// Mega64 programming
#define GPIO_Pin_M64_RST								GPIO_Pin_10	// PC.10
#define GPIO_Pin_M64_SCK								GPIO_Pin_8	// PA.08
#define GPIO_Pin_M64_MOSI								GPIO_Pin_6	// PB.06
#define GPIO_Pin_M64_MISO								GPIO_Pin_7	// PB.07


#endif // PCB9XT


#if defined(PCBXLITE)
  
	#define RCC_AHB1Periph_GPIOADC        RCC_AHB1Periph_GPIOC
	
	#define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            RCC_APB1Periph_TIM5
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_3  // PA.03
  #define PWM_TIMER                     TIM5
  #define PWM_GPIO                      GPIOA
  #define PWM_GPIO_AF                   GPIO_AF_TIM5
  #define PWM_IRQHandler                TIM5_IRQHandler
  #define PWM_IRQn                      TIM5_IRQn
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_1  // PC.01
  #define ADC_GPIO_PIN_POT2             GPIO_Pin_2  // PC.02
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define STICKS_GPIOA_PINS             (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
  #define PWM_GPIOA_PINS                STICKS_GPIOA_PINS
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_POT1 | (ANALOGS_PWM_ENABLED() ? 0 : STICKS_GPIOA_PINS))
  #define ADC_GPIOB_PINS                ADC_GPIO_PIN_POT2
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              ADC_Channel_6
  #define ADC_CHANNEL_POT2              ADC_Channel_8
  #define ADC_CHANNEL_BATT              ADC_Channel_10

#define PIN_FLP_J1                      GPIO_Pin_1  //PC.01
#define PIN_FLP_J2                      GPIO_Pin_2  //PC.02
#define PIN_MVOLT                       GPIO_Pin_0  //PC.00  
#define PIN_STICK_RV						        GPIO_Pin_0  // PA.00
#define PIN_STICK_RH						        GPIO_Pin_1  // PA.01
#define PIN_STICK_LV        						GPIO_Pin_2  // PA.02
#define PIN_STICK_LH        						GPIO_Pin_3  // PA.03


#define PWR_PRESS_BUTTON	// power on a button, not a switch

  #define STATUS_LEDS
  #define LED_GREEN_GPIO                GPIOE
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_5  // PE.05
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_4  // PE.04
  #define LED_BLUE_GPIO                 GPIOE
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_6  // PE.06

// Internal Module
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define INTMODULE_RCC_APB1Periph      RCC_APB1Periph_TIM12
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1
  #define INTMODULE_PWR_GPIO            GPIOD
  #define INTMODULE_PWR_GPIO_PIN        GPIO_Pin_9  // PD.09
  #define INTMODULE_TX_GPIO             GPIOB
  #define INTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PB.06
  #define INTMODULE_RX_GPIO             GPIOB
  #define INTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PB.07
  #define INTMODULE_TX_GPIO_PinSource   GPIO_PinSource6
  #define INTMODULE_RX_GPIO_PinSource   GPIO_PinSource7
  #define INTMODULE_USART               USART1
  #define INTMODULE_TX_GPIO_AF          GPIO_AF_USART1
  #define INTMODULE_USART_IRQn          USART1_IRQn
  #define INTMODULE_DMA_STREAM          DMA2_Stream7
  #define INTMODULE_DMA_STREAM_IRQ      DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_FLAG_TC         DMA_IT_TCIF7
  #define INTMODULE_DMA_CHANNEL         DMA_Channel_4
  #define INTMODULE_TIMER               TIM12
  #define INTMODULE_TIMER_IRQn          TIM8_BRK_TIM12_IRQn
  #define INTMODULE_TIMER_IRQHandler    TIM8_BRK_TIM12_IRQHandler
	#define INTMODULE_TIMER_FREQ          (PeripheralSpeeds.Peri1_frequency * PeripheralSpeeds.Timer_mult1)
	#define INTMODULE_TIMER_SR_MASK				0x0203
	#define INTMODULE_USART_IRQHandler		USART1_IRQHandler
//	#define PIN_INTPPM_OUT                GPIO_Pin_6  //PB.06



// External Module
  #define EXTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define EXTMODULE_RCC_APB2Periph      RCC_APB2Periph_TIM8
  #define EXTMODULE_PWR_GPIO            GPIOD
  #define EXTMODULE_PWR_GPIO_PIN        GPIO_Pin_11 // PD.11
  #define EXTMODULE_TX_GPIO             GPIOC
  #define EXTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PC.06
  #define EXTMODULE_RX_GPIO             GPIOC
  #define EXTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PC.07
  #define EXTMODULE_TX_GPIO_PinSource   GPIO_PinSource6
  #define EXTMODULE_RX_GPIO_PinSource   GPIO_PinSource7
  #define EXTMODULE_USART               USART6
  #define EXTMODULE_USART_IRQn          USART6_IRQn
  #define EXTMODULE_TIMER               TIM8
  #define EXTMODULE_TX_GPIO_AF          GPIO_AF_TIM8 // TIM8_CH1
  #define EXTMODULE_TIMER_CC_IRQn       TIM8_CC_IRQn
  #define EXTMODULE_TIMER_CC_IRQHandler TIM8_CC_IRQHandler
  #define EXTMODULE_DMA_CHANNEL         DMA_Channel_7
  #define EXTMODULE_DMA_STREAM          DMA2_Stream1
  #define EXTMODULE_DMA_IRQn            DMA2_Stream1_IRQn
  #define EXTMODULE_DMA_IRQHandler      DMA2_Stream1_IRQHandler
  #define EXTMODULE_DMA_FLAG_TC         DMA_IT_TCIF1
//  #define EXTMODULE_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define EXTMODULE_TIMER_FREQ          (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2)
	#define EXTMODULE_TIMER_SR_MASK				0x1FFF
	#define EXTMODULE_USART_IRQHandler		USART6_IRQHandler

	#define PIN_EXTPPM_OUT                  GPIO_Pin_6  //PC.06
	#define PORT_EXTPPM                     PIN_PORTC

// No trainer
  #define TRAINER_RCC_AHB1Periph        0
  #define TRAINER_RCC_APB1Periph        0
// No serial port
  #define SERIAL_RCC_AHB1Periph         0
  #define SERIAL_RCC_APB1Periph         0

// Heartbeat
  #define HEARTBEAT_GPIO                GPIOD
  #define HEARTBEAT_GPIO_PIN            GPIO_Pin_15 // PD.15

  #define BACKLIGHT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOA
  #define BACKLIGHT_RCC_APB1Periph      0
  #define BACKLIGHT_RCC_APB2Periph      RCC_APB2Periph_TIM1
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM1
  #define BACKLIGHT_GPIO                GPIOA
  #define BACKLIGHT_GPIO_PIN            GPIO_Pin_8 // PA.8
  #define BACKLIGHT_GPIO_PinSource      GPIO_PinSource10
  #define BACKLIGHT_GPIO_AF             GPIO_AF_TIM1
  #define BACKLIGHT_CCMR2               TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 // Channel 3, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC3E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR3

// LCD
  #define LCD_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
  #define LCD_RCC_APB1Periph            RCC_APB1Periph_SPI3
  #define LCD_SPI_GPIO                  GPIOC
  #define LCD_MOSI_GPIO_PIN             GPIO_Pin_12 // PC.12
  #define LCD_MOSI_GPIO_PinSource       GPIO_PinSource12
  #define LCD_CLK_GPIO_PIN              GPIO_Pin_10 // PC.10
  #define LCD_CLK_GPIO_PinSource        GPIO_PinSource10
  #define LCD_A0_GPIO_PIN               GPIO_Pin_11 // PC.11
  #define LCD_NCS_GPIO                  GPIOD
  #define LCD_NCS_GPIO_PIN              GPIO_Pin_3  // PD.03
  #define LCD_RST_GPIO                  GPIOD
  #define LCD_RST_GPIO_PIN              GPIO_Pin_2  // PD.02
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF_SPI3

// I2C Bus: EEPROM and CAT5137 digital pot for volume control
  #define I2C_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD)
  #define I2C_SPI_GPIO                  GPIOB
  #define I2C_SDA_GPIO_PIN              GPIO_Pin_9  // PB.09
  #define I2C_SCL_GPIO_PIN              GPIO_Pin_8  // PB.08
  #define I2C_WP_GPIO                   GPIOD
  #define I2C_WP_GPIO_PIN               GPIO_Pin_7  // PD.07
  #define I2C_SCL_GPIO_PinSource        GPIO_PinSource8
  #define I2C_SDA_GPIO_PinSource        GPIO_PinSource9

// Haptic
  #define HAPTIC_PWM
  #define HAPTIC_RCC_AHB1Periph         RCC_AHB1Periph_GPIOB
  #define HAPTIC_RCC_APB1Periph         RCC_APB1Periph_TIM2
  #define HAPTIC_RCC_APB2Periph         0
  #define HAPTIC_GPIO_PinSource         GPIO_PinSource3
  #define HAPTIC_GPIO                   GPIOB
  #define HAPTIC_GPIO_PIN               GPIO_Pin_3  // PB.03
  #define HAPTIC_GPIO_AF                GPIO_AF_TIM2
  #define HAPTIC_TIMER                  TIM2
  #define HAPTIC_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR2
  #define HAPTIC_CCMR1                  TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2
  #define HAPTIC_CCER                   TIM_CCER_CC2E
  #define BACKLIGHT_BDTR                TIM_BDTR_MOE

// Bluetooth
	#define BT_RCC_AHB1Periph             (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_DMA1)
  #define BT_RCC_APB1Periph             RCC_APB1Periph_USART3
  #define BT_RCC_APB2Periph             0
  #define BT_EN_GPIO                    GPIOE
  #define BT_EN_GPIO_PIN	              GPIO_Pin_15 // PE.15
  #define BT_GPIO_TXRX                  GPIOB
  #define BT_TX_GPIO_PIN                GPIO_Pin_10 // PB.10
  #define BT_RX_GPIO_PIN                GPIO_Pin_11 // PB.11
  #define BT_TX_GPIO_PinSource          GPIO_PinSource10
  #define BT_RX_GPIO_PinSource          GPIO_PinSource11
  #define BT_GPIO_AF                    GPIO_AF_USART3
  #define BT_USART                      USART3
  #define BT_USART_IRQHandler           USART3_IRQHandler
  #define BT_USART_IRQn                 USART3_IRQn
  #define BT_DMA_Stream_RX              DMA1_Stream1
  #define BT_DMA_Channel_RX             DMA_Channel_4

// Power  
	#define PWR_SWITCH_GPIO               GPIOA
  #define PWR_SWITCH_GPIO_PIN           GPIO_Pin_7  // PA.07
  #define PWR_ON_GPIO                   GPIOE
  #define PWR_ON_GPIO_PIN               GPIO_Pin_9  // PE.09
	
	#define GPIOPWR		                    GPIOE
	#define PIN_MCU_PWR                   GPIO_Pin_9  // PE.09
	#define GPIOPWRSENSE                  GPIOA
	#define PIN_PWR_STATUS                GPIO_Pin_7  // PA.07

	#define GPIOPWRINT                      GPIOD
	#define PIN_INT_RF_PWR                  GPIO_Pin_9	// PD9
	#define GPIOPWREXT                      GPIOD
	#define PIN_EXT_RF_PWR                  GPIO_Pin_11  // PD.11
	#define GPIOPWRSPORT                    GPIOD
	#define PIN_SPORT_PWR                   GPIO_Pin_8   // PD.08


#endif // XLITE

#ifdef PCBX9LITE
  #define HAPTIC_PWM
  #define HAPTIC_RCC_AHB1Periph         RCC_AHB1Periph_GPIOB
  #define HAPTIC_RCC_APB1Periph         RCC_APB1Periph_TIM2
  #define HAPTIC_RCC_APB2Periph         0
  #define HAPTIC_GPIO_PinSource         GPIO_PinSource3
  #define HAPTIC_GPIO                   GPIOB
  #define HAPTIC_GPIO_PIN               GPIO_Pin_3  // PB.03
  #define HAPTIC_GPIO_AF                GPIO_AF_TIM2
  #define HAPTIC_TIMER                  TIM2
  #define HAPTIC_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define HAPTIC_COUNTER_REGISTER       CCR2
  #define HAPTIC_CCMR1                  TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2
  #define HAPTIC_CCER                   TIM_CCER_CC2E

  #define BACKLIGHT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOA
  #define BACKLIGHT_RCC_APB1Periph      0
  #define BACKLIGHT_RCC_APB2Periph      RCC_APB2Periph_TIM1
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM1
  #define BACKLIGHT_GPIO                GPIOA
  #define BACKLIGHT_GPIO_PIN            GPIO_Pin_10 // PA.10
  #define BACKLIGHT_GPIO_PinSource      GPIO_PinSource10
  #define BACKLIGHT_GPIO_AF             GPIO_AF_TIM1
  #define BACKLIGHT_CCMR2               TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 // Channel 3, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC3E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR3
  #define BACKLIGHT_BDTR                TIM_BDTR_MOE

  #define HEARTBEAT_GPIO                GPIOC
  #define HEARTBEAT_GPIO_PIN            GPIO_Pin_9 // PC.09

// Internal Module
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define INTMODULE_RCC_APB1Periph      RCC_APB1Periph_TIM12
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1
  #define INTMODULE_PWR_GPIO            GPIOD
  #define INTMODULE_PWR_GPIO_PIN        GPIO_Pin_9  // PD.09
  #define INTMODULE_TX_GPIO             GPIOB
  #define INTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PB.06
  #define INTMODULE_RX_GPIO             GPIOB
  #define INTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PB.07
  #define INTMODULE_TX_GPIO_PinSource   GPIO_PinSource6
  #define INTMODULE_RX_GPIO_PinSource   GPIO_PinSource7
  #define INTMODULE_USART               USART1
  #define INTMODULE_TX_GPIO_AF          GPIO_AF_USART1
  #define INTMODULE_USART_IRQn          USART1_IRQn
  #define INTMODULE_DMA_STREAM          DMA2_Stream7
  #define INTMODULE_DMA_STREAM_IRQ      DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_FLAG_TC         DMA_IT_TCIF7
  #define INTMODULE_DMA_CHANNEL         DMA_Channel_4
  #define INTMODULE_TIMER               TIM12
  #define INTMODULE_TIMER_IRQn          TIM8_BRK_TIM12_IRQn
  #define INTMODULE_TIMER_IRQHandler    TIM8_BRK_TIM12_IRQHandler
	#define INTMODULE_TIMER_FREQ          (PeripheralSpeeds.Peri1_frequency * PeripheralSpeeds.Timer_mult1)
	#define INTMODULE_TIMER_SR_MASK				0x0203
	#define INTMODULE_USART_IRQHandler		USART1_IRQHandler
//	#define PIN_INTPPM_OUT                GPIO_Pin_6  //PB.06



// External Module
  #define EXTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define EXTMODULE_RCC_APB2Periph      RCC_APB2Periph_TIM8
  #define EXTMODULE_PWR_GPIO            GPIOD
  #define EXTMODULE_PWR_GPIO_PIN        GPIO_Pin_11 // PD.11
  #define EXTMODULE_TX_GPIO             GPIOC
  #define EXTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PC.06
  #define EXTMODULE_RX_GPIO             GPIOC
  #define EXTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PC.07
  #define EXTMODULE_TX_GPIO_PinSource   GPIO_PinSource6
  #define EXTMODULE_RX_GPIO_PinSource   GPIO_PinSource7
  #define EXTMODULE_USART               USART6
  #define EXTMODULE_TX_GPIO_USART_AF    GPIO_AF_USART6
  #define EXTMODULE_USART_IRQn          USART6_IRQn
  #define EXTMODULE_DMA_STREAM          DMA2_Stream1
  #define EXTMODULE_DMA_IRQn            DMA2_Stream1_IRQn
  #define EXTMODULE_DMA_IRQHandler      DMA2_Stream1_IRQHandler
  #define EXTMODULE_DMA_FLAG_TC         DMA_IT_TCIF1
	#define EXTMODULE_DMA_CHANNEL         DMA_Channel_7
	#define EXTMODULE_TIMER               TIM8
  #define EXTMODULE_TX_GPIO_AF          GPIO_AF_TIM8 // TIM8_CH1
  #define EXTMODULE_TIMER_CC_IRQn       TIM8_CC_IRQn
  #define EXTMODULE_TIMER_CC_IRQHandler TIM8_CC_IRQHandler
  #define EXTMODULE_TIMER_FREQ          (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2)
	#define EXTMODULE_TIMER_SR_MASK				0x1FFF
	#define EXTMODULE_USART_IRQHandler		USART6_IRQHandler


#endif // X3

#ifdef REV19
// Internal Module
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define INTMODULE_RCC_APB1Periph      RCC_APB1Periph_TIM12
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1
  #define INTMODULE_PWR_GPIO            GPIOA
  #define INTMODULE_PWR_GPIO_PIN        GPIO_Pin_7  // PA.07
  #define INTMODULE_TX_GPIO             GPIOB
  #define INTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PB.06
  #define INTMODULE_RX_GPIO             GPIOB
  #define INTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PB.07
  #define INTMODULE_TX_GPIO_PinSource   GPIO_PinSource6
  #define INTMODULE_RX_GPIO_PinSource   GPIO_PinSource7
  #define INTMODULE_USART               USART1
  #define INTMODULE_TX_GPIO_AF          GPIO_AF_USART1
  #define INTMODULE_USART_IRQn          USART1_IRQn
  #define INTMODULE_DMA_STREAM          DMA2_Stream7
  #define INTMODULE_DMA_STREAM_IRQ      DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_FLAG_TC         DMA_IT_TCIF7
  #define INTMODULE_DMA_CHANNEL         DMA_Channel_4
  #define INTMODULE_TIMER               TIM12
  #define INTMODULE_TIMER_IRQn          TIM8_BRK_TIM12_IRQn
  #define INTMODULE_TIMER_IRQHandler    TIM8_BRK_TIM12_IRQHandler
	#define INTMODULE_TIMER_FREQ          (PeripheralSpeeds.Peri1_frequency * PeripheralSpeeds.Timer_mult1)
	#define INTMODULE_TIMER_SR_MASK				0x0203
	#define INTMODULE_USART_IRQHandler		USART1_IRQHandler
//	#define PIN_INTPPM_OUT                GPIO_Pin_6  //PB.06



// External Module
  #define EXTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define EXTMODULE_RCC_APB2Periph      RCC_APB2Periph_TIM8
  #define EXTMODULE_PWR_GPIO            GPIOD
  #define EXTMODULE_PWR_GPIO_PIN        GPIO_Pin_08 // PD.08
  #define EXTMODULE_TX_GPIO             GPIOC
  #define EXTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PC.06
  #define EXTMODULE_RX_GPIO             GPIOC
  #define EXTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PC.07
  #define EXTMODULE_TX_GPIO_PinSource   GPIO_PinSource6
  #define EXTMODULE_RX_GPIO_PinSource   GPIO_PinSource7
  #define EXTMODULE_USART               USART6
  #define EXTMODULE_TX_GPIO_USART_AF    GPIO_AF_USART6
  #define EXTMODULE_USART_IRQn          USART6_IRQn
  #define EXTMODULE_DMA_STREAM          DMA2_Stream1
  #define EXTMODULE_DMA_IRQn            DMA2_Stream1_IRQn
  #define EXTMODULE_DMA_IRQHandler      DMA2_Stream1_IRQHandler
  #define EXTMODULE_DMA_FLAG_TC         DMA_IT_TCIF1
	#define EXTMODULE_DMA_CHANNEL         DMA_Channel_7
	#define EXTMODULE_TIMER               TIM8
  #define EXTMODULE_TX_GPIO_AF          GPIO_AF_TIM8 // TIM8_CH1
  #define EXTMODULE_TIMER_CC_IRQn       TIM8_CC_IRQn
  #define EXTMODULE_TIMER_CC_IRQHandler TIM8_CC_IRQHandler
  #define EXTMODULE_TIMER_FREQ          (PeripheralSpeeds.Peri2_frequency * PeripheralSpeeds.Timer_mult2)
	#define EXTMODULE_TIMER_SR_MASK				0x1FFF
	#define EXTMODULE_USART_IRQHandler		USART6_IRQHandler


//	#define BT_RCC_AHB1Periph             (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
	#define BT_RCC_AHB1Periph             (RCC_AHB1Periph_GPIOB)
  #define BT_RCC_APB1Periph             RCC_APB1Periph_USART3
//  #define BT_RCC_APB2Periph             0
  #define BT_EN_GPIO                    GPIOB
  #define BT_EN_GPIO_PIN	              GPIO_Pin_02 // PB.02 (BOOT1)
  #define BT_GPIO_TXRX                  GPIOB
  #define BT_TX_GPIO_PIN                GPIO_Pin_10 // PB.10
  #define BT_RX_GPIO_PIN                GPIO_Pin_11 // PB.11
  #define BT_TX_GPIO_PinSource          GPIO_PinSource10
  #define BT_RX_GPIO_PinSource          GPIO_PinSource11
  #define BT_GPIO_AF                    GPIO_AF_USART3
  #define BT_USART                      USART3
  #define BT_USART_IRQHandler           USART3_IRQHandler
  #define BT_USART_IRQn                 USART3_IRQn
//  #define BT_DMA_Stream_RX              DMA1_Stream1
//  #define BT_DMA_Channel_RX             DMA_Channel_4

#endif // REV19


#endif // HAL
