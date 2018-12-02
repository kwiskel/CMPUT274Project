/*
 * SD card attached to SPI bus
 * MOSI -- ICSP pin 4 or DPin 50
 * MISO -- ICSP pin 1 or DPin 51
 * GND --  ICSP pin 6 
 * CS -- DPin 10 -- > default is 53
 * CLK -- ICSP pin 3 or DPin 52
 */

#include <Arduino.h>
#include <SD.h> // library to read/write to SD cards
#include<TMRpcm.h>  // Audio library allows asynchronous playback , uses Timer and PWM output from digital WAV signal


const int SDpin = 10; // SD Card Reader Chip Select Pin  -- default of 10 
TMRpcm audio; //new instance of TMRpcm


i


int32_t littleEndian(File file,int index, int n){
  int32_t value = 0;
  file.seek(index);
  for (int i = 0; i < n; i++){
    //Serial.println(int32_t(bytes[i]),BIN);
    value += (int32_t(file.read()) << (i*8));
    //Serial.println(value,BIN);
    
  }
  
  return value;
  
}

void readHeader(File file){
  String fields[] = {"ChunkID","ChunkSize", "Format", "SubChunk1ID", "Subchunk1Size", "AudioFormat", "NumChannels", "SampleRate",
  "ByteRate","BlockAlign","BitsPerSamples","SubChunk2ID","Subchunk2size"}; //44 bytes waved file RIFF header
  int fieldSize[] = {4,4,4,4,4,2,2,4,4,2,2,4,4};
  char endian[] = {'b','l','b','b','l','l','l','l','l','l','l','b','l'}; // RIFF byte order scheme
  int index = 0;
  for (int i = 0; i < sizeof(fieldSize)/sizeof(fieldSize[0]);i++){
    Serial.print(fields[i] + ": ");
    if(endian[i] == 'b'){ // big endian format 
      file.seek(index);
      for (int j = 0; j < fieldSize[i]; j++){
        Serial.print(j);
        Serial.print(char(file.read()));
      }
      index += fieldSize[i];
      Serial.println();
    }else if(endian[i] == 'l'){ // convert to big endian format and print
      Serial.println(littleEndian(file,index,fieldSize[i]));
      index += fieldSize[i];
    }
  }
}


void setup() {

  // Arduino communicates with SD card reader using SPI (SPI library needed) -- Arduino as master, Serial Peripheral Interface
  // Synchronous communcation, 

  audio.speakerPin = 6; // initialize audio output pin
  Serial.begin(9600);

  long startTime = 0;
  bool playStatus;
  int currentVolume = 5;
  Serial.println("\nFiles found on the card (name, date and size in bytes)");
  Serial.print("Begin SD: ");
  Serial.println(SD.begin(SDpin));
  File file = SD.open("/"); // instance of file class returned by SD.open(), checks for WAV files on SD card starting in root folder and checking all folders
  int songCount = searchWav(file,"/",0,0, {});  // counts number of wav files
  Serial.print("Total number of songs: ");
  Serial.println(songCount); // prints out number of songs found
  String songFiles[songCount];
  file.close();
  file = SD.open("/"); // opens file in root directory
  searchWav(file,"/",0,1,songFiles); // creates an array of song fileName (function edits array (don't need to return array)) 
  // -- calls same function twice so as to avoid dynamic array creation
  file.close(); // closes file and ensures data is physcially saved to SD card
  int songIndex = 0; // read from flash memory or EEPROM to save last playing song?
  file = SD.open("song.wav");
  //readHeader(file);  
  readHeader(file);
  printLength(songLength(file));
  file.close();
  Serial.println(getName(songFiles[0]));
  while(true){
    if(Serial.available()){
      char command = Serial.read();
      // replace with commands from LCD 
      if(command == 'p'){ // play and pause song
         playPause();
      }else if(command == '0'){ // play song 0
        startTime = millis();
        playSong(songFiles[0]);
        songIndex = 0;
      }else if(command == '1'){ // play song 1
        startTime = millis();
        playSong(songFiles[1]);
        songIndex = 1;
      }else if(command == 'd'){ //turn volume down
        changeVolume(currentVolume,0);
      }else if(command == 'u'){ //turn volume up
        changeVolume(currentVolume,1);
      }else if(command == 'e'){ //list elapsed time / total time
        Serial.print("Elapsed Time: ");
        Serial.println((millis()-startTime)/1000);
        printLength(songLength(SD.open(songFiles[songIndex])));
      }else if (command == 't'){ // print title of current song
        Serial.println(getName(songFiles[songIndex]));
      }else if(command == 'a'){ // print artist of current song
        Serial.println(getArtist(songFiles[songIndex]));
      }
    }
  }
  // read list information (metadata) use file.seek() to go to end
  
  
  /* read bytes from wav file
  while(file.available()){
    uint8_t byteR = file.read();
    Serial.println(byteR);
  }*/

  
  
}

void loop() {

  // need play/pause status (for display on LCD)
  //bool playStatus = false; // playing = true, paused = false
  // start time status
  //long startTime = 0;
  // need elapsed status
  //long elapsedSeconds = 0;
  
  // main function
  // receive input from board or other Arduino to perform functions
  
}

int songLength(File file){ // returns the length of the song in seconds
  int seconds = 0;
  // read number of data bytes from file header (SubChunk2Size) @ byte 40 following RIFF header formatting
  // also need to read byteRate (number of bytes per second)

  // call littleEndian function to calculate byteSize of data chunk
  int32_t byteSize = littleEndian(file,40,4);
  
  // byteRate
  int32_t byteRate = littleEndian(file,28,4);
  seconds = byteSize/byteRate;
  file.close();
  return seconds;
  
  
}

void printLength(int seconds){
  // prints out length of song in minutes and seconds
  int minutes = 0;
  while(seconds >= 60){
    minutes+=1;
    seconds -=60;
  }
  Serial.print("Minutes: ");
  Serial.print(minutes);
  Serial.print(" Seconds: ");
  Serial.println(seconds);
}


String getName(String f){
  // returns name of song 
  char fName[f.length()];
  f.toCharArray(fName,f.length());
  char songName[32];
  audio.listInfo(fName,songName,0);
  return (String(songName));
}

String getArtist(String f){
  // returns artist of song
  char fName[f.length()];
  f.toCharArray(fName,f.length());
  char artistName[32];
  audio.listInfo(fName,artistName,1);
  return (String(artistName));
}



int searchWav(File dir,String location, int count, int save, String arr[]){ 
  // dir = directory to check for wav files
  // location = file location from root directory
  // count = the number of wav files found
  // save = 1 --> save the file names to arr[], 0 --> simply count number of wav files (so as to use static array when allocating)
  // arr[] = the array of fileNames

  // prints only .wav files that are greater than 4 bytes --> audio files to play
  
  while(true){ // while still a file to read
    File entry = dir.openNextFile();
    if(!entry){
      return count; // returns count of wav files found so far
      //no more files
      break;
    }
    if(entry.isDirectory()){
      //Serial.println(entry.name());
      count = searchWav(entry,location + entry.name() + "/", count, save, arr);
    }
    String fName = String(entry.name());   
    if(entry.size() > 4096 && fName.substring(fName.length()-3,fName.length()) == "WAV"){  // greater than 4 bytes and wav format
      if(save == 1){ // if saving songs to array
        arr[count] = location + entry.name();
        Serial.println(arr[count]);
        Serial.println(entry.size());
      }
     
      count++;
    }
    entry.close();
  }
  
}

void playSong(String f){ // plays new song with file location as input
  char fName[f.length()];
  f.toCharArray(fName,f.length());
  if(audio.isPlaying()){ // check if playing another song and stop playback
    audio.stopPlayback();
  } 
   // play new file
  audio.play(fName);
}

void playPause(){ // pauses or plays song
  audio.pause();
}

void stopSong(){
  audio.stopPlayback();
}

int changeVolume(int currentVolume, int change){
  
  if(change == 1){ //up volume by 1
     if(currentVolume == 7){
      //max volume -- > no change
     }else{
      audio.volume(1); // increment volume
      currentVolume +=1;
     }
  }else if(change == 0){ // down volume by 1
    if(currentVolume == 0){
      // min volume --> no change
    }else{
      audio.volume(0); //decrement volume
      currentVolume -=1;
    }
  }
  return currentVolume;
}

int main(){
  setup();
  while(true){
    loop();
  }
}