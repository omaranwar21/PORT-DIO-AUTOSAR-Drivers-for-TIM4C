/******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port.c
 *
 * Description: Source file for TM4C123GH6PM Microcontroller - Port Driver.
 *
 * Author: Omar Anwar
 ******************************************************************************/

#include "Port.h"
#include "Port_Regs.h"
#include "tm4c123gh6pm_registers.h"

#if (PORT_DEV_ERROR_DETECT == STD_ON)

#include "Det.h"
/* AUTOSAR Version checking between Det and Dio Modules */
#if ((DET_AR_MAJOR_VERSION != PORT_AR_RELEASE_MAJOR_VERSION)\
		|| (DET_AR_MINOR_VERSION  != PORT_AR_RELEASE_MINOR_VERSION)\
		|| (DET_AR_PATCH_VERSION  != PORT_AR_RELEASE_PATCH_VERSION))
#error "The AR version of Det.h does not match the expected version"
#endif

#endif

STATIC const Port_ConfigPin * Port_configPtr = NULL_PTR;
STATIC uint8 Port_Status = PORT_NOT_INITIALIZED;
/************************************************************************************
 * Service Name: Port_SetupGpioPin
 * Sync/Async: Synchronous
 * Reentrancy: reentrant
 * Parameters (in): ConfigPtr - Pointer to post-build configuration data
 * Parameters (inout): None
 * Parameters (out): None
 * Return value: None
 * Description: Function to Setup the pin configuration:
 *              - Setup the pin as Digital GPIO pin
 *              - Setup the direction of the GPIO pin
 *              - Provide initial value for o/p pin
 *              - Setup the internal resistor for i/p pin
 ************************************************************************************/
void Port_Init(const Port_ConfigType* ConfigPtr)
{

#if (PORT_DEV_ERROR_DETECT == STD_ON)
	/* check if the input configuration pointer is not a NULL_PTR */
	if (NULL_PTR == ConfigPtr)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_INIT_SID,
				PORT_E_PARAM_CONFIG);
	}
	else
#endif
	{
		/*
		 * Set the module state to initialized and point to the PB configuration structure using a global pointer.
		 * This global pointer is global to be used by other functions to read the PB configuration structures
		 */
		Port_Status    = PORT_INITIALIZED;
		/* address of the first pin structure --> Pin[0] */
		Port_configPtr = ConfigPtr->Pin;
	}

	/* point to the required Port Registers base address */
	volatile uint32 * PortGpio_Ptr = NULL_PTR;

	volatile uint32 delay = 0;
	volatile Port_PinType pinIndex = PORT_PIN0_ID;

	for (pinIndex = PORT_PIN0_ID; pinIndex < PORT_CONFIGURED_PINS; ++pinIndex)
	{
		switch (Port_configPtr[pinIndex].port_num)
		{
		case PORT_PORTA_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTB_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTC_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTD_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTE_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTF_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTA Base Address */
			break;
		default:
			break;
		}
		/* Enable clock for PORT and allow time for clock to start*/
		SYSCTL_REGCGC2_REG |= (1<<ConfigPtr->Pin[pinIndex].port_num);
		delay = SYSCTL_REGCGC2_REG;

		if( ((Port_configPtr[pinIndex].port_num == PORT_PORTD_ID) && (Port_configPtr[pinIndex].pin_num == PORT_PIN7_ID)) /* PD7 */
				|| ((Port_configPtr[pinIndex].port_num == PORT_PORTF_ID) && (Port_configPtr[pinIndex].pin_num == PORT_PIN0_ID)) ) /* PF0 */
		{
			/* Unlock the GPIOCR register */
			*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_LOCK_REG_OFFSET) = 0x4C4F434B;

			/* Set the corresponding bit in GPIOCR register to allow changes on this pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_COMMIT_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
		}
		else if( (Port_configPtr[pinIndex].port_num == PORT_PORTC_ID) && (Port_configPtr[pinIndex].pin_num <= PORT_PIN3_ID)) /* PC0 to PC3 */
		{
			/* Do Nothing ...  this is the JTAG pins */
			continue;
		}
		else
		{
			/* Do Nothing ... No need to unlock the commit register for this pin */
		}

		if (Port_configPtr[pinIndex].mode == PORT_PIN_MODE_DIO)
		{
			/* Steps:
			 *	1. Disable Analog Functionality.
			 * 	2. Disable alternative function.
			 * 	3. Clear the PMCx bits for this pin.
			 * 	4. Enable Digital Functionality.
			 */

			/* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

			/* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

			/* Clear the PMCx bits for this pin */
			*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_configPtr[pinIndex].pin_num * 4));

			/* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
		}
		else if (Port_configPtr[pinIndex].mode == PORT_PIN_MODE_ADC)
		{
			/* Steps:
			 * 	1. Disable Digital Functionality.
			 * 	2. Disable alternative function.
			 * 	3. Clear the PMCx bits for this pin.
			 *	4. Enable Analog Functionality.
			 */

			/* Clear the corresponding bit in the GPIODEN register to disable digital functionality on this pin */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

			/* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

			/* Clear the PMCx bits for this pin */
			*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_configPtr[pinIndex].pin_num * 4));

			/* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
		}
		else /* Another mode */
		{
			/* Steps:
			 *	1. Disable Analog Functionality.
			 * 	2. Enable alternative function.
			 * 	3. Write alternative function ID in the PMCx bits for this pin.
			 * 	4. Enable Digital Functionality.
			 */

			/* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

			/* Enable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

			/* Write alternative function ID in the PMCx bits for this pin */
			*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (Port_configPtr[pinIndex].mode & 0x0000000F << (Port_configPtr[pinIndex].pin_num * 4));

			/* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
		}

		if(Port_configPtr[pinIndex].direction == PORT_PIN_OUT)
		{
			/* Set the corresponding bit in the GPIODIR register to configure it as output pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

			if(Port_configPtr[pinIndex].initial_value == PORT_PIN_LEVEL_LOW)
			{
				/* Clear the corresponding bit in the GPIODATA register to provide initial value 0 */
				CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DATA_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
			}
			else
			{
				/* Set the corresponding bit in the GPIODATA register to provide initial value 1 */
				SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DATA_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
			}
		}
		else if(Port_configPtr[pinIndex].direction == PORT_PIN_IN)
		{
			/* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

			if(Port_configPtr[pinIndex].resistor == PULL_UP)
			{
				/* Set the corresponding bit in the GPIOPUR register to enable the internal pull up pin */
				SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_UP_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
			}
			else if(Port_configPtr[pinIndex].resistor == PULL_DOWN)
			{
				/* Set the corresponding bit in the GPIOPDR register to enable the internal pull down pin */
				SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_DOWN_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
			}
			else
			{
				/* Clear the corresponding bit in the GPIOPUR register to disable the internal pull up pin */
				CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_UP_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

				/* Clear the corresponding bit in the GPIOPDR register to disable the internal pull down pin */
				CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_PULL_DOWN_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);
			}
		}
		else
		{
			/* Do Nothing */
		}
	}
}


/************************************************************************************
 * Service Name: Port_SetPinDirection
 * Service ID[hex]: 0x01
 * Sync/Async: Synchronous
 * Reentrancy: reentrant
 * Parameters (in): Pin - Port Pin ID number , Direction - Port Pin Direction
 * Parameters (inout): None
 * Parameters (out): None
 * Return value: None
 * Description: Function to Sets the port pin direction.
 ************************************************************************************/
#if (PORT_SET_PIN_DIRECTION_API == STD_ON)
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction){

	uint8 error = FALSE;

	/* point to the required Port Registers base address */
	volatile uint32 * PortGpio_Ptr = NULL_PTR;

#if (PORT_DEV_ERROR_DETECT == STD_ON)

	/* check if the pin ID valid or not */
	if ( Pin >= PORT_CONFIGURED_PINS)
	{
		Det_ReportError(PORT_MODULE_ID,
				PORT_INSTANCE_ID,
				PORT_SET_PIN_DIRECTION_SID,
				PORT_E_PARAM_PIN);
		error = TRUE;
	}
	else
	{
		/* Do Nothing */
	}

	/* check if the pin direction is changeable or not */
	if (Port_configPtr[Pin].pinDirection_changeable == PORT_NOT_CHANGEABLE) {
		Det_ReportError(PORT_MODULE_ID,
				PORT_INSTANCE_ID,
				PORT_SET_PIN_DIRECTION_SID,
				PORT_E_DIRECTION_UNCHANGEABLE);
		error = TRUE;
	}
	else
	{
		/* Do Nothing */
	}

	/* check if the port initialized or not */
	if (Port_Status == PORT_NOT_INITIALIZED) {
		Det_ReportError(PORT_MODULE_ID,
				PORT_INSTANCE_ID,
				PORT_SET_PIN_DIRECTION_SID,
				PORT_E_UNINIT);
		error = TRUE;
	}
	else
	{
		/* Do Nothing */
	}
#endif

	if (FALSE == error) {

		switch (Port_configPtr[Pin].port_num)
		{
		case PORT_PORTA_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTB_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTC_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTD_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTE_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTF_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTA Base Address */
			break;
		default:
			break;
		}

		if(PORT_PIN_OUT == Direction)
		{
			/* Set the corresponding bit in the GPIODIR register to configure it as output pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_configPtr[Pin].pin_num);

		}
		else if(PORT_PIN_IN == Direction)
		{
			/* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_configPtr[Pin].pin_num);

		}
		else
		{
			/* Do Nothing */
		}
	}
}

#endif


/************************************************************************************
 * Service Name: Port_RefreshPortDirection
 * Service ID[hex]: 0x02
 * Sync/Async: Synchronous
 * Reentrancy: Non reentrant
 * Parameters (in): None
 * Parameters (inout): None
 * Parameters (out): None
 * Return value: None
 * Description: Refreshes port direction.
 ************************************************************************************/
void Port_RefreshPortDirection(void){

	uint8 error = FALSE;
	volatile uint32 * PortGpio_Ptr = NULL_PTR;
	volatile Port_PinType pinIndex = PORT_PIN0_ID;


#if (PORT_DEV_ERROR_DETECT == STD_ON)

	/* check if the port initialized or not */
	if (Port_Status == PORT_NOT_INITIALIZED)
	{
		Det_ReportError(PORT_MODULE_ID,
				PORT_INSTANCE_ID,
				PORT_REFRESH_PORT_DIRECTION_SID,
				PORT_E_UNINIT);
		error = TRUE;
	}
	else
	{
		/* Do Nothing */
	}
#endif


	if (FALSE == error)
	{
		for (pinIndex = PORT_PIN0_ID; pinIndex < PORT_CONFIGURED_PINS; ++pinIndex)
		{
			switch (Port_configPtr[pinIndex].port_num)
			{
			case PORT_PORTA_ID:
				PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
				break;
			case PORT_PORTB_ID:
				PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTA Base Address */
				break;
			case PORT_PORTC_ID:
				PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTA Base Address */
				break;
			case PORT_PORTD_ID:
				PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTA Base Address */
				break;
			case PORT_PORTE_ID:
				PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTA Base Address */
				break;
			case PORT_PORTF_ID:
				PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTA Base Address */
				break;
			default:
				break;
			}

			if (PORT_CHANGEABLE == Port_configPtr[pinIndex].pinDirection_changeable) {

				if(PORT_PIN_OUT == Port_configPtr[pinIndex].direction)
				{
					/* Set the corresponding bit in the GPIODIR register to configure it as output pin */
					SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

				}
				else if(PORT_PIN_IN == Port_configPtr[pinIndex].direction)
				{
					/* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
					CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIR_REG_OFFSET) , Port_configPtr[pinIndex].pin_num);

				}
				else
				{
					/* Do Nothing */
				}
			}
			else
			{
				/* Do Nothing */
			}

		}
	}
}


/************************************************************************************
 * Service Name: Port_GetVersionInfo
 * Service ID[hex]: 0x03
 * Sync/Async: Synchronous
 * Reentrancy: Non reentrant
 * Parameters (in): None
 * Parameters (inout): None
 * Parameters (out): versioninfo - Pointer to where to store the version information of this module.
 * Return value: None
 * Description: Returns the version information of this module.
 ************************************************************************************/
#if (PORT_VERSION_INFO_API == STD_ON)
void Port_GetVersionInfo(Std_VersionInfoType* versioninfo){
#if (PORT_DEV_ERROR_DETECT == STD_ON)
	/* Check if input pointer is not Null pointer */
	if(NULL_PTR == versioninfo)
	{
		/* Report to DET  */
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID,
				PORT_GET_VERSION_INFO_SID, PORT_E_PARAM_POINTER);
	}
	else
#endif /* (PORT_DEV_ERROR_DETECT == STD_ON) */
	{
		/* Copy the vendor Id */
		versioninfo->vendorID = (uint16)PORT_VENDOR_ID;
		/* Copy the module Id */
		versioninfo->moduleID = (uint16)PORT_MODULE_ID;
		/* Copy Software Major Version */
		versioninfo->sw_major_version = (uint8)PORT_SW_MAJOR_VERSION;
		/* Copy Software Minor Version */
		versioninfo->sw_minor_version = (uint8)PORT_SW_MINOR_VERSION;
		/* Copy Software Patch Version */
		versioninfo->sw_patch_version = (uint8)PORT_SW_PATCH_VERSION;
	}
}
#endif


/************************************************************************************
 * Service Name: Port_SetPinMode
 * Service ID[hex]: 0x04
 * Sync/Async: Synchronous
 * Reentrancy: reentrant
 * Parameters (in): Pin - Port Pin ID number, Mode - New Port Pin mode to be set on port pin
 * Parameters (inout): None
 * Parameters (out): None
 * Return value: None
 * Description: Sets the port pin mode.
 ************************************************************************************/
#if (PORT_SET_PIN_MODE_API == STD_ON)
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode){

	uint8 error = FALSE;

	/* point to the required Port Registers base address */
	volatile uint32 * PortGpio_Ptr = NULL_PTR;

#if (PORT_DEV_ERROR_DETECT == STD_ON)

	/* check if the pin ID valid or not */
	if ( Pin >= PORT_CONFIGURED_PINS)
	{
		Det_ReportError(PORT_MODULE_ID,
				PORT_INSTANCE_ID,
				PORT_SET_PIN_MODE_SID,
				PORT_E_PARAM_PIN);
		error = TRUE;
	}
	else
	{
		/* Do Nothing */
	}

	/* check if the pin Mode is changeable or not */
	if (Port_configPtr[Pin].pinMode_changeable == PORT_NOT_CHANGEABLE) {
		Det_ReportError(PORT_MODULE_ID,
				PORT_INSTANCE_ID,
				PORT_SET_PIN_MODE_SID,
				PORT_E_MODE_UNCHANGEABLE);
		error = TRUE;
	}
	else
	{
		/* Do Nothing */
	}

	/* check if the port initialized or not */
	if (Port_Status == PORT_NOT_INITIALIZED) {
		Det_ReportError(PORT_MODULE_ID,
				PORT_INSTANCE_ID,
				PORT_SET_PIN_MODE_SID,
				PORT_E_UNINIT);
		error = TRUE;
	}
	else
	{
		/* Do Nothing */
	}
#endif

	if (FALSE == error) {

		switch (Port_configPtr[Pin].port_num)
		{
		case PORT_PORTA_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTB_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTC_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTD_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTE_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTA Base Address */
			break;
		case PORT_PORTF_ID:
			PortGpio_Ptr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTA Base Address */
			break;
		default:
			break;
		}

		if (PORT_PIN_MODE_DIO == Mode)
		{
			/* Steps:
			 *	1. Disable Analog Functionality.
			 * 	2. Disable alternative function.
			 * 	3. Clear the PMCx bits for this pin.
			 * 	4. Enable Digital Functionality.
			 */

			/* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_configPtr[Pin].pin_num);

			/* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_configPtr[Pin].pin_num);

			/* Clear the PMCx bits for this pin */
			*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_configPtr[Pin].pin_num * 4));

			/* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_configPtr[Pin].pin_num);
		}
		else if (PORT_PIN_MODE_ADC == Mode)
		{
			/* Steps:
			 * 	1. Disable Digital Functionality.
			 * 	2. Disable alternative function.
			 * 	3. Clear the PMCx bits for this pin.
			 *	4. Enable Analog Functionality.
			 */

			/* Clear the corresponding bit in the GPIODEN register to disable digital functionality on this pin */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_configPtr[Pin].pin_num);

			/* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_configPtr[Pin].pin_num);

			/* Clear the PMCx bits for this pin */
			*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_configPtr[Pin].pin_num * 4));

			/* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_configPtr[Pin].pin_num);
		}
		else /* Another mode */
		{
			/* Steps:
			 *	1. Disable Analog Functionality.
			 * 	2. Enable alternative function.
			 * 	3. Write alternative function ID in the PMCx bits for this pin.
			 * 	4. Enable Digital Functionality.
			 */

			/* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
			CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_configPtr[Pin].pin_num);

			/* Enable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_ALT_FUNC_REG_OFFSET) , Port_configPtr[Pin].pin_num);

			/* Write alternative function ID in the PMCx bits for this pin */
			*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_CTL_REG_OFFSET) |= (Port_configPtr[Pin].mode & 0x0000000F << (Port_configPtr[Pin].pin_num * 4));

			/* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
			SET_BIT(*(volatile uint32 *)((volatile uint8 *)PortGpio_Ptr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_configPtr[Pin].pin_num);
		}
	}

}
#endif
