/*
  AutonomousControl.ino - File that controls the robot autonomously
  
  Description:
  This sketch provides functions for IR receiving and transmitting, as well as line following.

  Created by: Your Name
  Date: 1/18/2024
  Version: 1.0
*/

void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat) {
    /*
     * Since we are in an interrupt context and do not want to miss the next interrupts 
     *  of the repeats coming soon, quickly save data and return to main loop
     */
    IRresults.address = aAddress;
    IRresults.command = aCommand;
    IRresults.isRepeat = isRepeat;
    // Let main function know that new data is available
    justWritten = true;
}
