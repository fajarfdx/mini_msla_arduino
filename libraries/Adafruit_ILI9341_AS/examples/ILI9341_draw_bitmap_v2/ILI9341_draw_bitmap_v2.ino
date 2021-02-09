// On the SD card use 24 bit color BMP files (be sure they are 24-bit!)
// There are examples images in the sketch folder.

// Change IDE compiler option to -O2 as per Instructable to boost speed 
// http://www.instructables.com/id/Arduino-IDE-16x-compiler-optimisations-faster-code/
// this example then uses 85% of UNO FLASH and 52% of RAM for dynamic storage


// MS Paint can be used to crop, resize and flip and save images in 24bit BMP format

//          ######################## WARNINGS ########################
// ####     Only enable Font 2 or the UNO willl run out of FLASH memory!      ####
// ####  Do not try to use the F_AS_T option in the ILI9341_AS library yet    ####
// #### Change IDE compiler option to -O2 as per Instructable to boost speed  ####


// Updated 31/3/15 to include new drawRAW() function that draws raw 16 bit images.
// Raw images are created with UTFT library tool (libraries\UTFT\Tools\ImageConverter565.exe)
// Save as .raw file as this can be easily piped to the TFT pushColors() function
// without any tedious byte swapping etc
 
#include <Adafruit_GFX_AS.h>     // Core graphics library
#include <Adafruit_ILI9341_AS.h> // Hardware-specific library
//#include <SD.h>                // SD card library a bit slower and bigeger than SdFat
#include <SdFat.h>               // More compact and faster than SD
SdFat SD;                        // For SD compatibility
#include <SPI.h>                 // SPI libray obviously!

// These are the pins I use on an UNO, may need chaging for your setup

// Use hardware SPI lines
#define _sclk 13
#define _miso 12 // Needed for SD card, but does not need to be connected to TFT
#define _mosi 11 // Master Out Slave In to send commands/data to TFT and SD card

// TFT chip select and data/command line
#define _cs 10
#define _dc 9

// SD chip select
#define _sdcs 8

// TFT reset line, can be connected to Arduino reset
#define _rst 7

Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(_cs, _dc, _rst); // Invoke custom library

// You can use MS Paint to pick colours off an image and see the RGB values
// This colour is off the mouse image.
#define ILI9341_GREY 0xCE9A // Light grey

// These are used when calling drawBMP() function, see examples in loop()
#define BU_BMP 1 // Temporarily flip the TFT coords for standard Bottom-Up bit maps
#define TD_BMP 0 // Draw inverted Top-Down bitmaps in standard coord frame

uint32_t drawTime = 0; // Variable to save draw times for testing

/***************************************************************************************
** Function name:           setup
** Descriptions:            To set things up
***************************************************************************************/

void setup()
{
  Serial.begin(38400); // For debug messages

  Serial.print(F("Initialising SD card..."));
  if (!SD.begin(_sdcs, SPI_FULL_SPEED)) {  // sdFat library allows speed setting, e.g. SPI_HALF_SPEED
    //if (!SD.begin(_sdcs)) {              // Only needed when standard SD library is used
    Serial.println(F("failed!"));
    //return;
  }

  tft.init(); // Initialise the display (various parameters configured)

  Serial.println(F("Here we go..."));

  // Set text foreground and background colours
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
}

/***************************************************************************************
** Function name:           loop
** Descriptions:            Infinite loop
***************************************************************************************/

void loop()
{
  // Standard BMP files are usually stored with a raster scan of pixel values from bottom up
  // File names MUST use the 8.3 file name format (8 chars + . + any 3 extension letters)
  // By convention bitmap files end in .bmp
  // Bit maps MUST be in 24 bit colout (not 16 or 8 bit greyscale)
  
  // Landscape mode
  tft.setRotation(1);
  tft.fillScreen(ILI9341_GREY);
  
  // Draws raw image with top left corner pixel at x,y = 0,0
  drawRAW("Inst_1.raw", 0, 0, 320, 240);
  // Using a bmp will be 50% slower and draws from bottom up
  //drawBMP("Inst_1.bmp", 0, 0, BU_BMP);
  
  // Show draw time (time measuring lines can be commented out in function if not needed)
  int xpos = 0;
  xpos += tft.drawNumber(drawTime, xpos, 0, 2);
  tft.drawString("ms to draw", xpos, 0, 2);
      Serial.println(drawTime);
  delay(4000);

  // See how long slear screen takes
  unsigned long timenow = millis();
  tft.fillScreen(ILI9341_BLACK);
  timenow = millis()-timenow;
  Serial.println(timenow);

  // Draw bmp image top left corner at x,y = 40,0 which centres the 240 pixel wide bitmap
  // Image must fit (one day I will add clipping... but it will slow things down)
  drawBMP("240Moon.bmp", 40, 0, BU_BMP);
  xpos = 0;
  xpos += tft.drawNumber(drawTime, xpos, 0, 2);
  tft.drawString("ms to draw", xpos, 0, 2);
  delay(1000);

  // Now draw the raw image for speed comparison
  tft.fillScreen(ILI9341_BLACK);
  // Draw image top left corner at x,y = 40,0 which centres the 240 pixel wide bitmap
  // Image must fit (one day I will add clipping... but it will slow things down)
  drawRAW("240Moon.raw", 40, 0, 240, 240);
  xpos = 0;
  xpos += tft.drawNumber(drawTime, xpos, 0, 2);
  tft.drawString("ms to draw", xpos, 0, 2);
  Serial.println(drawTime);
  delay(1000);

  // Display Terminator image grey was picked off image by using MS Paint
  // and turned into a 565 BGR 16 bit format
  tft.fillScreen(0x94f5);
  // Look out
  drawRAW("225term.raw", 47, 7, 225, 225);
  delay(500);
  // We can draw on an image... plot coords can easily be determined with Paint and a calculator
  tft.fillCircle(47 + 80, 7 + 101, 3, ILI9341_RED);
  tft.fillCircle(47 + 140, 7 + 101, 3, ILI9341_RED);
  delay(1000);

  // To draw top down the image must be flipped vertically and stored, this is
  // easy to do in Paint. We can then use the TFT SGRAM flip features to make drawing faster...
  // May be a pain ;-) but drawing is >25% faster that doing the flip via file pointers!
  // and it only takes 2 clicks in Paint to "Flip Vertical", then can be saved upside down!
  tft.fillScreen(0x94f5);
  drawBMP("225termi.bmp", 47, 7, TD_BMP); // Note we have to set the flag for Top-Down drawing
  // TD_BMP flag invokes SGRAM coord flip in display, orientation is fliiped back by function
  // The x and y coordinates are of the image top left corner are still 0,0 now!
  delay(500);
  // We can draw upon an image... notice coords are the same as bottom up plot above
  // the drawBMP makes sure the inverted image is plotted in the correct relative coords!
  tft.fillCircle(47 + 80, 7 + 101, 3, ILI9341_RED);
  tft.fillCircle(47 + 140, 7 + 101, 3, ILI9341_RED);
  delay(1000);

  // A selfie - what I really look like when writing software!
  tft.fillScreen(ILI9341_WHITE);
  drawBMP("284taz.bmp", 18, 32, BU_BMP);
  delay(1000);

  // Draw the obligatory cute cat from a family holiday... to London!
  tft.setRotation(0); // We must set the orientation so the Tiger image fits!
  // the screen will be blank otherwise! (To do -> clipping to screen!)
  tft.fillScreen(ILI9341_WHITE);
  drawRAW("Tiger.raw", 0, 0, 240, 320);
  delay(1000);

  // Switch rotaion back to landscape
  tft.setRotation(1);
  tft.fillScreen(ILI9341_GREY);
  drawBMP("Inst_1.bmp", 0, 0, BU_BMP);
  delay(4000);

  // Test all standard rotations 0-3 with inverted top-down images
  // This is an orientation and colour encoding test bitmap created with MS Paint
  tft.setRotation(0);
  tft.fillScreen(ILI9341_GREY);
  drawBMP("test_dn.bmp", 0, 0, TD_BMP);
  delay(1000);

  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  drawBMP("test_dn.bmp", 0, 0, TD_BMP);
  delay(1000);

  tft.setRotation(2);
  tft.fillScreen(ILI9341_GREY);
  drawBMP("test_dn.bmp", 0, 0, TD_BMP);
  delay(1000);

  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  drawBMP("test_dn.bmp", 0, 0, TD_BMP);
  delay(1000);

  // Test all standard rotations 0-3 with small bottom-up images
  // Image used above was shrunk 50% with Paint
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);
  drawBMP("test_ups.bmp", 0, 0, BU_BMP);
  delay(1000);

  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  drawBMP("test_ups.bmp", 0, 0, BU_BMP);
  delay(1000);

  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
  drawBMP("test_ups.bmp", 0, 0, BU_BMP);
  delay(1000);

  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  drawBMP("test_ups.bmp", 0, 0, BU_BMP);
  delay(1000);

  // We could write a function to list and draw all bitmaps...
  // Could add "i" at start of all filenames to indicate an inverted
  // top-down image...
}


/***************************************************************************************
** Function name:           drawBMP
** Descriptions:            draw a BMP format bitmap to the screen
***************************************************************************************/

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size makes loading a little faster but the law of
// rapidly diminishing speed improvements applies.
// Suggest 8 minimum and 85 maximum (3 x this value is
// stored in a byte = 255/3 max!)
// A value of 8 is only ~20% slower than 24 or 48!
// Note that 5 x this value of RAM bytes will be needed
// Increasing beyond 48 gives little benefit.
// Use integral division of TFT (or typical often used image)
// width for slightly better speed to avoid short buffer purging

#define BUFF_SIZE 80

void drawBMP(char *filename, int x, int y, boolean flip) {
  if ((x >= tft.width()) || (y >= tft.height())) return;
  File     bmpFile;
  int16_t  bmpWidth, bmpHeight;   // Image W+H in pixels
  //uint8_t  bmpDepth;            // Bit depth (must be 24) but we dont use this
  uint32_t bmpImageoffset;        // Start address of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFF_SIZE];    // SD read pixel buffer (8 bits each R+G+B per pixel)
  uint16_t tftbuffer[BUFF_SIZE];       // TFT pixel out buffer (16-bit per pixel)
  uint8_t  sd_ptr = sizeof(sdbuffer); // sdbuffer pointer (so BUFF_SIZE must be less than 86)
  boolean  goodBmp = false;            // Flag set to true on valid header parse
  int16_t  w, h, row, col;             // to store width, height, row and column
  //uint8_t  r, g, b;   // brg encoding line concatenated for speed so not used
  uint8_t rotation;     // to restore rotation
  uint8_t  tft_ptr = 0;  // buffer pointer

  // Check file exists and open it
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.println(F("File not found")); // Can comment out if not needed
    return;
  }

  drawTime = millis(); // Save current time for performance evaluation, comment out if not needed

  // Parse BMP header to get the information we need
  if (read16(bmpFile) == 0x4D42) { // BMP file start signature check
    read32(bmpFile);       // Dummy read to throw away and move on
    read32(bmpFile);       // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    read32(bmpFile);       // Dummy read to throw away and move on
    bmpWidth  = read32(bmpFile);  // Image width
    bmpHeight = read32(bmpFile);  // Image height

    //if (read16(bmpFile) == 1) { // Number of image planes -- must be '1'
    // Only proceed if we pass a bitmap file check
    if ((read16(bmpFile) == 1) && (read16(bmpFile) == 24) && (read32(bmpFile) == 0)) { // Must be depth 24 and 0 (uncompressed format)
      //goodBmp = true; // Supported BMP format -- proceed!
      // BMP rows are padded (if needed) to 4-byte boundary
      rowSize = (bmpWidth * 3 + 3) & ~3;
      // Crop area to be loaded
      w = bmpWidth;
      h = bmpHeight;

      // We might need to alter rotation to avoid tedious pointer manipulation
      // Save the current value so we can restore it later
      rotation = tft.getRotation();
      // Use TFT SGRAM coord rotation if flip is set for 25% faster rendering
      if (flip) tft.setRotation((rotation + (flip<<2)) % 8); // Value 0-3 mapped to 4-7

      // We might need to flip and calculate new y plot coordinate
      // relative to top left corner as well...
      switch (rotation) {
        case 0:
          if (flip) y = tft.height() - y - h; break;
        case 1:
          y = tft.height() - y - h; break;
          break;
        case 2:
          if (flip) y = tft.height() - y - h; break;
          break;
        case 3:
          y = tft.height() - y - h; break;
          break;
      }

      // Set TFT address window to image bounds
      // Currently, image will not draw or will be corrputed if it does not fit
      // TODO -> efficient clipping, I don't need it to be idiot proof ;-)
      tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

      // Finally we are ready to send rows of pixels, writing like this avoids slow 32 bit multiply
      for (uint32_t pos = bmpImageoffset; pos < bmpImageoffset + h * rowSize ; pos += rowSize) {
        // Seek if we need to on boundaries and arrange to dump buffer and start again
        if (bmpFile.position() != pos) {
          bmpFile.seek(pos);
          sd_ptr = sizeof(sdbuffer);        }

        // Fill the pixel buffer and plot
        for (col = 0; col < w; col++) { // For each column...
          // Time to read more pixel data?
          if (sd_ptr >= sizeof(sdbuffer)) {
            // Push tft buffer to the display
            if (tft_ptr) {
              // Here we are sending a uint16_t array to the function
              tft.pushColors(tftbuffer, tft_ptr);
              tft_ptr = 0; // tft_ptr and sd_ptr are not always in sync...
            }
            // Finally reading bytes from SD Card
            bmpFile.read(sdbuffer, sizeof(sdbuffer));
            sd_ptr = 0; // Set buffer index to start
          }
          // Convert pixel from BMP 8+8+8 format to TFT compatible 16 bit word
          // Blue 5 bits, green 6 bits and red 5 bits (16 bits total)
          // Is is a long line but it is faster than calling a library fn for this
          tftbuffer[tft_ptr++] = (sdbuffer[sd_ptr++] >> 3) | ((sdbuffer[sd_ptr++] & 0xFC) << 3) | ((sdbuffer[sd_ptr++] & 0xF8) << 8);
        } // Next row
      }   // All rows done

      // Write any partially full buffer to TFT
      if (tft_ptr) tft.pushColors(tftbuffer, tft_ptr);
      drawTime = millis() - drawTime;
    } // End of bitmap access
  }   // End of bitmap file check
  //}     // We can close the file now

  bmpFile.close();
  //if(!goodBmp) Serial.println(F("BMP format not recognized."));
  tft.setRotation(rotation); // Put back original rotation
}

/***************************************************************************************
** Function name:           drawRAW
** Descriptions:            draw a 565 format 16 bit raw image file
***************************************************************************************/

// This function opens a ".raw" image file and displays it at the given coordinates.
// It is faster than plotting BMP images as the file is already in the correct
// format to pipe directly to the display.
// Uses same BUFF_SIZE as BMP function.
// The width and hsight of the image in pixels must be passed to the function
// as these parameters are not in the file

void drawRAW(char *filename, int16_t x, int16_t y, int16_t rawWidth, int16_t rawHeight) {
  File     rawFile;
  uint8_t  sdbuffer[2 * BUFF_SIZE];   // SD read pixel buffer (16 bits per pixel)

  // Check file exists and open it
  if ((rawFile = SD.open(filename)) == NULL) {
    Serial.println(F("File not found"));
    return;
  }

  drawTime = millis(); // Save current time for performance evaluation

  tft.setAddrWindow(x, y, x + rawWidth - 1, y + rawHeight - 1);

  // Work out how many whole buffers to send
  uint16_t nr = ((long)rawHeight * rawWidth)/BUFF_SIZE;
  while(nr--) {
    rawFile.read(sdbuffer, sizeof(sdbuffer));
    tft.pushColors(sdbuffer, BUFF_SIZE);
  }
  
  // Send any partial buffer
  nr = ((long)rawHeight * rawWidth)%BUFF_SIZE;
  if (nr) {
    rawFile.read(sdbuffer, nr<<1); // We load  2 x BUFF_SIZE bytes
    tft.pushColors(sdbuffer, nr);      // We send BUF_SIZE pixels
  }
    
  drawTime = millis() - drawTime;
  rawFile.close();
}

/***************************************************************************************
** Function name:           Support functions for drawBMP()
** Descriptions:            Read 16- and 32-bit types from the SD card file
***************************************************************************************/

// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File& f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File& f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

