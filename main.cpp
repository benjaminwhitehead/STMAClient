//
//  main.cpp
//  STMAClient
//
//  Created by Benjamin Whitehead on 12/21/15.
//  Copyright © 2015 Benjamin Whitehead. All rights reserved.
//
#include "crc2.h"
#include "download.h"
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

//#include <boost/filesystem.hpp>
//#include <boost/filesystem/fstream.hpp>
//#include "boost.h"


#if IBM
	#include <direct.h>
//    #include <WinSock2.h>
	#define GetCurrentDir _getcwd
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#define DEBUGS 1
//#define DEBUG_ONLY 1
using namespace std;


//typedef std::string string;
const std::string versionNumber = "1.0.1";
FILE *  gSTMAPreferenceFile_ptr;

// set these according to OS
std::string arguments;

std::string settingsFile;
std::string doNotAllowUpdates;  // tmp file created by plugin to indicate not to update.
std::string fileListFile;
std::string fileListUpFile;
std::string fileChangeList;
std::string fileFinalChangeList;
int XPlane = 1;  // if set need to prepend strings

vector<std::string> filelist;
vector<int> filelistCrc;

vector<std::string> filelistUp;
vector<int> filelistUpCrc;

vector<std::string> fileListDownload;
vector<std::string> ignoreList;
vector<int> deleteList;
	
int ReadFileList(std::string filename);
int writeFileList(std::string filename);
void displayFiles(void);
int stop(void);
int init(void);
int download(std::string myurl, std::string outfilename);
int modPath(std::string & filename);

char serverLocation[512];
char serverUpdate[512];
char serverPath[512];
int allowUpdates = 0;
int overrideAllowUpdates = 0;
int test_dirs();

int debugOut(char * text) {
#if DEBUGS
    ofstream myfile;
    myfile.open((char *)fileChangeList.c_str(), std::ofstream::out | std::ofstream::app);
    if (myfile.is_open()) {
                myfile << text << "\n";
                printf("%s\n",text );
    }
    myfile.close();
#endif
    return 0;
}
int debugOut(std::string text) {
#if DEBUGS
    ofstream myfile;
    myfile.open((char *)fileChangeList.c_str(), std::ofstream::out | std::ofstream::app);
    if (myfile.is_open()) {
                myfile << text << "\n";
                 printf("%s\n",(char *)text.c_str() );
   }
    myfile.close();
#endif
    return 0;    
}
int debugOut(std::string text1, std::string text2) {
#if DEBUGS
    ofstream myfile;
    stringstream conc;
    conc << text1 << text2;
   std::string output = conc.str();

    myfile.open((char *)fileChangeList.c_str(), std::fstream::app);

    if (myfile.is_open()) {
                myfile << output << "\n";
                printf("%s\n",(char *)output.c_str() );
    }
    myfile.close();
#endif
    return 0;    

}

unsigned checksum(void *buffer, size_t len, unsigned int seed)
{
      unsigned char *buf = (unsigned char *)buffer;
      size_t i;

      for (i = 0; i < len; ++i)
            seed += (unsigned int)(*buf++);
      return seed;
}

long FileSize(FILE *input)
{

  long fileSizeBytes;
  fseek(input, 0, SEEK_END);
  fileSizeBytes = ftell(input);
  fseek(input, 0, SEEK_SET);

  return fileSizeBytes;
}

int modPath (std::string & filename) {
    stringstream oss;
   std::string oss_s;
    if (XPlane == 1) {
        // prepend path
        //#if IBM
        //oss << "\\Resources\\plugins\\XAdventures\\XAdventure_data\\";
        //#else
        //oss << "./Resources/plugins/XAdventures/XAdventure_data/";
        oss << arguments;
        
        //#endif
    }
    oss << filename;
    oss_s = oss.str();
        
    filename = oss_s;

    return 0;
}

int test_dirs(void) {
// use the boost library to find folders, etc.
 //   boost::filesystem::path full_path( boost::filesystem::current_path() );
    //fprintf(stderr, "Full Path is:%s\n", full_path );
    
 //   stringstream oss;
 //   oss << full_path;
 //  std::stringoss_s = oss.str();
    
 //   debugOut(oss_s);
    return 0;
}

int cksum(std::string filename)
{
    modPath(filename);
    FILE *fp = fopen((char *)filename.c_str(), "rb");
    if(fp == NULL)
    {
        debugOut("\ncksum Error opening file %s\n", filename);
        return 0;
    }
    int crc;
    long bufsize = FileSize(fp), result;
    unsigned char *buffer = new unsigned char[bufsize];

    size_t len;
    
    len = fread(buffer, sizeof(char), bufsize, fp);
    //debugOut( "bytes read\n", len);
    crc = checksum(buffer,len,0);
    
    //stringstream oss2;
    //oss2 << "The checksum of " << filename << " is " << crc;
    //debugOut(oss2.str());
    
    delete [] buffer;
    fclose(fp);
    return crc;
}



void error(const char *msg)
{
    debugOut(msg);
    perror(msg);
    exit(0);
}
int getFileList(void) {
    //writeFileList();
    //debugOut(fileListFile);
    if (ReadFileList(fileListFile) == 0) {
        
        //debugOut("writeFileList",fileListFile);
        writeFileList(fileListFile);
        displayFiles();
    }
    else return -1;

    return 0;
}
int writeFileList(std::string filename) {
    //strcpy(STMASettingsFile,"./fileList.txt");
    
    //gSTMAPreferenceFile_ptr = fopen(STMASettingsFile,"w");
    //fprintf(gSTMAPreferenceFile_ptr, "%s,%d","STMAClient",75757);
    //fclose(gSTMAPreferenceFile_ptr);
    ofstream myfile;
    myfile.open((char *)filename.c_str());
    if (myfile.is_open()) {
		//debugOut("Written By PC What?");
		//myfile << "WrittenByPC,100 what????\n";
        for (int i=0;i<filelist.size();i++) {
			stringstream oss;
			oss << "," << filelistCrc[i];
			string oss_s = oss.str();

			debugOut(filelist[i],oss_s);
			//debugOut("   crc = ", oss_s);
            myfile << filelist[i] << "," << filelistCrc[i] << "\n";
        }
    }
    else return -1;
    myfile.close();
    return 0;
}
int getOverride(std::string filename) {
   std::string line;
    //strcpy(STMASettingsFile,"./settings.txt");
    ifstream myfile((char *)filename.c_str());
    int index = 0;

    //gSTMAPreferenceFile_ptr = fopen(STMASettingsFile,"r");
    if (myfile.is_open()) {
        while (! myfile.eof()) {
            int temp = 0;
            getline(myfile, line);
            printf("reading doNotAllowUpdates file line=%s\n",(char *)line.c_str());
            
            int found = line.find_first_of(":");
            if (found >0) {
                allowUpdates = 0;
                break;
            }
        }
    }
    else return -1;
    //fclose(gSTMAPreferenceFile_ptr); 
    myfile.close();
    return 0;

}

int getSettings(std::string filename) {
   std::string line;
    //strcpy(STMASettingsFile,"./settings.txt");
    ifstream myfile((char *)filename.c_str());
    int index = 0;

    //gSTMAPreferenceFile_ptr = fopen(STMASettingsFile,"r");
    if (myfile.is_open()) {
        while (! myfile.eof()) {
            int temp = 0;
            getline(myfile, line);
            printf("reading settings file line=%s\n",(char *)line.c_str());
            
            int found = line.find_first_of(":");
            if (found >0) {
                //scanf(gSTMAPreferenceFile_ptr, "%[^,] %d", &PrefText, &crc);
                // store in vector
               std::string name;
               std::string value;
                
                //sTemp.append(PrefText);
                name = line.substr(0,found);
                value = line.substr(found+1,line.size()-1);

                if (index == 0) strcpy(serverLocation,(char *)value.c_str());
                if (index == 1) strcpy(serverUpdate,(char *)value.c_str());
                if (index == 2) strcpy(serverPath,(char *)value.c_str());
                if (index == 3) {if (value.compare("Yes") == 0 && overrideAllowUpdates == 0) allowUpdates = 1;}

                if (name.compare("ignore") == 0) {
                    // build ignore list
                    ignoreList.push_back(value);
                }

                index++;
            }
        }
    }
    else return -1;
    //fclose(gSTMAPreferenceFile_ptr); 
    myfile.close();
    return 0;

}
int ReadFileList(std::string filename )
{
   std::string line;
    int crc = 0;
    ifstream myfile((char *)filename.c_str());

    //gSTMAPreferenceFile_ptr = fopen(STMASettingsFile,"r");
    if (myfile.is_open()) {
        while (! myfile.eof()) {
            int temp = 0;
            getline(myfile, line);
            debugOut("line in fileList = ",line);
            
            if (line.length() > 0) {
                int found = line.find_first_of(",");
                
                if (found >0) {
                    //scanf(gSTMAPreferenceFile_ptr, "%[^,] %d", &PrefText, &crc);
                    // store in vector
                    //debugOut("found > 0\n");
                    std::string sTemp;
                    //sTemp.append(PrefText);
                    sTemp = line.substr(0,found);
                    crc = stoi(line.substr(found+1,line.size()-1));
                    filelist.push_back(sTemp);
                    //debugOut("calling cksum:",sTemp);
                    int actualCrc = cksum(sTemp);
                    //stringstream oss;
                    //oss << "Actual CRC = " << actualCrc;
                    //debugOut(oss.str());
                    filelistCrc.push_back(actualCrc);
                }
                else {
                    if (line.length() > 0) {
                        debugOut("did not find , so adding bogus CRC\n");
                        line.append(",-100");
                        found = line.find_first_of(",");
                        std::string sTemp;
                        //sTemp.append(PrefText);
                        sTemp = line.substr(0,found);
                        crc = stoi(line.substr(found+1,line.size()-1));
                        filelist.push_back(sTemp);
                        //debugOut("calling cksum:",sTemp);
                        int actualCrc = cksum(sTemp);
                        //stringstream oss;
                        //oss << "Actual CRC = " << actualCrc;
                        //debugOut(oss.str());
                        filelistCrc.push_back(actualCrc);

                    }
                }
            }
        }
    }
    else return -1;
    //fclose(gSTMAPreferenceFile_ptr); 
    //debugOut("ReadFileList::myfile.close()");
    myfile.close();
    return 0;
}
int ReadFileListUp(std::string filename )
{
   std::string line;
    int crc = 0;
    ifstream myfile(filename);

    //gSTMAPreferenceFile_ptr = fopen(STMASettingsFile,"r");
    if (myfile.is_open()) {
        while (! myfile.eof()) {
            int temp = 0;
            getline(myfile, line);
            //printf("lineUp=%s\n",(char *)line.c_str());
            
            int found = line.find_first_of(",");
            if (found >0) {
                //scanf(gSTMAPreferenceFile_ptr, "%[^,] %d", &PrefText, &crc);
                // store in vector
               std::string sTemp;
                //sTemp.append(PrefText);
                sTemp = line.substr(0,found);
                crc = stoi(line.substr(found+1,line.size()-1));
                filelistUp.push_back(sTemp);
                //int actualCrc = cksum((char *)sTemp.c_str());

                filelistUpCrc.push_back(crc);
            }
        }
    }
    else return -1;
    //fclose(gSTMAPreferenceFile_ptr); 
    myfile.close();
    return 0;
}

void displayFiles (void) {

    //for (vector<int>::iterator it=filelistCrc.begin() ; it != filelistCrc.end(); ++it) {
    //    printf("%d\n",*it);
    // }
    for (int i=0;i<filelistCrc.size();i++) {
        int actualCrc = cksum(filelist[i]);

        printf("%s %d actualCrc = %d\n",(char *)filelist[i].c_str(),filelistCrc[i],actualCrc);

    }
}

int createOutBuff(std::string & buffer, std::string & content) {

   std::stringstream this_host; 

  std::string host = "POST /cgi-bin/server-update.cgi HTTP/1.1\nHost: www.meteorbike.com\nContent-Type: application/x-www-form-urlencoded\nContent-Length: ";

   this_host << "POST /" << serverUpdate << " HTTP/1.1\nHost: " << serverLocation << "\nContent-Type: application/x-www-form-urlencoded\nContent-Length: ";

   host = this_host.str();

   int contentLength = content.size();

//   strcpy(buffer, "POST /cgi-bin/server-update.cgi HTTP/1.1\nHost: www.meteorbike.com\nContent-Type: application/x-www-form-urlencoded\nContent-Length: 14\n\nname=Ben&sex=m\r\n");

    buffer.append(host);
    std::stringstream sstm;
    sstm << contentLength;
   std::string result = sstm.str();

    buffer.append(result);
    buffer.append("\n\n");
    buffer.append(content);
    buffer.append("\r\n");

    return 0;
}

int getContent(std::string & content) {
    int temp = filelistCrc.size();
    if (temp > 4) temp = 4;
    for (int i=0;i<temp;i++) {
        content.append(filelist[i]);
        content.append("=");
        content.append(to_string(filelistCrc[i]));
        content.append("&");
    }
    
    content.append("REQUEST=FILELIST&");
    content.append("SERVER=");
    content.append(serverLocation);
    content.append("&");
    content.append("LOCATION=");
    content.append(serverPath);
    content.append("&");
    content.append("SERVERUPDATE=");
    content.append(serverUpdate);
    content.append("&");

    printf("content=%s",(char *)content.c_str());

    return 0;
}
// downloadFile takes the local path file of the server only, not the XPlane path - ever.
int downloadFile(std::string fileToDownload,std::string FileToSave) {
    // download a file from meteorbike
   std::string serverLocationStr;
    std::stringstream concatStr;
    concatStr << serverLocation << "/" << serverPath << "/" << fileToDownload;
    serverLocationStr = concatStr.str();
    debugOut("download update file from:");
    debugOut(serverLocationStr);
    //debugOut(FileToSave);
    download(serverLocationStr, FileToSave);

    return 0;

}
int deleteFiles(void) {
    int doNotDownload = 0;
    for (int t=0;t<filelistUp.size();t++) {
        // if the CRC for the update list is 0, that means to remove the file
        if (filelistUpCrc[t] == 0) {
            for (int w=0;w<ignoreList.size();w++) {
                if (filelistUp[t].compare(ignoreList[w]) == 0) {
                    doNotDownload = 1;
                }
                break;
            }
            if (doNotDownload == 0) {
                debugOut("deleting this file ->>>",filelistUp[t]);
                std::string filename = filelistUp[t];
                modPath(filename);
                //debugOut(filename);
                // remove a file to see if we can
                if (remove((char *)filelistUp[t].c_str()) != 0)
                    debugOut("Error deleting file");
                else
                    debugOut("File successfully deleted");
             }
            else doNotDownload = 0; // reset
        }
    }

    return 0;
}
int checkReplacement(std::string & filename) {
    // if this is the name of our exec, download it to a different name, like update.exe
    #if IBM
    const int isWindows = 1;
    #elif LIN 
    const int isWindows = 2;
    #else
    const int isWindows = 0;
    #endif

    if (filename.find("./STMAClient.exe") != std::string::npos) {
        if (1 == isWindows) {
        // seems to be our thing.
        std::string tempName = filename;
        tempName.append("__temp");
        debugOut("Updating Running Windows Executable.");
        rename((char *)filename.c_str(),(char *)tempName.c_str());
        
        filename.append("__temp"); // the download will be to __temp until we rename the download and then delete the old
        
        return 1; // needs replacment
        }
        else return 2; // Do Not Replace!
    }
    else if (filename.find("./STMAClientLin") != std::string::npos) {
        if (2 == isWindows) {
        // seems to be our thing.
        std::string tempName = filename;
        tempName.append("__temp");
        debugOut("Updating Running Executable.");
        rename((char *)filename.c_str(),(char *)tempName.c_str());
        
        filename.append("__temp"); // the download will be to __temp until we rename the download and then delete the old
        
        return 1; // needs replacment
        }
        else return 2;
    }
    else if (filename.find("./STMAClient") != std::string::npos) {
        if (0 == isWindows) {
        // seems to be our thing.
        std::string tempName = filename;
        tempName.append("__temp");
        debugOut("Updating Running Executable.");
        rename((char *)filename.c_str(),(char *)tempName.c_str());
        
        filename.append("__temp"); // the download will be to __temp until we rename the download and then delete the old
        
        return 1; // needs replacment
        }
        else return 2;
    }
    return 0;

}
int downloadFileList(void) {

    int doNotDownload = 0;
    for (int t=0;t<fileListDownload.size();t++) {
        for (int w=0;w<ignoreList.size();w++) {
            if (fileListDownload[t].compare(ignoreList[w]) == 0) {
                doNotDownload = 1;
            }
            break;
        }
        if (doNotDownload == 0) {
            std::string filename = fileListDownload[t];
            std::string newname = filename;
            // special case where the downloaded file is this executable
            modPath(filename);
            modPath(newname);

            int replaceThis = checkReplacement(filename);

            if (replaceThis < 2) { // do not download if replace comes back with a 2 - that's a file we don't touch in this OS -- BECAUSE downloading it will change permissions to not executable.  
                //  Really, that's not a huge deal because most people only run one OS 
                //-but I run both and in the same folder, so that would be a problem.
                debugOut("downloading this file ->>>",fileListDownload[t]);
                downloadFile(fileListDownload[t],filename);
            }

            if (1 == replaceThis) {
                rename((char *)filename.c_str(),(char *)newname.c_str());  // rename __temp to normal filename
                //remove((char *)filename.c_str()); // delete the __tmp
            }
            
            //debugOut("newname = ",newname);
        }
        else doNotDownload = 0; // reset
    }

    return 0;
}
int updatesAvailable (void ){
    #if IBM
    const int isWindows = 1;
    #else 
    const int isWindows = 0;
    #endif
   // a complicated algorithm to determine if updates in the download list are actual viable updates
    // ignoring displaying the STMAClient...???
    int viable = 0;
    for (int i=0;i<fileListDownload.size();i++) {
        if (fileListDownload[i].find("fileList.txt") == std::string::npos &&
            fileListDownload[i].find("STMAClient") == std::string::npos) {
            viable = 1;
            break;
        }
    }
    return viable;

}

// this list goes out as the final change list.  It doesn't affect the actual downloads, changes, etc.  
// essentially just a report for the XPlane plugin to pick up.
int createChangeList (void) {
    int doNotDownload = 0;
    ofstream myfile;
    std::string fileName = fileFinalChangeList;
    modPath(fileName);
    myfile.open(fileName);
    if (myfile.is_open()) {
        myfile << "STMA AutoUpdate v1.1\n";
        if (0 == updatesAvailable()) {
            myfile << "   Files are up to date.\n";

        }
        else {
            myfile << "   Updates Available.\n";
        
            for (int i=0;i<fileListDownload.size();i++) {

                for (int w=0;w<ignoreList.size();w++) {
                    if (fileListDownload[i].compare(ignoreList[w]) == 0) {
                        doNotDownload = 1;
                    }
                    break;
                }
                if (doNotDownload == 0 && allowUpdates == 1) {
                    myfile << fileListDownload[i] << "\n";
                }
                else {
                    doNotDownload = 0; // reset
                    myfile << fileListDownload[i] << " needs update.\n";
                }
            }
        }
    }
    myfile.close();


    return 0;

}
int buildDiffList(void) {
    // we now have two files, ./fileList.txt and ./.fileListUp.txt
    // compare the two
    // if the filename is the same, but the crc is different, add it to the file download list
    // if the filename is new, add it
    
    // generate the two vectors for filenames and updated crcs
    ReadFileListUp(fileListUpFile);  

    for (int i=0;i<filelistUp.size();i++) {
        // grab first file of update into a temp string
        std::string fn1 = filelistUp[i];
        int fn1Crc = filelistUpCrc[i];

        int foundFile = 0;
        // find the same filename in filelist, if it doesn't exist, save it in our download list.
        for (int j=0;j<filelist.size();j++) {
            std::string fn2 = filelist[j];
            int fn2Crc = filelistCrc[j];

            if (fn1.compare(fn2) == 0) {
                // strings are same, now compare crc
                if (fn1Crc != fn2Crc) {
                    // differing CRCs, save this file to download later.
					stringstream oss;
					oss << fn1 << "," << fn1Crc << " != " << fn2 << "," << fn2Crc;
					string oss_s = oss.str();

					debugOut(oss_s);
                    if (filelistUpCrc[i] != 0) {
                        fileListDownload.push_back(fn1);
                    }
                }
                foundFile = 1;
                break;
            }
        }
        if (foundFile == 0) {
            debugOut("File Not Found, pushing: ", fn1);
            fileListDownload.push_back(fn1);
        }
    }

    //for (int t=0;t<fileListDownload.size();t++) {
		//debugOut("download this file here -> ", fileListDownload[t]);
        //printf("download this file -> %s\n",(char *)fileListDownload[t].c_str());
    //}

    return 0;
}

int getReturnData(std::string input) {
    // this is the data that was returned from the update server.
    // it should containsome parameters we can use to update our vars and overwrite our settings file.


    return 0;


}

int initFileNames(int first) {

//    char cCurrentPath[FILENAME_MAX];
//    stringstream myPath;
//    std::string myPath_s;
//
//    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
//        return errno;
//    }
//    myPath << cCurrentPath;
//    myPath_s = myPath.str();
//    std::size_t found;
//
//    found = myPath_s.find("Resources");
//    if (found != std::string::npos || XPlane == 0) {
//        XPlane = 0;
//        //debugOut("XPlane = 0\n");
//    }
//    else {
//        XPlane = 1;
//        //debugOut("XPlane = 1\n");
//    }



    // determine where this was started, from X-Plane, or locally.
    if (1 == XPlane && 0 == first) {
        doNotAllowUpdates=arguments;
        doNotAllowUpdates.append("doNotAllowUpdates.txt");
        settingsFile=arguments;
        settingsFile.append("settings.txt");
        fileListFile=arguments;
        fileListFile.append("fileList.txt");
        fileListUpFile=arguments;
        fileListUpFile.append("fileListUp.txt");
    }
    else {
        doNotAllowUpdates ="./doNotAllowUpdates.txt";
        settingsFile="./settings.txt";
        fileListFile="./fileList.txt";
        fileListUpFile="./fileListUp.txt";
    }

    return 0;
}
int initChangeFile(int first) {
    ofstream myfile;
	std::string fileName;
	if (1 == XPlane && 0 == first) {
        fileName = arguments;
        fileName.append("fileChangeList.txt");
	}
	else {
		fileName = "./fileChangeList.txt";
	}
    fileChangeList = fileName;
    //modPath(fileName);
    if (remove((char *)fileName.c_str()) != 0) { ; }
    if (first) {
//#ifdef DEBUG_ONLY
		myfile.open(fileName);
        if (myfile.is_open()) {
                    myfile << "Clean.\n";
        }
        myfile.close();
//#endif
	}


    if (first) {
    if (1 == XPlane) {
        fileFinalChangeList = arguments;
        fileFinalChangeList.append("./updateStatus.txt");
    }
    else {
        fileFinalChangeList.append("./updateStatus.txt");        
    }
    }

    return 0;
}
//int main (int argc, char *argv[]) {
//    initChangeFile();
//    exit(0);/
//}
int socketStuff(void) {
#ifndef IBM
   int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    std::string s_buff;
    std::string s_content;
    
    char buffer[512];
    char * host;
    
    portno = 80;
    //portno = 80; // http
    sockfd = socket(AF_INET, SOCK_STREAM, 6);
    debugOut("sockfd\n");
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(0);
    }
    server = gethostbyname(host);
    debugOut("got server\n");

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    debugOut("serv_addr\n");

    serv_addr.sin_family = AF_INET;
    debugOut("bcopy\n");
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    debugOut("trying to connect\n");

    int temp = 0;
    temp = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    //debugOut("temp = %d\n",temp);
    if (temp < 0) {
        error("ERROR connecting");
        exit(0);
    }

    bzero(buffer,512);
    //fgets(buffer,255,stdin);
    //strcpy(buffer, "GET /dance-2005/ HTTP/1.1 - Mozilla/5.0 (compatible; Baiduspider/2.0; +http://www.baidu.com/arch///spider.html)\r\n");
    getContent(s_content);
    
    createOutBuff(s_buff, s_content);
    strcpy (buffer,(char *)s_buff.c_str());

    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) {
        error("ERROR writing to socket");
        exit(0);
    }

    bzero(buffer,512);
    n = read(sockfd,buffer,512);
    if (n < 0) {
        error("ERROR reading from socket");
        return 0;
    }
    close(sockfd);

    debugOut("Read Back:\n");
    debugOut(buffer);
    debugOut("End Read Back.");

    std::stringstream mybuf;
    mybuf << buffer;
    std::string mybufStr = mybuf.str();
    getReturnData(mybufStr);
#endif
    return 0;
}

int cleanup(void) {

	if (remove((char *)fileListUpFile.c_str()) != 0) { ; }
            //debugOut("Error deleting file");
        //else
            //debugOut("File successfully deleted");
    return 0;
}

int main(int argc, char *argv[])
{
    // pre-init to dump the path on the cmd line
    initFileNames(1);
    initChangeFile(1);
    // then re-set back to 1 (default value) and re-init
    XPlane = 1;

    if (argc > 1) {
        arguments = argv[1];
        // problem in windows where if X-Plane 10 has a space in there it treats it as multiple args.
        //if (argc >2) {arguments.append(" "); arguments = arguments.append(argv[2]);}
        for (int i=2;i<argc;i++) {
            arguments.append(" ");
            arguments.append(argv[i]);
        }
        debugOut("invocation called:",argv[0]);
        debugOut("path passed on command line:",arguments);
        XPlane = 1;

        if (arguments.find("-genfile")!=std::string::npos) { XPlane = 0; overrideAllowUpdates = 1; arguments.clear();}
    }
    else XPlane = 0;

    init();
    initFileNames(0);

    if (argc > 1) {
       debugOut("path passed on command line...");
       debugOut(arguments);
    }
    
    if (0 != getSettings(settingsFile)) {
        debugOut("could not open settings file:",settingsFile);
    }
    getOverride(doNotAllowUpdates);
    initChangeFile(0);
    debugOut("Started on PC ver3 this time.");

    if (getFileList() < 0) {
        debugOut("could not get filelist");

        error("Could not get filelist");
        exit(0);
    }


    downloadFile("fileList.txt",fileListUpFile);

    buildDiffList();

    if (allowUpdates) {
        deleteFiles();
        downloadFileList();
    }
    
    createChangeList();

    cleanup();

    stop();

    return 0;
}







































