// ##################################################
// ##                 BH1750_Init                  ##
// ##################################################

void BH1750_Init()
{
  // begin returns a boolean that can be used to detect setup problems.
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) 
  {
    Serial.println("BH1750 Advanced begin");
    setColor(0, 0, 50);
  }
  else 
  {
    Serial.println("Error initialising BH1750");
    setColor(50, 0, 0);
  }
}

// ##################################################
// ##              Process PIR data                ##
// ##################################################

void pirRun(){
  int n = digitalRead(PIRPIN);
  if(n != pirStatus)
  {
    pirStatus = n;
    
    //delay(50);                    // delay against bouncing inputs

    sprintf(commandtopic, "%s%s", basetopic, pirTopic);
    if (n)
      mqttClient.publish(commandtopic,"on");  
    else  
      mqttClient.publish(commandtopic,"off");  
      
   }
}

// ##################################################
// ##                Process DHT                   ##
// ##################################################

void dht22Run(){
  if (timeElapsed > dhtInterval) 
  {        
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    humidity1 = dht_a.readHumidity();
    humidity2 = dht_b.readHumidity();
    // Read temperature as Celsius (the default)
    temperature1 = dht_a.readTemperature();
    temperature2 = dht_b.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity1) || isnan(temperature1) ) {
      Serial.println("Failed to read from DHT_A sensor!");
      return;
    }

    if (isnan(humidity2) || isnan(temperature2) ) {
      Serial.println("Failed to read from DHT_B sensor!");
      return;
    }
  
    char buffer[10];
    dtostrf(temperature1,0, 0, buffer);
    Serial.print("Temperature_1 : "); 
    Serial.println(temperature1);  
    sprintf(commandtopic, "%s%s", basetopic, dhtaTempTopic);
    mqttClient.publish(commandtopic,buffer);
  
    dtostrf(humidity1,0, 0, buffer);
    Serial.print("Humidity_1    : "); 
    Serial.println(humidity1);  
    sprintf(commandtopic, "%s%s", basetopic, dhtaHummTopic);
    mqttClient.publish(commandtopic,buffer);

    dtostrf(temperature2,0, 0, buffer);
    Serial.print("Temperature_2 : "); 
    Serial.println(temperature2);  
    sprintf(commandtopic, "%s%s", basetopic, dhtbTempTopic);
    mqttClient.publish(commandtopic,buffer);
  
    dtostrf(humidity2,0, 0, buffer);
    Serial.print("Humidity_2    : "); 
    Serial.println(humidity2);  
    sprintf(commandtopic, "%s%s", basetopic, dhtbHummTopic);
    mqttClient.publish(commandtopic,buffer);

    timeElapsed = 0;              // reset the counter to 0 so the counting starts over...
    ReadDHT = true;
  }
}

// ##################################################
// ##                      dsRun                   ##
// ##################################################
void ds18B20Run()
{
  if (dsDelayElapsed > (dsStart + dsRead))
  {
      if (standalone==false)
        setColor(0,10,0);
      else  
        setColor(20,10,0);
      char temperaturenow [15];
      dtostrf(sensors.getTempCByIndex(0),3, 2, temperaturenow);  //// convert float to char      
      Serial.print("Temperature_3 : "); 
      Serial.println(temperaturenow);  
      sprintf(commandtopic, "%s%s", basetopic, dsTopic);
      mqttClient.publish(commandtopic,temperaturenow);
      dsRunning = false;
      dsDelayElapsed = 0;
  } 
  else if (dsDelayElapsed > dsStart)
  {
      if (dsRunning == false)
      {
        setColor(0,0,100);
        sensors.requestTemperatures();
        dsRunning = true;
      }
  }
}

// ##################################################
// ##                      BH1750Run               ##
// ##################################################
void bh1750Run()
{
  if (bh1750DelayElapsed > (bh1750Interval))
  {
    
      dtostrf(lightMeter.readLightLevel(),3, 2, buff);  //// convert float to char      
      Serial.print("LightLevel_1  : "); 
      Serial.println(buff);  
      sprintf(commandtopic, "%s%s", basetopic, bh1750Topic);
      mqttClient.publish(commandtopic,buff);
      bh1750DelayElapsed = 0;
  } 
}


// ##################################################
// ##                    setColor                  ##
// ##################################################
void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}

