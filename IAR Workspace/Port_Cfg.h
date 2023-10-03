 /******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port_Cfg.h
 *
 * Description: Pre-Compile Configuration Header file for TM4C123GH6PM Microcontroller - Port Driver
 *
 * Author: Omar Anwar
 ******************************************************************************/

#ifndef PORT_CFG_H
#define PORT_CFG_H

/*
 * Module Version 1.0.0
 */
#define PORT_CFG_SW_MAJOR_VERSION                       (1U)
#define PORT_CFG_SW_MINOR_VERSION                       (0U)
#define PORT_CFG_SW_PATCH_VERSION                       (0U)

/*
 * AUTOSAR Version 4.0.3
 */
#define PORT_CFG_AR_RELEASE_MAJOR_VERSION               (4U)
#define PORT_CFG_AR_RELEASE_MINOR_VERSION               (0U)
#define PORT_CFG_AR_RELEASE_PATCH_VERSION               (3U)

/* Pre-compile option for Development Error Detect */
#define PORT_DEV_ERROR_DETECT                           (STD_ON)

/* Pre-compile option for Version Info API */
#define PORT_VERSION_INFO_API                           (STD_OFF)

/* Pre-compile option for presence of Port_SetPinDirection API */
#define PORT_SET_PIN_DIRECTION_API						(STD_ON)

/* Pre-compile option for presence of Port_SetPinMode API */
#define PORT_SET_PIN_MODE_API							(STD_ON)

/* Number of PINS */
#define PORT_CONFIGURED_PINS							(43U)

/* Ports ID */
#define PORT_PORTA_ID									(0U)
#define PORT_PORTB_ID									(1U)
#define PORT_PORTC_ID									(2U)
#define PORT_PORTD_ID									(3U)
#define PORT_PORTE_ID									(4U)
#define PORT_PORTF_ID									(5U)

/* Pins ID */
#define PORT_PIN0_ID                	                (0U)
#define PORT_PIN1_ID                	                (1U)
#define PORT_PIN2_ID                	                (2U)
#define PORT_PIN3_ID                	                (3U)
#define PORT_PIN4_ID                	                (4U)
#define PORT_PIN5_ID                	                (5U)
#define PORT_PIN6_ID                	                (6U)
#define PORT_PIN7_ID                	                (7U)

#endif /* PORT_CFG_H */
