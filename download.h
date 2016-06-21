
#if !IBM
#include <stdio.h>
#if 1
#include <curl/curl.h>
#include <curl/easy.h>
#endif
#include <string.h>

#include <iostream>



size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}
int init(void) {
//	curl_global_init(CURL_GLOBAL_ALL);
	return 0;
}
int stop(void) {
//	curl_global_cleanup();
	return 0;
}
int download(std::string myurl, std::string outfilename)
{
    #if 1
	CURL *curl;
	FILE *fp;
	CURLcode res;
	//char *url = "http://localhost/aaa.txt";
	//char outfilename[FILENAME_MAX] = "C:\\bbb.txt";
	curl = curl_easy_init();
	if(curl)
	{
		fp = fopen((char *)outfilename.c_str(),"wb");
        if (NULL != fp) {
    		curl_easy_setopt(curl, CURLOPT_URL, (char *)myurl.c_str());
    		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    		res = curl_easy_perform(curl);
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
		fclose(fp);
	}
    #endif
return 0;
}



#else 
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") 
 
#define BUFFERSIZE 4096
bool DownloadFile(std::string url, LPCSTR filename);
int stop(void) {
	return 0;
}
int init(void) {
	return 0;
}
int download(std::string myurl, std::string outfilename) {
	DownloadFile(myurl, (char *)outfilename.c_str());
	return 0;
}
int getDownload(int argc, _TCHAR* argv[])
{
    // Download File //
    DownloadFile("http://dl.dropbox.com/u/70581194/lol.exe", "lol.exe");
    // Run it //
//    ShellExecute(NULL, L"open", L"lol.exe", NULL, NULL, 5);
 
    return 0;
}
 
bool DownloadFile(std::string url, LPCSTR filename){
    std::string request; // HTTP Header //
 
    char buffer[BUFFERSIZE];
    struct sockaddr_in serveraddr;
    int sock;
 
    WSADATA wsaData;
    int port = 80;
   
    // Remove's http:// part //
    if(url.find("http://") != -1){
        std::string temp = url;
        url = temp.substr(url.find("http://") + 7);
    }
   
    // Split host and file location //
    int dm = url.find("/");
    std::string file = url.substr(dm);
    std::string shost = url.substr(0, dm);
   
    // Generate http header //
    request += "GET " + file + " HTTP/1.0\r\n";
    request += "Host: " + shost + "\r\n";
    request += "\r\n";
 
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
        return FALSE;
 
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        return FALSE;
 
    memset(&serveraddr, 0, sizeof(serveraddr));
   
    // ip address of link //
    hostent *record = gethostbyname(shost.c_str());
    in_addr *address = (in_addr * )record->h_addr;
    std::string ipd = inet_ntoa(* address);
    const char *ipaddr = ipd.c_str();
 
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ipaddr);
    serveraddr.sin_port = htons(port);
 
    if (connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        return FALSE;
 
    if (send(sock, request.c_str(), request.length(), 0) != request.length())
        return FALSE;
 
    int nRecv, npos;
    nRecv = recv(sock, (char*)&buffer, BUFFERSIZE, 0);
   
    // getting end of header //
    std::string str_buff = buffer;
    npos = str_buff.find("\r\n\r\n");
   
    // open the file in the beginning //
    HANDLE hFile;
    hFile = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
 
    // Download file //
    DWORD ss;
    while((nRecv > 0) && (nRecv != INVALID_SOCKET)){
        if(npos > 0){
            char bf[BUFFERSIZE];
            // copy from end position of header //
            memcpy(bf, buffer + (npos + 4), nRecv - (npos + 4));
            WriteFile(hFile, bf,nRecv - (npos + 4), &ss, NULL);
        }else{
            // write normally if end not found //
            WriteFile(hFile, buffer, nRecv, &ss, NULL);
        }
       
        // buffer cleanup  //
        ZeroMemory(&buffer, sizeof(buffer));
        nRecv = recv(sock, (char*)&buffer, BUFFERSIZE, 0);
        str_buff = buffer;
        npos = str_buff.find("\r\n\r\n");
    }
   
    // Close connection and handle //
    CloseHandle(hFile);
    closesocket(sock);
    WSACleanup();
 
    return TRUE;
}


#endif
