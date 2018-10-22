

void MQTTinit() {
    sprintf (macString, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);  
    sprintf (buff, "%s%s/#", basevector, macString);
    mqttClient.subscribe(buff); // subscribing to e.g. raw/mac/MAC/#
    Serial.print("Subscribing to Topic: ");
    Serial.println(buff);

    // First Send init to reset OpenHAB
    mqttClient.publish(basevector, "init");
    // Now publish this MAC address to a generic topic e.g. raw/mac
    Serial.print("Sending to Topic: ");
    Serial.print(basevector);
    Serial.print(" Message: ");
    Serial.println(macString);
    mqttClient.publish(basevector, macString);
    
    // Retrieve Zonename and Zonevect from openHab
    unsigned long mqttconnecttimeout = millis();
    while (basetopic[0] == '<') { // whilst we haven't finished downloading controller startup stuff
      if (millis() - mqttconnecttimeout > initialmqttconnecttimeout) {
        Serial.println("MQTT timeout");
        break;
      }
      mqttClient.loop(); // This is usually found in the main loop, but we are checking for incoming MQTT messages at this stage too
    }
    mqttresponsetime = millis() - mqttconnecttimeout;

    // Set up commandtopic
    sprintf(commandtopic, "%s%s", basetopic, statusvector);
    mqttClient.publish(commandtopic, "Setup Done!");
    setColor(0, 10, 0); // Color green
    
    sprintf(commandtopic, "%s%s%s", basetopic, relayTopic,"/#");
    mqttClient.subscribe(commandtopic);  
    Serial.print("Subscribing to topic: ");
    Serial.println(commandtopic);
    sprintf(commandtopic, "%s%s", basetopic, RGBTopic);
    mqttClient.subscribe(commandtopic);  
    Serial.print("Subscribing to topic: ");
    Serial.println(commandtopic);
    mqttClient.subscribe(openhabTopic);  
    Serial.print("Subscribing to topic: ");
    Serial.println(openhabTopic);
    // publish the state of the relays

    byte index;
    for (index = 0; index < NUMBUTTONS; index++) {
      if(currentstate[index]==0)
        sprintf (buff, "%s","off"); 
      else
        sprintf (buff, "%s","on"); 
      sprintf(commandtopic, "%s%s%s%d", basetopic, relayTopic, "Status", index);
      mqttClient.publish(commandtopic,buff);
      Serial.print(commandtopic);
      Serial.print(" ");
      Serial.println(buff);
    }
}

// ##################################################
// ##                 mqttRun                      ##
// ##################################################
void mqttRun()
{
  if (!mqttClient.loop()) 
  {   // ProcessMQTT events
    mqttConnected = false;
    if (MQTTDisconnectElapsed > MQTTDisconnect)
    {
      
      Serial.println("MQTT Client disconnected...");
      if (mqttClient.connect(MQTTName))
      {
        setColor(0, 10, 0); // Color green
        Serial.println("MQTT reconnected");
        MQTTinit();
      } 
    
      else 
      {
        setColor(10, 0, 0); // Color green
        Serial.println("MQTT failed !!");
        Serial.print("retry in ");
        Serial.print(MQTTDisconnect);
        Serial.println(" milliseconds");
      }
      MQTTDisconnectElapsed = 0;
    }
  }
  else
    mqttConnected = true;
}


// ##################################################
// ##            MQTT Callback function            ##
// ##################################################

void callback(char* topic, byte* payload, unsigned int length) {
  char* json;
  json = (char*) malloc(length + 1);
  memcpy(json, payload, length);
  json[length] = '\0';  
  String MyTopic = topic;



  String TmpStr;
  int t;

  // Controller Setup
  sprintf (buff, "%s%02X%02X%02X%02X%02X%02X", basevector, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);  
  TmpStr = buff;
  if(MyTopic.substring(0,TmpStr.length()) == TmpStr) {
    standalone = false;
    if (payload[4]==84) {
      for (int i=0; i<30; i++) basetopic[i] = payload[i+10]; // base topic (T)
    }
    if (payload[4]==82) {                                  // Remote reboot (R)
      resetFunc();
    }
  }
  
  // RGB Color Strip
  sprintf(buff, "%s%s", basetopic, RGBTopic);
  TmpStr = buff;
  if(MyTopic.substring(0,TmpStr.length()) == TmpStr) {
    String value = String((char*)payload);
    SoffitR = value.substring(0,value.indexOf(',')).toInt();
    SoffitG = value.substring(value.indexOf(',')+1,value.lastIndexOf(',')).toInt();
    SoffitB = value.substring(value.lastIndexOf(',')+1).toInt();
    ColorChange = true;
  }

  // Relays
  sprintf(buff, "%s%s", basetopic, relayTopic);
  TmpStr = buff;
  if(MyTopic.substring(0,TmpStr.length()) == TmpStr) {
    String SubStr = MyTopic.substring(TmpStr.length() + 1);
    int Rn = SubStr.toInt() - 1;
    if ((Rn < 9) && (Rn > -1)) // Rn between 1 and 8
    {
      if (String(json) == "on")  bitWrite(myRelay,Rn,LOW);
      if (String(json) == "off") bitWrite(myRelay,Rn,HIGH);
      Serial.print("Switching Relay-");
      Serial.print(Rn);
      Serial.print(" to ");
      Serial.println(json);
    }  
  }
   // openhab time topic
  TmpStr = openhabTopic;
  if(MyTopic.substring(0,TmpStr.length()) == TmpStr) {
    openhabTimeElapsed = 0;
    setColor(20,0,0);
    String value = String((char*)payload);
    Serial.print("Openhab time  : ");
    Serial.println(json);
    Serial.println("");
    standalone = false;
  }
 
 free(json);
}


