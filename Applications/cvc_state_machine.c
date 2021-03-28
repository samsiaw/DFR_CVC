/*
 * cvc_state_machine.c
 *
 *  Created on: Feb 12, 2019
 *      Author: f002bc7
 */

/* Includes ------------------------------------------------------------------------*/

#include "cvc_state_machine.h"


/* Private Variables ------------------------------------------------------------------------*/

volatile cvc_state_t cvc_state = PRECHARGE;
static cvc_fault_status_t cvc_fault = CVC_OK;
static cvc_error_code_t cvc_error = NONE;

static int Dash_BRB_Pressed = 0;
static int TSMS_closed = 1;
static int voltage_check_timer = 0;
static uint8_t voltage_check_timer_started = 0;
static int precharge_timer = 0;
static uint8_t precharge_90p_voltage = 0;
static uint8_t precharge_timer_started = 0;
static uint8_t precharge_complete = 0;
static int buzzer_timer = 0;
static uint8_t buzzer_timer_started = 0;

static uint8_t push_button = 0;
static uint8_t ready_to_drive = 0;

static float pack_voltage;
static float bus_voltage;

static uint8_t log_disable_prev = 0xFF;
static uint8_t log_disable_temp = 0xFF;
static uint8_t log_disable_counter = 0;

static int temperature_value = 0;

/* Pins settings at PRECHARGE mode */
static uint8_t error_pin = HI;
static uint8_t warn_pin = HI;
static uint8_t second_Airs_pin = OPEN;


#define true 1;
#define false 0;
/* Vars to set:
 *
 * Already set:
 * Airs Power on/ off
 * Charge enabled
 * buzzer_timer
 * bus voltage/ power amt
 * Temperature
 * RTD
 *
 */
static uint8_t airs_power = LO; // Confirm initial state from ben
static uint8_t charge_enabled = LO;

void state_machine()
{
	// add any checks that must be done in all states here

	switch(cvc_state)
	{
	case PRECHARGE: //mostly unchanged (review required)
		// error_pin, warn_pin, 2nd_airs_pin
		set_pins(HI, HI, OPEN);

		/* get batt & bamo voltages */
		xSemaphoreTake(CAN_Inputs_Vector_Mutex, portMAX_DELAY);

		pack_voltage = (float) 50.0;
		bus_voltage = (float) ((int)CAN_inputs[DC_BUS_VOLTAGE])/10.0;	//*96.0f/3600.0f;

		xSemaphoreGive(CAN_Inputs_Vector_Mutex);

		/* wait for 90% precharge */
		if (bus_voltage >= pack_voltage * 0.95f) //changed to 95%
		{
			precharge_90p_voltage = 1;
		}

		/* start precharge timer and wait */
		if (precharge_90p_voltage && !precharge_timer_started)
		{
			precharge_timer = PRE_CHARGE_TIMER_LOAD;
			precharge_timer_started = 1;
		}
		else if (precharge_90p_voltage && precharge_timer_started)
		{
			precharge_timer--;
		}

		if (precharge_timer_started && precharge_timer == 0)
		{
			precharge_complete = 1;
		}

		/* send Bamocar set message when precharge complete to close second AIR */
		if (precharge_complete)
		{

			cvc_state = DRIVE;
			precharge_90p_voltage = 0;
			precharge_timer_started = 0;
			precharge_complete = 0;
		}
		else
		{
			cvc_state = PRECHARGE;
		}

		/* TODO: alert driver AIRs are closed (MOTEC alert) */

		break;
	

	case DRIVE: //mostly unchanged (review required)
		set_pins(HI, HI, CLOSE);

		/* set SPI outputs */

		cvc_state = DRIVE;

		pack_voltage = (float) 50.0;
		bus_voltage = (float) ((int)CAN_inputs[DC_BUS_VOLTAGE])/10.0;

		/* check for open Tractive System Master Switch */
		if (bus_voltage <= 0.95 * pack_voltage) //changed to 0.95
		{
			cvc_state = PRECHARGE;
		}

		break; 


	case VOLTAGE_CHECK:
		set_pins(LO, HI, OPEN);

		//TODO: get voltage value
		//TODO: if voltage timer runs out, voltage check failed -> error (voltage_error)
		break;

	case PRECHARGE_WAIT:
		set_pins(HI, HI, OPEN);

		if (cvc_fault == CVC_WARNING){
			cvc_state = CVC_WARN;
		}
		else if (cvc_error != NONE ){
			cvc_state = CVC_ERROR;
			cvc_error = NONE; //TODO: -> SET TO STH OTHER THAN NONE
		}
		else if (get_airs_power_status() == HI){
			cvc_state = PRECHARGE;
		}

		break;

	case CVC_WARN:
		set_pins(HI, LO, OPEN);

		if (cvc_error != NONE){
			cvc_state = CVC_ERROR;
		}
		break;

	case CVC_ERROR:
		set_pins(LO, HI, OPEN);

		if (get_temperature() < TEMPERATURE_THRESHOLD){
			cvc_state = CVC_ERROR;
			cvc_error = NONE; //TODO: -> SET TO STH OTHER THAN NONE
		}
		break;

	case RTD: //READY TO DRIVE
		set_pins(HI, HI, CLOSE);

		if (cvc_fault == CVC_WARNING){
			cvc_state = CVC_WARN;
		}
		else if (cvc_error != NONE){
			cvc_state = CVC_ERROR;
			cvc_error = NONE; //TODO: -> SET TO STH OTHER THAN NONE
		}
		else if (ready_to_drive == HI){
			cvc_state = BUZZER;
		}
		else if (charge_enabled() == HI){
			cvc_state = CHARGING;
		}
		else if (get_airs_power_status() == LO){
			cvc_state = PRECHARGE_WAIT;
		}

		break;

	case BUZZER:
		set_pins(HI, HI, CLOSE); 

		//TODO: set buzzer timer

		if (cvc_fault == CVC_WARNING){
			cvc_state = CVC_WARN;
		}
		else if (cvc_error != NONE){
			cvc_state = CVC_ERROR;
			cvc_error = NONE; //TODO: -> SET TO STH OTHER THAN NONE
		}
		else if (get_airs_power_status() == LO){
			cvc_state = PRECHARGE_WAIT;

		}
		else if (charge_enabled() == HI){
			cvc_error = CHARGE_ERROR;
		}
		break;

	case CHARGING:
		set_pins(HI, HI, CLOSE);

		if (cvc_fault == CVC_WARNING){
			cvc_state = CVC_WARN;
		}
		else if (cvc_error != NONE){
			cvc_error = NONE; //TODO: SET TO STH OTHER THAN NONE
			cvc_state = CVC_ERROR;
		}
		else if (charge_enabled() == LO){
			cvc_error = NONE; //TODO: -> SET TO STH OTHER THAN NONE
			cvc_state = CVC_ERROR;
		}
		else if (is_charge_done()){ //TODO: via CAN 

		}
		else if (get_airs_power_status() == LO){
			cvc_state = PRECHARGE_WAIT;
		}
	default:
		break;
	}
}


void safety_monitor(void)
{
	//monitor_bamocar()
	//monitor_bms()
}


void cvc_error_handler(cvc_fault_status_t fault, cvc_error_code_t error)
{
	// TODO: account for pre-scheduler errors (init fault handler, probably just infinite loop)

	BSP_LED_On(LED_RED);
	cvc_fault = fault;
	cvc_error = error;
}

void init_fault_handler(void)
{
	BSP_LED_On(LED_RED);
	while(1);
}

int get_temperature(void){ //TODO:
	return 0;
}

int is_charge_done(void){//TODO:
	return false; 
}

int get_airs_power_status(void){
	return LO; //OR HI;
}

int charge_enabled(void){ //TODO: Is charge enabled?
	return false;
}

int enable_charge(void){

}

void set_pins(error_pin, warn_pin, second_airs){

}