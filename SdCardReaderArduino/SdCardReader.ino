/*
 * SD card attached to SPI bus
 * MOSI -- ICSP pin 4 or DPin 50
 * MISO -- ICSP pin 1 or DPin 51
 * GND --  ICSP pin 6 
 * CS -- DPin 10 -- > default is 53
 * CLK -- ICSP pin 3 or DPin 52
 */


#include <SPI.h> // Serial Peripheral Interface library, SD card reader communicates using SPI
#include <SD.h> // library to read/write to SD cards

const int SDpin = 10; // Chip Select Pin  -- default of 10 


// Setup using functions from SD utility librairies

Sd2Card card; //creates instance of Sd2Card --> creates a new card object
SdVolume cardVolume; //creates instance of SdVolume; --> to view the volume partition information
SdFile root; //creates instance of SdFile --> to views files on card



void setup() {

  // Arduino communicates with SD card reader using SPI (SPI library needed) -- Arduino as master, Serial Peripheral Interface
  // Synchronous communcation, 

  //SPI.begin(); // initialzes SPI busm SCK = LOW, MOSI = LOW, SS = HIGH --> Outputs, HIGH SS --> no communication, slave ignore master
  Serial.begin(9600);
  /*
  if(!card.init(SPI_HALF_SPEED, SDpin)) { // init() -- > initializes card with half clock rate and CS.
    // using initilization code from library -- > test to see if card working
    Serial.println("Not working");
    
  }else {
    Serial.println("Card Reader working");
  }

  Serial.println();
  Serial.print("Card type:    ");
  Serial.println(card.type()); // gets the card type

  if(!cardVolume.init(card)){
    Serial.println("Could not find FAT16/32 partition");
  }else{
    Serial.print("Volume type is:     FAT");
    Serial.println(cardVolume.fatType(),DEC); // Prints out card formatting in decimal
    float volumeSize;
    volumeSize = cardVolume.blocksPerCluster(); // clusters are collections of blocks
    volumeSize *= cardVolume.clusterCount(); // total number of blocks
    volumeSize /=2; //SD card blocks --> 512 bytes and 1024 bytes is 1 kb, so 2 blocks are 1 kb
    volumeSize/=(1024); // memory in mb
  
    Serial.print("Volum size in Gb: ");
    Serial.println((volumeSize/1024));
  }
  */
  Serial.println("\nFiles found on the card (name, date and size in bytes)");
  //root.openRoot(cardVolume); // opens root folder of the SD card
  //root.ls(LS_R);
  
  Serial.print("Begin SD: ");
  Serial.println(SD.begin(SDpin));
  File file = SD.open("/Songs"); // instance of file class returned by SD.open()
  /*Serial.println(file.name());
  while(file.available()){
    char byteR = file.read();
    Serial.print(byteR);
  }
  Serial.println();
  //Serial.println(file.available()); // number of bytes availbale for reading from file
  //Serial.println(char(file.peek())); // reads a byte from file without advancing to next one
  //Serial.println(file.position());
  //file.seek(2);
  Serial.println(file.size()); //size of file in bytes*/
  printDirectory(file); 
  file.close(); // closes file and ensures data is physcially saved to SD card
  
  /*
  Serial.print("Makes?: ");
  Serial.println(SD.mkdir("/Songs"));
  Serial.print("Directory?: ");
  Serial.println(SD.exists("/Songs"));
  Serial.print("REmoed?: ");
  Serial.println(SD.rmdir("/Songs"));
  Serial.print("Directory?: ");
  Serial.println(SD.exists("/Songs"));*/
  
  
  //SD.end();
  
  //File file = SD.open("/");// opens root directory
  
 
  //printDirectory(file,0); 
  //Serial.println("done");
  //Serial.println("attempting to create a file");

  // list all files in card with date and size
  //root.ls(LS_R | LS_DATE | LS_SIZE);

}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void printDirectory(File dir){

  // print only .mp3 files that are greater than 4 bytes
  //numTabs --> number of tabs when printing (directory and subdirectories layout)
  while(true){ // while still a file to read
    File entry = dir.openNextFile();
    if(!entry){
      //no more files
      break;
    }
    //for (uint8_t i = 0; i < numTabs;i++){
     //Serial.print('\t');
    //}
    if(entry.size() > 4096){// greater than 4 bytes
      Serial.println(entry.name());
    }
    entry.close();
  }
  
}
