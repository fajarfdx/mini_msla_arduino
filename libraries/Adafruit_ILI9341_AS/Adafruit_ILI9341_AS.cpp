/***************************************************
  This is our library for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  13/2/15
  Updated to Arduino only and hardware SPI only
  Cleaned out legacy code
  18/3/15
  Minor tweaks for enhanced drawing speed on UNO
 ****************************************************/

#include "Adafruit_ILI9341_AS.h"
#include <avr/pgmspace.h>
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>


// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ILI9341_AS::Adafruit_ILI9341_AS(int8_t cs, int8_t dc, int8_t rst) : Adafruit_GFX_AS(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  _mosi  = _sclk = 0;
}

void Adafruit_ILI9341_AS::spiwrite(uint8_t c) {
#ifdef F_AS_T
    SPDR = c;
    while(!(SPSR & _BV(SPIF)));
#else
    uint8_t backupSPCR = SPCR;
    SPCR = mySPCR;
    SPDR = c;
    while(!(SPSR & _BV(SPIF)));
    SPCR = backupSPCR;
#endif
}


void Adafruit_ILI9341_AS::writecommand(uint8_t c) {
#ifdef F_AS_T
  PORTB &= ~_BV(1); // D9
  PORTB &= ~_BV(2); //D10
  SPDR = c;
  while(!(SPSR & _BV(SPIF)));
  PORTB |= _BV(2);
#else
  *dcport &=  ~dcpinmask;
  *csport &= ~cspinmask;
  spiwrite(c);
  *csport |= cspinmask;
#endif
}


void Adafruit_ILI9341_AS::writedata(uint8_t c) {
#ifdef F_AS_T
  PORTB |= _BV(1); // D9
  PORTB &= ~_BV(2); //D10
  SPDR = c;
  while(!(SPSR & _BV(SPIF)));
  PORTB |= _BV(2);
#else
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
  spiwrite(c);
  *csport |= cspinmask;
#endif
} 

void Adafruit_ILI9341_AS::writeBegin() {
#ifdef F_AS_T
  PORTB |= _BV(1); // D9
  PORTB &= ~_BV(2); //D10
#else
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
#endif
} 

void Adafruit_ILI9341_AS::writeEnd() {
#ifdef F_AS_T
  PORTB |= _BV(2);
#else
  *csport |= cspinmask;
#endif
} 

// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
#ifdef SPI_HAS_TRANSACTION
static inline void spi_begin(void) __attribute__((always_inline));

static inline void spi_begin(void) {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
}

static inline void spi_end(void) __attribute__((always_inline));

static inline void spi_end(void) {
  SPI.endTransaction();
}

#else
#define spi_begin()
#define spi_end()
#endif

void Adafruit_ILI9341_AS::init(void) {
  if (_rst > 0) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
  }

  pinMode(_dc, OUTPUT);
  pinMode(_cs, OUTPUT);
  csport    = portOutputRegister(digitalPinToPort(_cs));
  cspinmask = digitalPinToBitMask(_cs);
  dcport    = portOutputRegister(digitalPinToPort(_dc));
  dcpinmask = digitalPinToBitMask(_dc);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2); // 8 MHz (full! speed!)
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  mySPCR = SPCR;

  // toggle RST low to reset
  if (_rst > 0) {
    digitalWrite(_rst, HIGH);
    delay(5);
    digitalWrite(_rst, LOW);
    delay(20);
    digitalWrite(_rst, HIGH);
    delay(150);
  }

  //if(cmdList) commandList(cmdList);
  
  if (hwSPI) spi_begin();
  writecommand(0xEF);
  writedata(0x03);
  writedata(0x80);
  writedata(0x02);

  writecommand(0xCF);  
  writedata(0x00); 
  writedata(0XC1); 
  writedata(0X30); 

  writecommand(0xED);  
  writedata(0x64); 
  writedata(0x03); 
  writedata(0X12); 
  writedata(0X81); 
 
  writecommand(0xE8);  
  writedata(0x85); 
  writedata(0x00); 
  writedata(0x78); 

  writecommand(0xCB);  
  writedata(0x39); 
  writedata(0x2C); 
  writedata(0x00); 
  writedata(0x34); 
  writedata(0x02); 
 
  writecommand(0xF7);  
  writedata(0x20); 

  writecommand(0xEA);  
  writedata(0x00); 
  writedata(0x00); 
 
  writecommand(ILI9341_PWCTR1);    //Power control 
  writedata(0x23);   //VRH[5:0] 
 
  writecommand(ILI9341_PWCTR2);    //Power control 
  writedata(0x10);   //SAP[2:0];BT[3:0] 
 
  writecommand(ILI9341_VMCTR1);    //VCM control 
  writedata(0x3e);
  writedata(0x28); 
  
  writecommand(ILI9341_VMCTR2);    //VCM control2 
  writedata(0x86);  //--
 
  writecommand(ILI9341_MADCTL);    // Memory Access Control 
  writedata(0x48);

  writecommand(ILI9341_PIXFMT);    
  writedata(0x55); 
  
  writecommand(ILI9341_FRMCTR1);    
  writedata(0x00);  
  writedata(0x18); 
 
  writecommand(ILI9341_DFUNCTR);    // Display Function Control 
  writedata(0x08); 
  writedata(0x82);
  writedata(0x27);  
 
  writecommand(0xF2);    // 3Gamma Function Disable 
  writedata(0x00); 
 
  writecommand(ILI9341_GAMMASET);    //Gamma curve selected 
  writedata(0x01); 
 
  writecommand(ILI9341_GMCTRP1);    //Set Gamma 
  writedata(0x0F); 
  writedata(0x31); 
  writedata(0x2B); 
  writedata(0x0C); 
  writedata(0x0E); 
  writedata(0x08); 
  writedata(0x4E); 
  writedata(0xF1); 
  writedata(0x37); 
  writedata(0x07); 
  writedata(0x10); 
  writedata(0x03); 
  writedata(0x0E); 
  writedata(0x09); 
  writedata(0x00); 
  
  writecommand(ILI9341_GMCTRN1);    //Set Gamma 
  writedata(0x00); 
  writedata(0x0E); 
  writedata(0x14); 
  writedata(0x03); 
  writedata(0x11); 
  writedata(0x07); 
  writedata(0x31); 
  writedata(0xC1); 
  writedata(0x48); 
  writedata(0x08); 
  writedata(0x0F); 
  writedata(0x0C); 
  writedata(0x31); 
  writedata(0x36); 
  writedata(0x0F); 

  writecommand(ILI9341_SLPOUT);    //Exit Sleep 
  if (hwSPI) spi_end();
  delay(120); 		
  if (hwSPI) spi_begin();
  writecommand(ILI9341_DISPON);    //Display on 
  if (hwSPI) spi_end();

}


void Adafruit_ILI9341_AS::setAddrWindow(int16_t x0, int16_t y0, int16_t x1,
 int16_t y1) {

#ifdef F_AS_T
  // Column addr set
    PORTB &= ~_BV(1); // D9
    PORTB &= ~_BV(2); //D10
    SPDR = ILI9341_CASET;
    while(!(SPSR & _BV(SPIF)));

  PORTB |= _BV(1);
    SPDR = x0>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = x0; while(!(SPSR & _BV(SPIF)));
    SPDR = x1>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = x1; while(!(SPSR & _BV(SPIF)));

  // Row addr set
    PORTB &= ~_BV(1); // D9
    PORTB &= ~_BV(2); //D10
    SPDR = ILI9341_PASET; while(!(SPSR & _BV(SPIF)));

    PORTB |= _BV(1);
    SPDR = y0>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = y0; while(!(SPSR & _BV(SPIF)));
    SPDR = y1>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = y1; while(!(SPSR & _BV(SPIF)));

  // write to RAM
    PORTB &= ~_BV(1); // D9
    PORTB &= ~_BV(2); //D10
    SPDR = ILI9341_RAMWR; while(!(SPSR & _BV(SPIF)));

  //CS, HIGH;
  PORTB |= _BV(2);
  PORTB |= _BV(1);
#else
  writecommand(ILI9341_CASET); // Column addr set
  writedata(x0 >> 8);
  writedata(x0 & 0xFF);     // XSTART 
  writedata(x1 >> 8);
  writedata(x1 & 0xFF);     // XEND

  writecommand(ILI9341_PASET); // Row addr set
  writedata(y0>>8);
  writedata(y0);     // YSTART
  writedata(y1>>8);
  writedata(y1);     // YEND

  writecommand(ILI9341_RAMWR); // write to RAM
#endif
}

void Adafruit_ILI9341_AS::drawPixel(int16_t x, int16_t y, uint16_t color) {

  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

#ifdef F_AS_T
  // Column addr set
    PORTB &= ~_BV(1); // D9
    PORTB &= ~_BV(2); //D10
    SPDR = ILI9341_CASET;
    while(!(SPSR & _BV(SPIF)));

  PORTB |= _BV(1);
    SPDR = x>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = x; x++; while(!(SPSR & _BV(SPIF)));
    SPDR = x>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = x; while(!(SPSR & _BV(SPIF)));

  // Row addr set
    PORTB &= ~_BV(1); // D9
    PORTB &= ~_BV(2); //D10
    SPDR = ILI9341_PASET; while(!(SPSR & _BV(SPIF)));

  PORTB |= _BV(1);
    SPDR = y>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = y; y++; while(!(SPSR & _BV(SPIF)));
    SPDR = y>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = y; while(!(SPSR & _BV(SPIF)));

  // write to RAM
    PORTB &= ~_BV(1); // D9
    PORTB &= ~_BV(2); //D10
    SPDR = ILI9341_RAMWR; while(!(SPSR & _BV(SPIF)));

  PORTB |= _BV(1);
    SPDR = color>>8; while(!(SPSR & _BV(SPIF)));
    SPDR = color; while(!(SPSR & _BV(SPIF)));

  //CS, HIGH;
  PORTB |= _BV(2);
  PORTB |= _BV(1);
#else
  spi_begin();
  setAddrWindow(x,y,x+1,y+1);

  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;

  spiwrite(color >> 8);
  spiwrite(color);

  *csport |= cspinmask;
  spi_end();
#endif
}

// Sends an array of 16-bit color values to the TFT; used
// externally by BMP examples.  Assumes that setAddrWindow() has
// previously been called to define the bounds.  Max 255 pixels at
// a time (BMP examples read in small chunks due to limited RAM).
void Adafruit_ILI9341_AS::pushColors(uint16_t *data, uint8_t len) {
  uint16_t color;
  spi_begin();

  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
  uint8_t backupSPCR =SPCR;
  SPCR = mySPCR;

  while(len--) {
    color = *data++;
    // This order is fast as we loop back & fetch during the SPI wait period!
    while(!(SPSR & _BV(SPIF)));
    SPDR = color>>8;
    while(!(SPSR & _BV(SPIF)));
    SPDR = color;
  }
  while(!(SPSR & _BV(SPIF)));

  SPCR = backupSPCR;
  *csport |= cspinmask;

  spi_end();
}

// This is the byte array version for 16 bit raw images.
void Adafruit_ILI9341_AS::pushColors(uint8_t *data, uint8_t len) {
  spi_begin();

  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;
  uint8_t backupSPCR =SPCR;
  SPCR = mySPCR;
  while(len--) {
    // This order is fast as we loop back during the SPI wait period!
    while(!(SPSR & _BV(SPIF)));
    SPDR = *data++;
    while(!(SPSR & _BV(SPIF)));
    SPDR = *data++;
  }
  while(!(SPSR & _BV(SPIF)));
  SPCR = backupSPCR;
  *csport |= cspinmask;

  spi_end();
}

// Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer this uses
// the eficient FastH/V Line draw routine for segments of 2 pixels or more
void Adafruit_ILI9341_AS::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {

  boolean steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx = x1 - x0, dy = abs(y1 - y0);;


  int16_t err = dx>>1, ystep=-1, xs=x0, dlen=0;
  if (y0 < y1) ystep = 1;

  // Split into steep and not steep for FastH/V separation
  if(steep) {
    for (; x0<=x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;
        if (dlen==1) drawPixel(y0, xs, color);
        else drawFastVLine(y0, xs, dlen, color);
        dlen = 0; y0 += ystep; xs=x0+1;
      }
    }
    if(dlen) drawFastVLine(y0, xs, dlen, color);
  }
  else
  {
    for (; x0<=x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;
        if (dlen==1) drawPixel(xs, y0, color);
        else drawFastHLine(xs, y0, dlen, color);
        dlen = 0; y0 += ystep; xs=x0+1;
      }
    }
    if(dlen) drawFastHLine(xs, y0, dlen, color);
  }
}


/* Original Adafruit drawLine is slower
// Bresenham's algorithm - thx wikpedia
void Adafruit_ILI9341_AS::drawLine(int16_t x0, int16_t y0,
			    int16_t x1, int16_t y1,
			    uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}
*/

/*
// Compact version of Bresenham's algorithm is less bytes but slower than Adafruits
void Adafruit_ILI9341_AS::drawLine(int16_t x0, int16_t y0,
			    int16_t x1, int16_t y1,
			    uint16_t color) {
int x = x1-x0;
int y = y1-y0;
int dx = abs(x), sx = x0<x1 ? 1 : -1;
int dy = -abs(y), sy = y0<y1 ? 1 : -1;
int err = dx+dy, e2;

for (;;){ // loop
  drawPixel(x0,y0,color);
  e2 = err<<1;
  if (e2 >= dy) {
    if (x0 == x1) break;
    err += dy; x0 += sx;
    }

  if (e2 <= dx) {
    if (y0 == y1) break;
    err += dx; y0 += sy;
    }
  }
}
*/


void Adafruit_ILI9341_AS::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;

  if((y+h-1) >= _height) 
    h = _height-y;

  spi_begin();
  setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;

#ifdef F_AS_T
  PORTB &= ~_BV(2); //D10
  PORTB |= _BV(1);

  while (h--) {
    while(!(SPSR & _BV(SPIF)));
    SPDR = hi;
    while(!(SPSR & _BV(SPIF)));
    SPDR = lo;
  }
  while(!(SPSR & _BV(SPIF)));
  PORTB |= _BV(2);
#else
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;

  while (h--) {
    spiwrite(hi);
    spiwrite(lo);
  }
  *csport |= cspinmask;
#endif
  spi_end();
}


void Adafruit_ILI9341_AS::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;

  spi_begin();
  setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;

#ifdef F_AS_T
  PORTB &= ~_BV(2); //D10
  PORTB |= _BV(1);

  while (w--) {
    while(!(SPSR & _BV(SPIF)));
    SPDR = hi;
    while(!(SPSR & _BV(SPIF)));
    SPDR = lo;
  }
  while(!(SPSR & _BV(SPIF)));
  PORTB |= _BV(2);
#else
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;

  while (w--) {
    spiwrite(hi);
    spiwrite(lo);
  }
  *csport |= cspinmask;
#endif
  spi_end();
}

void Adafruit_ILI9341_AS::fillScreen(uint16_t color) {
  fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void Adafruit_ILI9341_AS::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  spi_begin();
  setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;

  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;

  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {

#ifdef F_AS_T
      while(!(SPSR & _BV(SPIF)));
      SPDR = hi;
      while(!(SPSR & _BV(SPIF)));
      SPDR = lo;
#else
      spiwrite(hi);
      spiwrite(lo);
#endif
    }
  }
#ifdef F_AS_T
while(!(SPSR & _BV(SPIF)));
#endif
  *csport |= cspinmask;
  spi_end();
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ILI9341_AS::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void Adafruit_ILI9341_AS::setRotation(uint8_t m) {
  rotation=m%8;
  if (hwSPI) spi_begin();
  writecommand(ILI9341_MADCTL);
  switch (rotation) {
   case 0:
     writedata(MADCTL_MX | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
     break;
   case 1:
     writedata(MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
   case 2:
    writedata(MADCTL_MY | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
    break;
   case 3:
     writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
   case 4:
     writedata(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
     break;
   case 5:
     writedata(MADCTL_MV | MADCTL_MX | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
   case 6:
    writedata(MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
    break;
   case 7:
     writedata(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
  }
  if (hwSPI) spi_end();
}


void Adafruit_ILI9341_AS::invertDisplay(boolean i) {
  spi_begin();
  writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);
  spi_end();
}

