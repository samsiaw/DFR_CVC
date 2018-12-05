## DFR_CVC

### How to use this branch:
The purpose of this branch is to test the RTOS implementation of CAN messaging. The relevant functions and defines are found in `cvc_can.c` and `cvc_can.h`. 
This code was tested using a CAN bus made up of two NUCLEO-F767ZI boards with WaveShare CAN shields. 
#### To use:
  * One one computer, in `cvc_can.h`:
    * Uncomment `SENDER_` macro  
    * Set desired transmission frequency using `CAN_Tx_FREQ` 
    * Set desired test length using `TEST_DURATION`
    * Build project
    * Flash project to one NUCLEO-F767ZI board
  * On another computer, in `cvc_can.h`:
    * Comment `SENDER_` macro
    * Build project
    * Flash project to second NUCLEO-F767ZI board
  * Run receiver nucleo board in debug mode from one computer
  * Run sender nucleo board in debug mode from other computer
  * After specified duration, check that `Tx_msg_count` in sender code `cvc_can.c` and `Rx_msg_count` in receiver code are the same
