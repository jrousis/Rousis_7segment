/* Library to interface with the Texas Instruments TLC5916 and TLC5917
   8-Channel Constant-Current LED Sink Drivers.
*/
/* Version History
   1.0.0    08/03/2018  A.T.   Original
   1.1.0    09/25/2020  A.T.   Support more daisy-chained displays.
                               Fixed array bound issue in previous
                               un-numbered file version.
   1.2.0    01/17/2021  A.T.   Add support for special mode.
*/
//#include <analogWrite.h>
#if defined(ESP32)
	#include <analogWrite.h>
#endif
#include "Rousis7segment.h"

Rousis7segment::Rousis7segment(byte n, byte SDI, byte CLK, byte LE, byte OE) {
  SDI_pin = SDI;
  CLK_pin = CLK;
  LE_pin  = LE;
  OE_pin  = OE;
  numchips = n;

  //digitalWrite(OE_pin, HIGH);
  enableState = DISABLE;
  //pinMode(OE_pin, OUTPUT);
  //analogWriteResolution(OE_pin, 10);
#if defined(ESP32)
  analogWrite(OE_pin, 0, 80, 8, 0);
#else
  pinMode(OE_pin, OUTPUT);
  digitalWrite(OE_pin, LOW);
#endif

  init();
}

Rousis7segment::Rousis7segment(byte n, byte SDI, byte CLK, byte LE) {
  SDI_pin = SDI;
  CLK_pin = CLK;
  LE_pin  = LE;
  OE_pin  = NO_PIN;
  numchips = n;

  init();
}

void Rousis7segment::init() {
  if (numchips < MINCHIPS) numchips = MINCHIPS;
  if (numchips > MAXCHIPS) numchips = MAXCHIPS;

  pinMode(SDI_pin, OUTPUT);
  pinMode(CLK_pin, OUTPUT);
  digitalWrite(LE_pin, LOW);
  pinMode(LE_pin, OUTPUT);
}

void Rousis7segment::print(const char* s, bool Invert) {
  int i;
  char c;
  byte pos;
  uint8_t dot;
  char character;

  if (Invert){
    for (i = 0; i < numchips; i++) {
      // Need a range check and adjustment, since segment map starts
      // at ASCII 32 (element 0) and ends at ASCII 0x7f (element 96)
      // Out of range default to a blank character
      // if (s[i] > 127) pos = 0; --> This would be a negative value, included in next check
        dot = s[i] & 0x80;
        character = s[i] & 0b01111111;
        if (character < 32) pos = 0;
        else pos = character - 32;
        c = ICM7218_segment_map[pos];
        c |= dot;
        write(c);
    }
  } else {
    for (i = numchips - 1; i >= 0; i--) {
    // Need a range check and adjustment, since segment map starts
    // at ASCII 32 (element 0) and ends at ASCII 0x7f (element 96)
    // Out of range default to a blank character
    // if (s[i] > 127) pos = 0; --> This would be a negative value, included in next check
	    dot = s[i] & 0x80;
	    character = s[i] & 0b01111111;
        if (character < 32) pos = 0;
        else pos = character - 32;
        c = ICM7218_segment_map[pos] ;
	    c |=  dot;
        write(c);
    }
  }
 
  toggleLE();
}

void Rousis7segment::print(unsigned int n){
  byte size;
  if (numchips > 2) size = 2;
  else size = numchips;
  for (byte i = 0; i < size; i++) {
    write(n);
    n = n/256; // Get the next most significant byte
  }
  toggleLE();
}

void Rousis7segment::printDirect(const uint8_t* s) {
  for (int i = numchips - 1; i >= 0; i--) {
    write(s[i]);
  }
  toggleLE();
}

void Rousis7segment::displayEnable() {
  if (OE_pin != NO_PIN) {
    digitalWrite(OE_pin, LOW);
    enableState = ENABLE;
  }
}

void Rousis7segment::displayDisable() {
  if (OE_pin != NO_PIN) {
    digitalWrite(OE_pin, HIGH);
    enableState = DISABLE;
  }
}

void Rousis7segment::normalMode() {
  if (OE_pin != NO_PIN) {
    digitalWrite(OE_pin, HIGH);
    toggleCLK();
    digitalWrite(OE_pin, LOW);
    toggleCLK();
    digitalWrite(OE_pin, HIGH); // Disables the display
    toggleCLK();
    toggleCLK();   // Mode switching
    toggleCLK();   // Now in normal mode
    if (enableState == ENABLE) displayEnable(); // Re-enable display if it was enabled previously
  }
}

void Rousis7segment::specialMode() {
  if (OE_pin != NO_PIN) {
    digitalWrite(OE_pin, HIGH);
    toggleCLK();
    digitalWrite(OE_pin, LOW);
    toggleCLK();
    digitalWrite(OE_pin, HIGH); // Disables the display
    toggleCLK();
    digitalWrite(LE_pin, HIGH);
    toggleCLK();   // Mode switching
    digitalWrite(LE_pin, LOW);
    toggleCLK();   // Now in special mode
    // Switching to special mode disables the display by default
    enableState = DISABLE;
  }
}

void Rousis7segment::displayBrightness(byte b) {
  if (OE_pin != NO_PIN){
      b = 255 - b;
	analogWrite(OE_pin, b);
	/*Serial.print("Printed brightness: ");
	Serial.println(b, DEC);
	Serial.print("To the pin: ");
	Serial.println(OE_pin, DEC);*/
  }else{
	Serial.println("No set brightness...");
  }
}
#if defined(ESP32)
void Rousis7segment::write(byte n) {
   // Serial.println(n, HEX);
    //This is converted by Rousis James!~
    digitalWrite(SDI_pin, n & 0x80);	//DOT
    toggleCLK();
    digitalWrite(SDI_pin, n & 0x10);	//A
    toggleCLK();
    digitalWrite(SDI_pin, n & 0x20);	//B
    toggleCLK();
    digitalWrite(SDI_pin, n & 0x01);	//C
    toggleCLK();

    digitalWrite(SDI_pin, n & 0x04);	//E
    toggleCLK();
    digitalWrite(SDI_pin, n & 0x40);	//G
    toggleCLK();
    digitalWrite(SDI_pin, n & 0x08);	//F
    toggleCLK();
    digitalWrite(SDI_pin, n & 0x02);	//D
    toggleCLK();

}
#else
void Rousis7segment::write(byte n) {
  uint8_t i;
  //This is converted by Rousis James!~
  i = n & 0x80;
  if (i){digitalWrite(SDI_pin, 1);}else{ digitalWrite(SDI_pin, 0);} //DOT
  toggleCLK();
  i = n & 0x10;
  if (i){ digitalWrite(SDI_pin, 1);}else{digitalWrite(SDI_pin, 0);}
  toggleCLK();
  i = n & 0x20;
  if (i) { digitalWrite(SDI_pin, 1);}else{digitalWrite(SDI_pin, 0);}
  toggleCLK();
  i = n & 0x01;
  if (i){ digitalWrite(SDI_pin, 1);}else{digitalWrite(SDI_pin, 0);}
  toggleCLK();
  i = n & 0x04;
  if (i){digitalWrite(SDI_pin, 1);}else{ digitalWrite(SDI_pin, 0);}
  toggleCLK();
  i = n & 0x40;
  if (i) {digitalWrite(SDI_pin, 1);}else{digitalWrite(SDI_pin,0);}
  toggleCLK();
  i = n & 0x08;
  if (i) {digitalWrite(SDI_pin, 1);}else{digitalWrite(SDI_pin, 0);}
  toggleCLK();
  i = n & 0x02;
  if (i) {digitalWrite(SDI_pin, 1);}else{digitalWrite(SDI_pin, 0);}
  toggleCLK();  
}
#endif

/*

// Original...
void Rousis7segment::write(byte n) {
	digitalWrite(SDI_pin, n & 0x01);
	toggleCLK();
	digitalWrite(SDI_pin, n & 0x02);
	toggleCLK();
	digitalWrite(SDI_pin, n & 0x04);
	toggleCLK();
	digitalWrite(SDI_pin, n & 0x08);
	toggleCLK();
	digitalWrite(SDI_pin, n & 0x10);
	toggleCLK();
	digitalWrite(SDI_pin, n & 0x20);
	toggleCLK();
	digitalWrite(SDI_pin, n & 0x40);
	toggleCLK();
	digitalWrite(SDI_pin, n & 0x80);
	toggleCLK();
}
 */

void Rousis7segment::toggleLE() {
  // TLC5916 minimum LE pulse time is 20 ns, so don't need to worry about
  // putting in a hard-coded delay.
  digitalWrite(LE_pin, HIGH);
  digitalWrite(LE_pin, LOW);
}

void Rousis7segment::toggleCLK() {
  // TLC5916 minimum CLK pulse time is 20 ns, so don't need to worry about
  // putting in a hard-coded delay.
  digitalWrite(CLK_pin, HIGH);
  digitalWrite(CLK_pin, LOW);
}

void Rousis7segment::TestSegments(uint8_t numchips) {
    uint8_t n[12];
    size_t i;
    uint8_t val = 0b00000100;

    val = 0b00000010;
    for ( i = 0; i < numchips; i++){n[i]=val;}
    printDirect(n);
    delay(500);
    val = 0b00001000;
    for (i = 0; i < numchips; i++) {n[i] = val;}
    printDirect(n);
    delay(500);
    val = 0b01000000;
    for (i = 0; i < numchips; i++) {n[i] = val;}
    printDirect(n);
    delay(500);
    val = 0b00000100;
    for (i = 0; i < numchips; i++) {n[i] = val;}
    printDirect(n);
    delay(500);
    val = 0b00000001;
    for (i = 0; i < numchips; i++) {n[i] = val;}
    printDirect(n);
    delay(500);
    val = 0b00100000;
    for (i = 0; i < numchips; i++) {n[i] = val;}
    printDirect(n);
    delay(500);
    val = 0b00010000;
    for (i = 0; i < numchips; i++) {n[i] = val;}
    printDirect(n);
    delay(500);
    val = 0b10000000;
    for (i = 0; i < numchips; i++) {n[i] = val;}
    printDirect(n);
    delay(500);
}
