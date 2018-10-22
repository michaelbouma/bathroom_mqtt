// ##################################################
// ##                 RelayInit                    ##
// ##################################################
void relayInit()
{
  // Initialize the relays
  for (uint8_t i = 0; i < 8; i++){
    pinMode(RelayStart + i,OUTPUT);     // Set ports as output
    digitalWrite(RelayStart + i,HIGH);  // switch off all relays, ports are active LOW
  }    
}

// ##################################################
// ##                  SwitchInit                  ##
// ##################################################
void switchInit()
{
  // Initialize the relays
  for (uint8_t i = 0; i < 8; i++){
    pinMode(InputStart + i,INPUT_PULLUP);      // Initialize inputs
  }    
}
  
// ##################################################
// ##                 SwitchRelay                  ##
// ##################################################
void switchRelay() {  
  if (oldRelay != myRelay)
  {
    for (uint8_t i = 0; i < 8; i++)
      digitalWrite(RelayStart + i,bitRead(myRelay,i));  // Set output
    oldRelay = myRelay;  
  }
}
// ##################################################
// ##                 ReadInputs                   ##
// ##################################################
void readInputs() {  
  static byte tempstate[NUMBUTTONS];
  byte index;
  for (index = 0; index < NUMBUTTONS; index++) { 
    byte n = digitalRead(InputStart + index);     // Read the ports
    if (n != bitRead(myButton,index))             // If port has changed publish the data
    {
      if (n==0)
      {
        tempstate[index]--;
        if (tempstate[index] <= 0 )
        {
          bitWrite(myButton,index,0);
          sprintf(commandtopic, "%s%s%d", basetopic, switchTopic, index);
          mqttClient.publish(commandtopic,"off");
          Serial.print(commandtopic);
          Serial.println(" off");
        }
      }
      else
      {
        tempstate[index]++;
        if (tempstate[index] > debounce )
        {
          bitWrite(myButton,index,1);
          sprintf(commandtopic, "%s%s%d", basetopic, switchTopic, index);
          mqttClient.publish(commandtopic,"on");
          Serial.print(commandtopic);
          Serial.println(" on");
        }
      }
    }
  }
}

