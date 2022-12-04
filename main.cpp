// g++ main.cpp cpptk.cc base/cpptkbase.cc -IC:\tools\msys64\mingw64\include\tcl8.6 -ltcl86 -IC:\tools\msys64\mingw64\include\tk8.6 -ltk86 -IC:\C++\Boost -mwindows
#include "cpptk.h"
#include <iostream>
#include <fstream>

using namespace Tk;
using namespace std;

// Hex Colors
const char headBgColor[9] = "#353535\0";
const char fgColor[9] = "#EEEEEE\0";
const char bgColor[9] = "#555555\0";

// Bools for if scrollbars are currently showing
bool showRight = false, showBottom = false;

// ids
const char* rightS = ".s\0";
const char* botS = ".sx\0";
const char* textWid = ".t\0"; 
string drop;

// file to save to
string toSave = "";

// saves the path to current file so user can save instead of save as
void saveCurrentFile();
void updateSavePath(const char* c){
	if(toSave == ""){
		drop << add(command) -menulabel("Save") -command(saveCurrentFile);
	}
	toSave = string(c);
}

// Opens file and puts it into text widget
void openFile(const char* filename){	
	ifstream f(filename);
	string fileContent(istreambuf_iterator<char>(f.rdbuf()), istreambuf_iterator<char>());
	updateSavePath(filename);
	
	// put the file content into the text widget
	textWid << deletetext(txt(1,0), Tk::end);
	textWid << insert(Tk::end, fileContent);
	textWid << edit(Tk::reset); // clear undo stack
}

// wrapper for openFile that pops up a file search
void openFileTk(){	
	string fileName(tk_getOpenFile());
	openFile(fileName.c_str());
}

// Writes text widget contents into file
void saveFile(const char* filename){
	try{
		string content(textWid << Tk::get(txt(1,0), Tk::end));
		ofstream f(filename);
		f << content;
		updateSavePath(filename);
	}catch(exception const &e){
		cerr << "Couldn't save to file " << filename << '\n' << e.what();
	}
}

// Wrapper for saveFile that pops up a file search
void saveFileAs(){
	string fileName(tk_getSaveFile());
	saveFile(fileName.c_str());
}

// Wrapper for saveFile that saves to last opened file
// Only shows up if either file opened by user or user did Save As
void saveCurrentFile(){
	saveFile(toSave.c_str());
}

// save by ctrl+s
void saveKeyBinding(){
	if(toSave == "") return;
	saveCurrentFile();
}

// prints info I want to see for debugging
void printDebug(){
	update();
	string temp(".sx" << Tk::get());
	cout << "Horizantal scrollbar loc: " << temp << '\n';
	temp = string(".s" << Tk::get());
	cout << "Vertical scrollbar loc: " << temp << '\n';
}

// checks if text breaks out of screen and shows scroll bars if so
void scrollBarChecker(){
	update(); // update to make sure scrollbar pos is correct
	string scrollPosY(rightS << Tk::get());
	string scrollPosX(botS << Tk::get());
	bool yIsBig = scrollPosY != "0.0 1.0", xIsBig = scrollPosX != "0.0 1.0";
	if(yIsBig != showRight){
		if(yIsBig){
			if(showBottom){
				pack(rightS) -side(Tk::right) -Tk::fill(y) -Tk::before(textWid) -Tk::before(".sx");
			}else{
				pack(rightS) -side(Tk::right) -Tk::fill(y) -Tk::before(textWid);
			}
		}else{
			pack(Tk::forget, rightS);
		}
		showRight = yIsBig;
	}
	if(xIsBig != showBottom){
		if(xIsBig){
			pack(botS) -side(Tk::bottom) -Tk::fill(x) -Tk::before(textWid);
		}else{
			pack(Tk::forget, botS);
		}
		showBottom = xIsBig;
	}
}

int main(int argc, char *argv[])
{
	const bool DEBUG = true;
	try{
		init(argv[0]);

		// Menu
		frame(".mbar") -bg(headBgColor);
		pack(".mbar") -Tk::fill(x);
		
		menubutton(".mbar.file") -text("File") -submenu(".mbar.file.m") -bg(headBgColor) -fg(fgColor) -activeforeground(fgColor) -activebackground(bgColor);
		pack(".mbar.file") -side(Tk::left);

		//button(".mbar.exit") 
		//pack(".mbar.exit") -side(Tk::right);
		
		drop = string(menu(".mbar.file.m") -bg(headBgColor) -fg(fgColor) -activeforeground(fgColor) -activebackground(bgColor));
		if(DEBUG) drop << add(command) -menulabel("Debug Info") -command(printDebug);
		drop << add(command) -menulabel("Open") -command(openFileTk);
		drop << add(command) -menulabel("Save As") -command(saveFileAs);

		// create the text widget with its scrollbar
		textw(textWid) -wrap(none) -xscrollcommand(".sx set") -yscrollcommand(".s set") -bg(bgColor) -fg(fgColor) -undo(true);
		scrollbar(rightS) -command([](const vector<string>& cmd) { textWid << yview(cmd); }) -bg(headBgColor) -highlightbackground(headBgColor) -activebackground(headBgColor);
		scrollbar(botS) -orient(horizontal) -command([](const vector<string>& cmd) { textWid << xview(cmd); }) -bg(headBgColor);
		pack(rightS) -side(Tk::right) -Tk::fill(y);
		pack(botS) -side(Tk::bottom) -Tk::fill(x);
		pack(textWid) -expand(true) -Tk::fill(Tk::both);
		pack(Tk::forget, rightS);
		pack(Tk::forget, botS);
		Tk::bind(textWid, "<KeyPress>", scrollBarChecker);
		Tk::bind(".", "<Control-KeyPress-s>", saveKeyBinding);
		Tk::bind(".", "<Configure>", scrollBarChecker);

		if(argc > 1){
			// opened a file
			try{
				openFile(argv[1]);
			}catch(exception const &e){
				cerr << "Failed to open file " << argv[1] << '\n' << e.what();
			}
			scrollBarChecker();
		}		
		
		runEventLoop();
	}
	catch (exception const &e){
		cerr << "Error in main: " << e.what();
	}
}
