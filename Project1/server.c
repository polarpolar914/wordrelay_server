#include "stdio.h"
#include <stdlib.h>
#include <winsock2.h>
//#include <Windows.h>
//#include <wininet.h>  

#define BUFSIZE 1024
#pragma comment (lib, "ws2_32")
//#pragma comment (lib, "wininet.lib")
//#pragma comment (lib, "./ew32.lib")

DWORD WINAPI makeThread(void* data);
/*

int Utf8ToAscii(char** dest, char* src)
{
  
    //////////////////////////////////////////////////////////////////////////////
 
    WCHAR* wc_buffer;
    int len;

    int wide_char_len = MultiByteToWideChar(0xFDE9u, 0, src, -1, 0, 0);
    wc_buffer = (WCHAR*)malloc(2 * wide_char_len | -((unsigned long long)wide_char_len >> 31 != 0));
    MultiByteToWideChar(0xFDE9u, 0, src, -1, wc_buffer, wide_char_len);
    len = WideCharToMultiByte(0, 0, wc_buffer, -1, 0, 0, 0, 0);
    *dest = (CHAR*)malloc(len);
    WideCharToMultiByte(0, 0, wc_buffer, -1, *dest, len, 0, 0);
    free(wc_buffer);
    return len;
}

DWORD ReadHtmlText(HINTERNET ah_http_file, char* ap_html_string)
{

    HANDLE h_wait_event = CreateEvent(NULL, TRUE, 0, NULL);
    if (h_wait_event == NULL) return 0;

    char buffer[1025];
    DWORD read_byte = 0, error_count = 0, total_bytes = 0;

    while (InternetReadFile(ah_http_file, buffer, 1024, &read_byte)) {

        memcpy(ap_html_string + total_bytes, buffer, read_byte);
        total_bytes += read_byte;
        if (read_byte < 1024) {


            error_count++;
            if (error_count > 10) break;
            else WaitForSingleObject(h_wait_event, 50);
        }
        else error_count = 0;
    }

    *(ap_html_string + total_bytes) = 0;
    CloseHandle(h_wait_event);
    return total_bytes;
}


void LoadDataFromWebPage(const char* domain, const char* path, char** rslt)
{
    HINTERNET h_session = InternetOpenA("BlogScanner", PRE_CONFIG_INTERNET_ACCESS, NULL, INTERNET_INVALID_PORT_NUMBER, 0);
    HINTERNET h_connect = InternetConnectA(h_session, domain, INTERNET_INVALID_PORT_NUMBER, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    HINTERNET h_http_file = HttpOpenRequestA(h_connect, "GET", path, "HTTP/1.0", NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);

    char* p_utf8_html_str = (char*)malloc(2 * 1024 * 1024);
    if (p_utf8_html_str != NULL) {
        if (HttpSendRequest(h_http_file, NULL, 0, 0, 0) == TRUE) {

            ReadHtmlText(h_http_file, p_utf8_html_str);

            Utf8ToAscii(rslt, p_utf8_html_str);
        }
        free(p_utf8_html_str);
    }


    if (h_http_file != NULL) InternetCloseHandle(h_http_file);
    if (h_connect != NULL) InternetCloseHandle(h_connect);
    if (h_session != NULL) InternetCloseHandle(h_session);
}
*/

struct Player {
    SOCKET conn;
    char name[20];
};

struct Player playerList[100];
int playerCount = 0;

int main()
{
    
    system("chcp 65001"); // 코드페이지를 UTF-8로 변경.
    system("cls");

    WSADATA wsaData; // 야 이거 오류 난 부분 설명 좀 해주셈 그러니까 그 오류난 위치 좀
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error - Cannot load 'winsock.dll' file\n");
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("Error - Invalid socket\n");
        return 1;
    }

    SOCKADDR_IN serverAddr;

    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        printf("Error - Fail bind\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, 5) == SOCKET_ERROR) {
        printf("Error - Fail listen \n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);
    memset(&clientAddr, 0, addrLen);
    SOCKET clientSocket;

    HANDLE hThread;

    while (1) {
        clientSocket = playerList[playerCount++].conn = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
        hThread = CreateThread(NULL, 0, makeThread, (void*)clientSocket, 0, NULL);
        CloseHandle(hThread);
    }

    closesocket(listenSocket);

    WSACleanup();
//////////////////////////////////////////////////////
/*
    char* buffer = NULL;
    char word[20]={0};
   printf("단어를 입력하세요 : ");
    gets(word);
*/

/* client.c 만들어서 확인해야 함 일단 서버 컴파일은 gcc server.c -lws2_32
 이제 클라이언트 만들어서 접속이 되는지를 확인해야 함
 여기서 대충 만들어서 github 링크에다가 올리면 내가 그거 받아가지고 돌려볼게
 넌 이거 api 컴파일 해결하면 될듯
 1. client.c 먼저 만들어주셈 만듬
 2. 그리고 api 컴파일 해결해주셈 api부분 다지운거 아님?
주석 처리만 함
그런 TMI 는 생략해주시고 일단 client.c 따로 만들어서 보내주셈 뭘 보내야됨?
 만들기만 하면
여기서 만들면 더 좋고 근데 컴파일 괜찮음?
 3. 개발 ㄱ 딴프로젝트 파일에서 하고있음
  혹시 네이버나 다음 사전 긁어오는게 빠를려나 해서 뫘는데 나오는 xml이 너무 길어서 처리하기 힘들듯
    LoadDataFromWebPage("stdict.korean.go.kr", "/api/search.do?certkey_no=2201&key=77CF6E24D5171B35E3614C01BE677683&type_search=search&q=단어", &buffer);
    if (buffer) {
        puts(buffer);
        free(buffer);
    }
    else {
        puts("Error");
    }*/
}

DWORD WINAPI makeThread(void* data) {
    SOCKET socket = (SOCKET)data;

    char messageBuffer[BUFSIZE];
    int receiveBytes;
    while (receiveBytes = recv(socket, messageBuffer, BUFSIZE, 0)) {
        if (receiveBytes > 0) {
            printf("TRACE - Receive message : %s (%d bytes)\n", messageBuffer, receiveBytes);

            for (int i=0;i<playerCount;i++){
                if (!playerList[i].conn) continue;
                if (playerList[i].conn == socket) continue;
                int sendBytes = send(playerList[i].conn, messageBuffer, strlen(messageBuffer), 0);
            if (sendBytes > 0) {
                printf("TRACE - Send message : %s (%d bytes)\n", messageBuffer, sendBytes);
            }
            }
        }
        else {
            break;
        }
    }
    closesocket(socket);
    return 0;
}