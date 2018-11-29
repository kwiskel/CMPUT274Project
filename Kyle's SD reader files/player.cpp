#include <Arduino.h>
#include <SPI.h> // Serial Peripheral Interface  -- not needed
#include <SD.h> // library to read from/write to SD card 
#include <TMRpcm.h> // Audio player library


TMRpcm tmrpcm; // object to use in sketch;
/*
SD card attached to SPI bus (Arduino MEGA)
MOSI -- ICSP 4 or pin 50 
MISO -- ICSP pin 1 or pin 51
CS -- pin 53 (default)
CLK - ICSP 3 or pin 52
*/

void printFiles(File dir){ // prints out the files in the input directory only (just needed for SONGS folder)
	while(true){
		File file = dir.openNextFile();
		if(!file){
			// no more files
			break;
		}
		if(file.size() > 4096){ // greater than 4 bytes -- actual song file
			Serial.println(file.name()); // print out file name
		}
		file.close(); // closes file
	}
}

void play_music(File song){ // going to use OOP to do this
	tmrpcm.play(song);
}

void audio_setup(){

	tmrpcm.speakerPin = 5; // 5,6,11,46 only on Mega
	tmrpcm.quality(1); // quality??
	tmrpcm.volume(5); // max volume
}

void sd_setup(){
	Serial.begin(9600);
	if(!SD.begin()){ // begins connection to card, 1 if working, 0 if not working
		Serial.println("Not properly connecting to SD card");
	}else {
		Serial.println("SD card working")
	}

	File file  = SD.open("/Songs");
	printFiles(file); // print files in directory
	//readSongs(file); // reads songs and creates an object of Song class for each
	Song song0("/Songs/song.wav", "Walk the Moon", "Pop", "Shut up and Dance", 3, 19); // file,type,name,artist,minutes,seconds
	song0.play(tmrpcm); // hopefully play Wav without library and have mp3 decompressor
	file.close();


}





int main(){
	audio_setup();
	sd_setup();
	
	
}