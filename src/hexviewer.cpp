/*
Software: Hexviewer 1.0
Author: Daniel Elgh
Date: 2012-12-12
Desc: Easy hexviewer for Windows and unix-like systems. 
Notice: it's a viewer, not an editor.

Copyright © All Rights Reserved

Known "bugs":
	* Inturrupting while colors is set,
	  returns to console in that color.
	  
	* Align might fail with some console
	  widths in hex_output function.
	  
	* File identifier might not read
	  zip file content properly due
	  to buffer-cuts of signatures.

*/
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>

#ifdef WIN32
	#define WIN32_MEAN_AND_LEAN 
	#include <windows.h>
	#pragma optimize("gsy", on)
	#pragma warning(disable:4996)
	HANDLE hStdout;
#else //#elif __GNUC__
	//typedefs if not 'defined' by your compiler.
	typedef unsigned long  DWORD;
	typedef unsigned short WORD;
	typedef unsigned char  BYTE;
#endif


//Function prototypes
std::streamoff file_identifier(const char *filename);
std::streamoff hex_output(const char *filename);
std::streamoff dual_output(const char *filename);
void color(const unsigned char ch);
void color(void);
void arguments(int argc, char **argv);

const int FNAME_LEN = 512;

//Declare global settings
std::streamoff (*output)( const char* ); //Pointer function
struct settings_t {
	std::streampos offset;
	bool pause;
	bool confirm;
	bool colors;
	bool fileid;
	bool fileid_only;
	unsigned short cli_width;
	unsigned short cli_height;
} setting;

int main(int argc, char **argv){
	//Declare local variables
	char fname[FNAME_LEN] = {0};
	std::streamoff fsize = 0;

	//Init default values for all settings.
	setting.offset		= 0;			//Default 0
	setting.pause		= true;		//Default true
	setting.confirm		= true;		//Default true
	setting.colors		= true;		//Default true
	setting.fileid		= false;		//Default false
	setting.fileid_only	= false;		//Default false
	setting.cli_width		= 80;			//Default on Windows 80
	setting.cli_height	= 25;			//Default on Windows 25

#ifdef WIN32
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
#endif
	output = &dual_output; //Default output method
	
	if (argc > 1) {
		strcpy(fname, argv[1]);
		arguments(argc, argv);
	} else {
		std::cout << " Hexviewer v1.0\nEnter filename: ";
		std::cin.getline(fname, sizeof(fname));
	}

	if (fname[0] == 0x22) {
		strncpy(fname, &fname[1], strlen(fname));
		fname[strlen(fname) - 1] = 0;
	}
	
	if (setting.fileid) fsize = file_identifier(fname);
	if (!setting.fileid_only) fsize = output(fname);

	color(); //Resets console color

	if (fsize == std::streamoff(-1)) {
		std::cout << "Unexpected error, file might not exist. File: " << fname << std::endl << "Press enter to exit";
		if (setting.confirm) std::cin.get();
		return EXIT_FAILURE;
	} else {
		std::cout << std::endl << " - EOF - " << std::dec << fsize << " bytes - Press enter to exit. ";
		if (setting.confirm) std::cin.get();
		return EXIT_SUCCESS;
	}
}

//OBSERVE; File identifier is poorly written and not 100% working. 
// Zip file info may lead to uncorrect output. Bigger buffer size decreases risk.
std::streamoff file_identifier( const char *filename ){
	std::fstream file;
	std::streamoff fsize;
	unsigned char buffer[1024*24] = {0};

	file.open(filename, std::fstream::in | std::ios::binary);
	if (file.is_open()) {
		//Get file size
		fsize = file.tellg(); file.seekg(0, std::ios::end);
		fsize = file.tellg() - fsize;	file.seekg(0, std::ios::beg);

		for (unsigned int i = 0; (i < sizeof(buffer) && !file.eof()); i++) buffer[i] = file.get();

		std::cout << "File type: ";
		if (!memcmp(&buffer[0], "MZ", 2))	//EXE
		{
			std::cout << "Windows executable or Dynamic-link library (DLL)" << std::endl;
		}
		else if (!memcmp(&buffer[0], "\x7F\x45\x4C\x46", 4))	//ELF
		{
			std::cout << "Unix binary and executable. Executable and Linkable Format (ELF, formerly called Extensible Linking Format)" << std::endl;
		}
		else if (!memcmp(&buffer[0], "\x89PNG\r\n\x1A\n", 8))	//PNG
		{
			std::cout << "PNG image" << std::endl;
		}
		else if (!memcmp(&buffer[0], "\xFF\xD8", 2))	//JPEG
		{
			std::cout << "JPEG image" << std::endl;
		}
		else if (!memcmp(&buffer[0], "BM", 2))	//BMP
		{
			std::cout << "BMP image" << std::endl;
			std::cout << "Width: "  << DWORD((buffer[18]) | (buffer[19] << 8) | (buffer[20] << 16 | (buffer[21] << 24))) << " pixels" << std::endl
					  << "Height: " << DWORD((buffer[22]) | (buffer[23] << 8) | (buffer[24] << 16 | (buffer[25] << 24))) << " pixels" << std::endl;
		}
		else if (!memcmp(&buffer[0], "Rar!", 4))	//RAR
		{
			std::cout << "Rar archive" << std::endl;
		}
		else if (!memcmp(&buffer[0], "PK\x03\x04", 4))	//ZIP
		{
			unsigned int n_files = 0;
			std::cout << "Zip archive\n -[Content]-" << std::endl;
			while (!file.eof()) {
				for (unsigned int i = 0; i < sizeof(buffer)-4; i++) {
					if (!memcmp(&buffer[i], "PK\x03\x04", 4)) {
						bool folder = false;
						if (DWORD(short(buffer[i+17]) + short(buffer[i+16]) + short(buffer[i+15]) + short(buffer[i+14])) == 0) folder = true;

						WORD zip_fname_len = WORD(buffer[i + 26] | buffer[i + 27] << 8);
						if (folder) std::cout << "Folder name: ";
						else std::cout << "File name: ";

						for(WORD n = 0; n < zip_fname_len; n++) std::cout << buffer[i + 30 + n];
						if (!folder) {
							std::cout << "\nCRC32: " << std::uppercase << std::hex
							<< short(buffer[i+17]) << short(buffer[i+16]) << short(buffer[i+15]) << short(buffer[i+14])
							<< std::dec;
						}
						std::cout << std::endl << std::endl;

						n_files++;
					}// end if
				}// end for 
				memset(buffer, '\0', sizeof(buffer));
				for (unsigned int i = 0; i < sizeof(buffer) && !file.eof(); i++) buffer[i] = file.get();
			}// end while !file.eof()
			std::cout << "Number of files in archive: " << n_files << std::endl;
		}
		else if (!memcmp(&buffer[0], "%PDF", 4))	//PDF
		{
			std::cout << "PDF document" << std::endl
					  << "Version: " << buffer[5] << buffer[6] << buffer[7] << std::endl;
		}
		else if (!memcmp(&buffer[0], "\x49\x44\x33", 3)) //MP3
		{
			std::cout << "MP3 file" << std::endl;

			memset(buffer, '\0', sizeof(buffer));
			file.seekg(-128, std::ios::end);
			for (unsigned int i = 0; i < sizeof(buffer) && !file.eof(); i++) buffer[i] = file.get();

			if (!memcmp(&buffer[0], "TAG", 3)) {
				std::cout << "Title: " << (char*)buffer+3 << std::endl;
				std::cout << "Artist: " << (char*)buffer+33 << std::endl;
				std::cout << "Album: " << (char*)buffer+63 << std::endl;
			}
		}
		else if (!memcmp(&buffer[0], "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8)) //OLECF
		{
			std::cout << "OLECF file" << std::endl
					  << "Notice! Many possibilities; Microsoft Office 97-2003 documents (DOC, XLS, PPT) or maybe a  \"Thumbs.db\" file. " << std::endl;
		}
		else if (!memcmp(&buffer[0], "\xFF\xFE", 2))	// Little endian
		{
			std::cout << "UTF-16 file format: Little endian" << std::endl;
		}
		else if (!memcmp(&buffer[0], "\xFE\xFF", 2))	// Big endian
		{
			std::cout << "UTF-16 file format: Big endian" << std::endl;
		} else {
			std::cout << "Unknown. Might be an normal text, crypted or a corrupted file.\n Notice that this list is not even close to complete." << std::endl;
		}

		std::cout << std::endl << "File size:\t" << fsize << " Bytes" << std::endl;
		if (fsize > 1073741824)//GB
			std::cout << "\t\t~" << (fsize / 1073741824) << " GB" << std::endl;
		else if (fsize > 1048576)//MB
			std::cout << "\t\t~" << (fsize / 1048576) << " MB" << std::endl;
		else if (fsize > 1024)//kB
			std::cout << "\t\t~" << (fsize / 1024) << " kB" << std::endl;

		file.close();
	}else return -1;
	return fsize;
}

std::streamoff hex_output( const char *filename ){
	std::fstream file;
	unsigned char buffer;
	std::streamoff counter = 0;

	file.open(filename, std::fstream::in | std::ios::binary);
	if (file.is_open()) {
		while (!file.eof()) {
			for (int i = 0; i < (setting.cli_width / 3 * (setting.cli_height+1) ) - 3; i++) {
				counter++;
				buffer = file.get();
				if (file.eof()) break;
				color(buffer);
				//printf("%02X ", buffer);
				std::cout << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << int(buffer) << " ";
				if ((i % (setting.cli_width / 3 + 1) == 0)) std::cout << "\b\b\b\b";
			}
			if (!file.eof() && setting.pause) std::cin.get();
		}
		file.close();
	}else return -1;
	return counter;
}

std::streamoff dual_output( const char *filename ){
	std::fstream file;
	std::streamoff fsize;
	std::streamoff counter = 0;
	std::streamoff address = 1 + setting.offset;
	unsigned char buffer[9] = {0};// memset(buffer, '\0', sizeof(buffer));
	//Open file
	file.open( filename, std::fstream::in | std::ios::binary);
	if (file.is_open()) {
		//Get filesize and rewind.
		file.seekg( setting.offset, std::ios::beg); fsize = file.tellg();
		file.seekg( 0, std::ios::end); fsize = file.tellg() - fsize;
		file.seekg( setting.offset, std::ios::beg);
		if (fsize < -1) { 
			std::cout << "ERROR!";
			std::cin.get();
			exit( EXIT_FAILURE );
		}
		std::cout << "   Address\t\tHex\t\t  Plain\t  (Page: " << std::dec << counter / 184 + 1 << "/" <<  fsize / 184 + 1 << ")" << std::endl;
		while (!file.eof()) {
			color();
			std::cout << " 0x" << std::setw(8) << std::setfill('0') << std::hex << std::uppercase << int(address) << " | "; address += 8;//Print address

			for (int i = 0; i < 8; i++) { // Read 8 bytes a time
				buffer[i] = file.get();
				if (file.eof()) { buffer[i] = 0; while(i++ < 8) std::cout << "\x20\x20\x20"; break;} //EOF padding
				color(buffer[i]); std::cout << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << int(buffer[i]) << "\x20";
				if (buffer[i] < 32) buffer[i] = 0x20; // Unviewable chars is replaced with _space_
				counter++;
			}
			color();
			std::cout << " | " << buffer << std::endl;
			memset(buffer, 0, sizeof(buffer));
			if (setting.pause) if ((counter % 23) == 0 && (counter != 0)) { std::cin.get(); std::cout << "   Address\t\tHex\t\t  Plain\t  (Page: " << std::dec << counter/184+1 << "/" <<  fsize/184+1 << ")" << std::endl;}
		}
		file.close();
	}else return -1;
	return counter;
}

void color( const unsigned char ch ){
	if (!setting.colors) return;
#ifdef WIN32
	if (ch >= 0 && ch <= 31)
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
	else if (ch >= 32 && ch <= 126)
		SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	else if (ch == 127)
		SetConsoleTextAttribute(hStdout, FOREGROUND_RED);
	else if (ch >= 128)
		SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#elif __GNUC__
	if (ch >= 0 && ch <= 31)
		std::cout << "\033[2;31m"; //RED
	else if (ch >= 32 && ch <= 126)
		std::cout << "\033[2;32m"; //GREEN
	else if (ch == 127)
		std::cout << "\033[2;33m"; //YELLOW
	else if (ch >= 128)
		std::cout << "\033[2;34m"; //BLUE
#endif
}

void color( void ){
	if (!setting.colors) return;
#ifdef WIN32
	SetConsoleTextAttribute(hStdout, 7);
#elif __GNUC__
	std::cout << "\033[2;37m"; //RESTORE
#endif
}

void arguments(int argc, char **argv){
	for(int i = 0; i < argc; i++){
		short a = 1;
		if(argv[i][0] == '-'){
			if(argv[i][1] == '-')a++;
			switch (argv[i][a]){
			case 'd':
				output = &dual_output;
				break;
			case 'x':
				output = &hex_output;
				break;
			case 'o':
				if (i == argc-1) break;
				setting.offset = atoi(argv[++i]) - 1;
				break;
			case 'n':
				if (argv[i][a+1] == 'p') setting.pause = false;
				if (argv[i][a+1] == 'c') setting.confirm = false;
				if (argv[i][a+1] == 'C') setting.colors = false;
				break;
			case 'c':
				if (i == argc-1) break;
				if (argv[i][a+1] == 'h') setting.cli_height = atoi(argv[++i]);
				if (argv[i][a+1] == 'w') setting.cli_width  = atoi(argv[++i]);
				break;
			case 'f':
				if (argv[i][a+1] == 'i') setting.fileid = true;
				if (argv[i][a+1] == 'i' && argv[i][a+2] == 'o') setting.fileid_only = true;
			break;
			case 'h':
			default:
				std::cout << "Hexviewer 1.0 - Help\n Syntax: hexviewer filename [-d|-x] [-o byte] [-np] [-nc] [-nC] \n\n"
						  << " -d\t\tDual output with hex and plain (default)\n -x\t\tOnly hex\n -o offset\tStart output at offset. Offset byte in decimal\n -np\t\tNo pause\n -nc\t\tNo exit confirm\n -nC\t\tNo colors\n -fi\t\tRun file identifier before printing hex\n -fio\t\tRun file identifier only (excluding hex printing)";
				exit(0);
				break;
			}
		}
	}
}
