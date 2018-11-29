
#include <Arduino.h>
#include <TMRpcm.h>

class Song {

	private:
		File file;
		String name;
		String type;
		String artist;
		int length[2];
		int completed[2];
	public:
		// Constructor with default values
		Song(File f = "", String n = "unknown", String t = "unknown", String a = "unknwown", int m = 0, int s = 0){
			file = f;
			name = n;
			type = t;
			artist = a;
			length = {m,s};

		}

		bool play(TMRpcm tmrcpm){
			tmrcpm.play(file);
		}
		bool pause(){
			tmrcpm.pause();
		}
		String getName(){
			return name;
		}
		String getType(){
			return type;
		}
		String getArtist(){
			return artist
		}
		int* getLength(){
			return length;
		}
		int* completed(){ // time completed in song
			return completed;
		} 
		int* remaining(){ // time remaining in song
			int remaining[] = {length[0]-completed[0], length[1]-completed[1]};
			return remaining;
		}
		

}