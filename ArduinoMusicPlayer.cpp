// SD code from the sd examples.

#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <TouchScreen.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <SD.h>
#include <SPI.h>
#include <math.h>

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 6
#define TFT_MOSI 51
#define TFT_MISO 50
#define TFT_CLK 52
#define YP A2  // Touchscreen pin
#define XM A3  // Touchscreen pin
#define YM 5   // Touchscreen pin
#define XP 4   // Touchscreen pin

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
File songFile;


void setup() {
  init();
  Serial.begin(9600);
  Serial3.begin(9600);

  // Initialize the display
  tft.begin();
  tft.setRotation(1);
  /*
  // Initialize the SD card
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  */
}


void drawPlayButton(bool pressed, uint8_t position) {

  // If the button is being pressed, we need to make it print quickly, so
  // it will just print a new colour of text on top of the old text.
  if(pressed) {
    tft.fillTriangle(304, 64+(position*46), 280, 52+(position*46), 280, 76+(position*46), 0xF974);
  }

  else {
    tft.fillRoundRect(198, 44+(position*46), 116, 40, 8, 0xFFFF);
    tft.setCursor(202, 74+(position*46));
    tft.setFont(&FreeSansBold18pt7b);
    tft.setTextColor(0x0000);
    tft.print("Play");
    tft.fillTriangle(304, 64+(position*46), 280, 52+(position*46), 280, 76+(position*46), 0x0000);
  }
}


void drawPlayer(uint8_t songNumber) {
  // Clears the screen
  tft.fillScreen(0x0000);

  // Back Button
  tft.fillTriangle(6, 20, 26, 6, 26, 33, 0xF974);
  tft.fillTriangle(16, 20, 26, 6, 26, 33, 0x0000);

  // Draw divider bar
  tft.fillRect(6, 38, 308, 2, 0xF974);

  // Music Player Title
  tft.setCursor(32, 32);
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(0xFFFF);
  tft.print("Arduino Tunes");

  // Draw visuallizer bar
  tft.fillRect(6, 198, 308, 2, 0xF974);

  const char *songList[2][4] = {
    {"Rick Astley", "Loud Luxury & anders", "Martin Garrix & David Guetta", "Gryffin & Zohara"},
    {"Never Gonna Give You Up", "Love No More", "So Far Away", "Remember"}
  };

  // Song Information
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(0xFFFF);
  tft.setTextWrap(false);

  // Draw Artist Name
  tft.setCursor(6, 217);
  tft.print(songList[0][songNumber - 1]);

  // Draw Song Name
  tft.setCursor(6, 235);
  tft.print(songList[1][songNumber - 1]);

  // Draw all of the bars at full magnitude
  /*
  for(int i = 0; i < 12; i++) {
    for(int j = 1; j <= 15; j++) {
      tft.fillRect(30+(i*22), 198-(j*8), 19, 4, 0xF974);
    }
    //tft.fillRect(28+(i*22), 46, 21, 156, 0xF974);
  }
  */
}


void drawHomeScreen() {
  // Clears the screen
  tft.fillScreen(0x0000);

  // Music Player Title
  tft.setCursor(6, 32);
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(0xFFFF);
  tft.print("Arduino Tunes");

  // Draw divider bar
  tft.fillRect(6, 38, 308, 2, 0xF974);

  // Draw Songs to select from
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(0xFFFF);
  tft.setTextWrap(false);

  const char *songList[2][4] = {
    {"Rick Astley", "Loud Luxury & anders", "Martin Garrix & Dav...", "Gryffin & Zohara"},
    {"Never Gonna Give Y...", "Love No More", "So Far Away", "Remember"}
  };

  for(int i = 0; i < 4; ++i) {
    tft.setCursor(6, 56+(i*46));
    tft.print(songList[0][i]);
    tft.setCursor(6, 74+(i*46));
    tft.print(songList[1][i]);
  }

  // Draw play button
  for(int i = 0; i < 4; ++i)
    drawPlayButton(false, i);
}


void drawVisualizer(uint8_t* bars, uint8_t* oldBars) {
  /*
  Updates the viualizer based on the old state and the current state
  */

  // This is the gradient of colours for the visualizer.
  int colours[15] = {0x003F, 0x1039, 0x203B, 0x3039, 0x4836, 0x5814, 0x6812, 0x780F, 0x900D, 0xA00B, 0xB009, 0xC806, 0xD804, 0xE802, 0xF800};

  int heightDifference = 0;

  for(int i = 0; i < 12; ++i) {
    // Calculates the new position of the bar
    heightDifference = bars[i] - oldBars[i];

    // If the new position of the bar is higher, print a rectangle one position
    // higher than the last for every unit it is higher (and use the colour
    // for the higher position). Draws from bottom to top.
    if(heightDifference > 0) {
      for(int j = oldBars[i] + 1; j <= bars[i]; j++) {
        // 30, 198 is the top left corner of the first rectangle. (i*22) is
        // the distance between each bar, (j*8) is the vertical distance
        // between rectangles.
        tft.fillRect(30+(i*22), 198-(j*8), 19, 4, colours[j-1]);
      }
    }

    // If the position is lower, so the same as shown above, but draw black
    // rectangles instead. Draws from top to bottom.
    if(heightDifference < 0) {
      for(int j = bars[i] + 1; j <= oldBars[i]; j++) {
        tft.fillRect(30+(i*22), 198-(j*8), 19, 4, 0x0000);
      }
    }

    oldBars[i] = bars[i];
  }
  // We need to clear the array from memory after every draw
  // to circumvent overflows.
  delete[] bars;
}


uint8_t* fft() {
  static int fftCounter = 0;
  uint8_t* dummy = new uint8_t[12];
  srand(fftCounter);
  for(int i = 0 ; i < 12; ++i){
    dummy[i] = rand()%16;
  }
  ++fftCounter;
  if(fftCounter > 100) {
    fftCounter = 0;
  }
  return dummy;
}


void displayPage(bool inHomeScreen) {
  /*
  This funtion is passed the state that the program is in. It first draws
  the home screen and awaits for a button to be pressed. Once a button
  has been pressed, we exit the home screen and go to the visualizer, where
  fft is called and the visualizer is redrawn until the back button is pressed
  in which the funtion will terminate. It is expected that this funtion is
  called again so that the home screen is entered.
  */
  uint8_t* bars = fft();

  // Home Screen State

  // TODO Pass the recieved "songList" into the drawHomeScreen() so that
  // We can draw the song names
  drawHomeScreen();
  while(inHomeScreen) {

    // Waits for a play button to be pressed.
    TSPoint p = ts.getPoint();
    if(p.z > ts.pressureThreshhold) {
      if((607 < p.x) && (p.x  < 743) && (585 < p.y)) { // Play Button 1
        Serial.println("Song 1 selected.");

        // Draws the pink play arrow on the play button at position 0 to give
        // user feedback
        drawPlayButton(true, 0);
        inHomeScreen = false;
        drawPlayer(1);
      }

      else if((444 < p.x) && (p.x < 607) && (585 < p.y)) { // Play Button 2
        Serial.println("Song 2 selected.");

        // Draws the pink play arrow on the play button at position 1 to give
        // user feedback
        drawPlayButton(true, 1);
        inHomeScreen = false;
        drawPlayer(2);
      }

      else if((295 < p.x) && (p.x < 444) && (585 < p.y)) { // Play Button 3
        Serial.println("Song 3 selected");

        // Draws the pink play arrow on the play button at position 2 to give
        // user feedback
        drawPlayButton(true, 2);
        inHomeScreen = false;
        drawPlayer(3);
      }

      else if((p.x < 295) && (585 < p.y)) { // Play Button 4
        Serial.println("Song 4 selected");

        // Draws the pink play arrow on the play button at position 3 to give
        // user feedback
        drawPlayButton(true, 3);
        inHomeScreen = false;
        drawPlayer(4);
      }
    }
  }

  // Visualizer State
  uint8_t oldBars[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
  for(int i = 0; i < 12; ++i)
    bars[i] = 0;

  while(!inHomeScreen) {
    // Calculate the fourier transform and pass that to the visualizer to draw.
    bars = fft();
    drawVisualizer(bars, oldBars);

    // Test for the display being pressed on the top left corner (the back
    // button).
    TSPoint p = ts.getPoint();
    if((p.z > ts.pressureThreshhold) && (p.x > 780) && (p.y < 150)) {
      inHomeScreen = true;
    }
  }
}


int main() {
  setup();
  bool inHomeScreen = true;
  // Read from the Serial3 to generate a 2D array "songList", in which you have
  // songList[Artist1, Artist2, Artist3, Artist4][Song1, Song2, Song3, Song4]
  // Pass this array to "displayPage()".

  while(true) {
    displayPage(inHomeScreen);
  }

  return 0;
}
