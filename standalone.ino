// ##################################################
// ##     Things to do when running standalone     ##
// ##################################################

void standaloneRun (void) {
  if (standalone==false)
    if (openhabTimeElapsed > heartbeat_timeout)
      standalone = true;

  if (standalone==true) // We lost connection with openhab, we are running on our own.
  {
    // Check if there was a button pressed
    if (oldButton != myButton)
    {
      for (uint8_t index = 0; index < 8; index++)
      {
        if (bitRead(myButton,index) != bitRead(oldButton,index))
        {
          // Switch 1 near door
          if (index == 0)
          {
            digitalWrite(RelayStart + 0,bitRead(myButton,index));  // Ceiling LED 1
          }
          // Switch 2 near door
          if (index == 1)
          {
            digitalWrite(RelayStart + 1,bitRead(myButton,index));  // Ceiling LED 2
            digitalWrite(RelayStart + 2,bitRead(myButton,index));  // Ceiling LED 3
          }
          // Switch 1 near mirror
          if (index == 2)
          {
            digitalWrite(RelayStart + 4,bitRead(myButton,index));  // Fan 1
            digitalWrite(RelayStart + 5,bitRead(myButton,index));  // Fan 2
          }
          // Switch 2 near mirror
          if (index == 3)
          {
            digitalWrite(RelayStart + 6,bitRead(myButton,index));  // Light above mirror
          }
        }
      }
      
      oldButton = myButton;  
    }

    // handle DHT22 data
    if (ReadDHT == true)
    {
      
      ReadDHT = false;
    }
  }
}
