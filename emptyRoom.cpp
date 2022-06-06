/* this program will find all the empty rooms in any building given its name and the amount of time needed to stay in the room
 * for future comments that reference it, an example of the 12hr format looks like this: "11:00am" or "1:00pm"
 * enjoy!
 * -Eric Wang
 *
 */
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <vector>
#include <algorithm>

using namespace std;
vector<string> emptyRooms;

// time_t is a arithmetic unit in c++ that needs to be converted into a 12hr format
// this function converts it
string secondsTo12HrFormat(time_t t) {
	string currentTime = string(ctime(&t));

	string hrsMins = currentTime.substr(11,5);

	if(stoi(hrsMins.substr(0,2)) < 12) {
		hrsMins += "am";
	} else {
		if(stoi(hrsMins.substr(0,2)) > 12) {
			hrsMins = to_string(stoi(hrsMins.substr(0,2))-12) + hrsMins.substr(2,3);
		}
		hrsMins += "pm";
	}
	return hrsMins;
}

// the time string from Lou's list looks like "TuTh 12:30pm - 1:45pm"
// but we want to process it to get startTime = "12:30pm" and endTime = "1:45pm" to make it easier to compare time
// that's what findStartTime() and findEndTime() do
string findStartTime(string time) {
	int startTimeIndex = time.find_first_of("0123456789");
	string startTime = time.substr(startTimeIndex, 6);
	if(startTime[startTime.length()-1] == 'a' || startTime[startTime.length()-1] == 'p') startTime += "m";
	return startTime;	
}

string findEndTime(string time) {
	int endTimeIndex = time.find_first_of("-") + 2;
	string endTime = time.substr(endTimeIndex, time.find_last_of("m"));
	return endTime;	
}

// prints any vector, inserts a new line when the remainder of the index divided by 4 is 3 (the math is weird but the format looks good)
void printVector(vector<string> vec) {
	for(int i = 0; i < vec.size(); i++) {
		cout << vec.at(i) << "   ";
		if(i % 4 == 3) cout << endl;
	}
	cout << endl;
}

// reads data file and populates roomTime and roomLocation while intentionally NOT including the below:
// class with To Be Announced meeting times, meetings outside of the input variable named "building" (the one the user wants to stay in), 
void getRoomTimeAndLocationFromFile(vector<string> &roomTime, vector<string> &roomLocation, string building, string weekday) {
	ifstream myFile;
	myFile.open("newclassData.csv");

	string time, location;
	while(myFile.good()) {
		getline(myFile, time, ',');
		getline(myFile, location, '\n');
		
		if(time.find("TBA") == string::npos && location.find("TBA") == string::npos) {
			if(
				equal(building.begin(), building.end(), location.begin()) 
				&& (weekday == "Su" || weekday == "Sa" || time.find(weekday) != string::npos)
			){	
				roomTime.push_back(time);
				roomLocation.push_back(location);
			}

		}
					
	}
	myFile.close();

}

// given a building and a time needed to stay there, prints the empty rooms in that building that are open during the specified time
void findRoom(string building, int timeNeededInMins) {
	time_t t = time(0); // The number of seconds since 00:00 hours, Jan 1, 1970 UTC, it gets its own special unit in C++
	string weekday = string(ctime(&t)).substr(0,2); // The day of the week in format of MoTuWeThFrSaSu
	string hrsMins = secondsTo12HrFormat(t); // Take the current time in seconds and convert it to a 12hr format like 12:00am or 1:00pm
	string leaveHrsMins = secondsTo12HrFormat(t+timeNeededInMins*60); // Calculate the time the user will leave in seconds, then convertit to 12hr format 

	vector<string> roomTime; // list of class meeting times
	vector<string> roomLocation; // list of class location (matches with class meeting time such that roomTime.at(x) and roomLocation.at(x) refers to the same class 
	vector<string> nonEmptyRooms; // list of rooms that have meeting times that overlap with the time the user would like to stay, and thus are not empty
	getRoomTimeAndLocationFromFile(roomTime, roomLocation, building, weekday); // this function reads the data file and populates roomTime and roomLocation	
	
	// startTime is time the meeting starts in 12hr format, endTime is time the meeting ends in 12hr format
	// hrsMins is the current time in 12hr format, leaveHrsMins is the time the user is planning to leave in 12hr format
	// loops through the list of roomTime, checks if period from startTime to endTime conflicts with period from hrsMins to leaveHrsMins,
	// if the periods conflict for a roomTime.at(x), then add the corresponding roomLocation.at(x) to the nonEmptyRoom list
	for(int i = 0; i < roomTime.size(); i++) {
		string startTime = findStartTime(roomTime.at(i));
		string endTime = findEndTime(roomTime.at(i));
		if(	(startTime <= leaveHrsMins && startTime >= hrsMins) 
			|| (endTime <= leaveHrsMins && endTime >= hrsMins)
			|| (startTime <= hrsMins && endTime >= leaveHrsMins)
			&& find(nonEmptyRooms.begin(), nonEmptyRooms.end(), roomLocation.at(i)) ==  nonEmptyRooms.end()
		) {
			nonEmptyRooms.push_back(roomLocation.at(i));
		}

	}

	// loops through the entire list of roomLocation (all rooms at the school), then push the rooms that are NOT IN nonEmptyRooms to the list of emptyRooms
	for(int i = 0; i < roomLocation.size(); i++) {
		if( find(nonEmptyRooms.begin(), nonEmptyRooms.end(), roomLocation.at(i)) ==  nonEmptyRooms.end()
				&& find(emptyRooms.begin(), emptyRooms.end(), roomLocation.at(i)) == emptyRooms.end()) {
			emptyRooms.push_back(roomLocation.at(i));
		}
	}
	printVector(emptyRooms); // this function prints the any list in neat rows. It does not return anything 	
}

int main() { // Enter building name with correct capitalization and the number of minutes needed to stay in the room
	string buildingName;
	int timeWantToStay;
	while(true) { // infinite loop so the user don't have to restart the program to search a second building
		cout << "---------------------------------------------------------------------------------" << endl;
		cout << "Enter the building name using the Lou's List format" << endl;
		cout << "(captalize every word, abbreviate words like Engineering, etc), followed by a '/'" << endl; 
		cout << "then followed by the number of minutes you want to stay in a room." << endl;
		cout << "The format should look like this: New Cabell Hall/120 or Mechanical Engr Bldg/180" << endl;
		getline(cin, buildingName, '/');
		cout << "Start of empty room list (list is empty if none found):" << endl;
		findRoom(buildingName, timeWantToStay);
		cout << "End of room list. Ctrl-C to exit." << endl;
		emptyRooms.clear();
	} 
}
