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
//String songFiles[];
int songCount;
long startTime = 0;
bool playStatus;
int currentVolume = 5;
int previous[5]; // previous 5 played songs --> lowest index most recent played




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
        //Serial.print(j);
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


String * setup() {

  // Arduino communicates with SD card reader using SPI (SPI library needed) -- Arduino as master, Serial Peripheral Interface
  // Synchronous communcation, 
  
  audio.speakerPin = 6; // initialize audio output pin
  Serial.begin(9600);
  Serial3.begin(9600); // communicate with LCD Arduino
  randomSeed(analogRead(4)); // generate random sead --> for shuffle feature

  Serial.println("\nFiles found on the card (name, date and size in bytes)");
  Serial.print("Begin SD: ");
  Serial.println(SD.begin(SDpin));
  File file = SD.open("/"); // instance of file class returned by SD.open(), checks for WAV files on SD card starting in root folder and checking all folders
  songCount = searchWav(file,"/",0,0, {});  // counts number of wav files
  Serial.print("Total number of songs: ");
  Serial.println(songCount); // prints out number of songs found
  String songFiles[songCount];
  file = SD.open("/"); // opens file in root directory
  searchWav(file,"/",0,1,songFiles); // creates an array of song fileName (function edits array (don't need to return array)) 
  // -- calls same function twice so as to avoid dynamic array creation
  Serial.println();

  // connect to other Arduino -- (using code from Assignment#2)
  // "server" arduino is Arduino with Audio, "client" Arduino is arduino with LCD
  //serverSetup();
  //sendInitialInfo();

  return songFiles;
  
}

void loop(String songFiles[]) {
  
  
  while(true){
    if (Serial.available()){
      char byteR = Serial.read();
      Serial.println(byteR);
      if(byteR == 'p'){
        playPause();
        if(audio.isPlaying()){
          Serial.println("playing");
        }else{
          Serial.println("paused");
        }
      }else if(byteR == 's'){ //play new song --> next byte is index of song
        while(!Serial.available()){
          
        }
        char song = Serial.read();
        int index = (song)-'0'; // convert char to int 
        Serial.println(index);
        playSong(songFiles[index]); // play new song (also shift previous);
        shiftPrevious(int(song));
      }else if(byteR == 'u'){ // volume up
        currentVolume = changeVolume(currentVolume,1);
        Serial.print("Current volume: ");
        Serial.println(currentVolume);
      }else if(byteR == 'd'){ // volume down
        currentVolume = changeVolume(currentVolume,0);
        Serial.print("Current volume: ");
        Serial.println(currentVolume);
      }
    }
  }
  
  
}

void clientSetup(){ // LCD Arduino Setup
  
}

void serverSetup(){ // Audio Arduino Setup
  
}

void sendInitialInfo(char type, String files[]){ // send any information to other Arduino (song information)
 // Once setup send begin signal
 Serial3.write('b');
 // send all song names + artists in order
 for (int i = 0; i < songCount; i++){
  // send 's' then Song name then carriage return and 'a' then artist name
  Serial3.write('s');
  char sName = getName(files[i]);
  sendSerial3(sName);// send sName
  Serial3.write('\r');
  Serial3.write('a');
  char aName = getArtist(files[i]);
  sendSerial3(aName);// send aName;
 }
}

void sendSerial3(char bytes[]){
  int aSize = sizeof(bytes)/sizeof(bytes[0]); 
  for (int i = 0;  i < aSize; i ++){
    Serial3.write(bytes[i]);
  }
}



// following 2 functions can be implemented in LCD code (arduino)
void shuffleSong(){
  //pick random song next (based on previous songs played)
  int nextRandom = 0;
  while(true){
    // generate random number between 0 and (songCount-1) -- index of songs 
    // random number is not last 5 played songs unless less than 5 songs*
    nextRandom = random(0,songCount); // between 0 and songCount-1 
    if(songCount <=5){
      break;
    }else{ // more than 5 songs
      for (int i =0; i < 5; i++){
        if (previous[i] == nextRandom){
          // generate another random
          i = 5;
        }
      }
    }
    
  }
}

void shiftPrevious(int last){ //shifts data in previous songs (when starting playing song);
  for (int i = 4; i > 1; i++){
    previous[i] = previous[i-1]; // shift and loses 6th recently played
  }
  previous[0] = last;
  
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


char* getName(String f){
  // returns name of song 
  char fName[f.length()];
  f.toCharArray(fName,f.length()+1);
  //Serial.print("Fname: ");
  //Serial.println(fName);
  char songName[32];
  audio.listInfo(fName,songName,0);
  return ((songName));
}

char* getArtist(String f){
  // returns artist of song
  char fName[f.length()];
  f.toCharArray(fName,f.length()+1);
  char artistName[32];
  audio.listInfo(fName,artistName,1);
  return ((artistName));
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
  f.toCharArray(fName,f.length()+1);
  if(audio.isPlaying()){ // check if playing another song and stop playback
    Serial.println("audioplaying");
    
    audio.stopPlayback();
  } 
  Serial.println(fName);
   // play new file
  audio.play(fName);
  Serial.print("Playing Song: ");
  Serial.println(getName(f));
  Serial.print("Artist: ");
  Serial.println(getArtist(f));
  f.toCharArray(fName,f.length()+1);
  File file = SD.open(fName);
  //Serial.println(file.name());
  //readHeader(file);  // prints out header info
  printLength(songLength(file)); // prints out length of song in minutes/seconds
  file.close();
}

void playPause(){ // pauses or plays song
  audio.pause();
}

void stopSong(){
  audio.stopPlayback();
}

int changeVolume(int currentVolume, int change){
  
  if(change == 1){ //up volume by 1
     if(currentVolume == 6){
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
  String songFiles[] = setup();
  while(true){
    loop();
  }
  return 0;
}