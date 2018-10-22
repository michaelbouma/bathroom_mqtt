// ##################################################
// ##   Write RGB colors to the WS2801 LED strip   ##
// ##################################################

void ws2801Run (void) {
  if (ColorChange==true)
  {
    strip_colors[0] = Color(SoffitR,SoffitG,SoffitB);
    post_frame();
    
    ColorChange = false;
  }  
}

// ##################################################
// ##      Send data to the WS2801 Led Strip       ##
// ##################################################

void post_frame (void) {
  //Each LED requires 24 bits of data
  //MSB: R7, R6, R5..., G7, G6..., B7, B6... B0 
  //Once the 24 bits have been delivered, the IC immediately relays these bits to its neighbor
  //Pulling the clock low for 500us or more causes the IC to post the data.

  for(int LED_number = 0 ; LED_number < 1 ; LED_number++) {
    long this_led_color = strip_colors[LED_number]; //24 bits of color data

    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
      //Feed color bit 23 first (red data MSB)
      
      digitalWrite(CKIPIN, LOW); //Only change data when clock is low
      
      long mask = 1L << color_bit;
      //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.
      
      if(this_led_color & mask) 
        digitalWrite(SDIPIN, HIGH);
      else
        digitalWrite(SDIPIN, LOW);
  
      digitalWrite(CKIPIN, HIGH); //Data is latched when clock goes high
    }
  }

  //Pull clock low to put strip into reset/post mode
  digitalWrite(CKIPIN, LOW);
  delayMicroseconds(500); //Wait for 500us to go into reset
}

// ##################################################
// ##             Convert RGB data                 ##
// ##################################################

uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}


