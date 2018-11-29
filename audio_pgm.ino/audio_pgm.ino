//TMRpcm library is needed

#include "SD.h"
//#define SD_ChipSelectPin 10
#include "TMRpcm.h"
#include "SPI.h"

TMRpcm tmrpcm;

void setup()
{
tmrpcm.speakerPin=6;
Serial.begin(9600);
if(!SD.begin(10))
{
  Serial.println("SD fail");
  return;
}
tmrpcm.setVolume(5);
tmrpcm.play("song.wav");

}

void loop() {
  // put your main code here, to run repeatedly:

}
