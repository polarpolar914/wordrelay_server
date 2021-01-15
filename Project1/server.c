#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <winsock2.h>
#include <Windows.h>
#include <wininet.h>

#define BUFSIZE 1024
#pragma comment (lib, "ws2_32")
#pragma comment (lib, "wininet.lib")
#pragma comment (lib, "./ew32.lib")

DWORD WINAPI makeThread(void* data);
DWORD WINAPI makeSevThread(void* data);

// gcc server.c -lws2_32 -lwininet

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

int wordcheck(char* word) {
    char url[49] = "/api/v2/entries/en/";
    char* buffer = NULL;
    strcat(url, word);
    LoadDataFromWebPage("api.dictionaryapi.dev", url, &buffer);
    if (buffer) {
        if (strchr(buffer, 'D')) {//단어가 유효하지 않을시 0반환 
            free(buffer);
            return 0;
        }
        else {//단어 유효시 1반환  
            free(buffer);
            return 1;
        }
    }
    else {//API오류시 -1반환 
        free(buffer);
        return -1;
    }
    return -2;
}

char* wordrandom() {
    char* buffer = NULL; int i, j; char* word = NULL;
    LoadDataFromWebPage("random-word-api.herokuapp.com", "/word?number=1", &buffer);
    int length = strlen(buffer);
    for (i = 0; i < (length - 2); i++) {
        buffer[i] = buffer[i + 2];
    }
    buffer[length - 4] = '\0';
    word = buffer;
    free(buffer);
    return word;
}

typedef struct Player {
    SOCKET conn;
    char name[1024];
    int life;
} player;

typedef struct Node {
    player* p;
    struct Node* next;
} node;

node* head = NULL;
node* tail = NULL;

node* insertNode(player* p) {
    if (head == NULL) {
        head = (node*)malloc(sizeof(node));
        head->p = p;
        head->next = NULL;
        tail = head;
    } else {
        node* n = (node*)malloc(sizeof(node));
        tail->next = n;
        n->p = p;
        n->next = NULL;
        tail = n;
    }
}

void deleteNode(node* n) {
    node* index = head;
    if (index->next == NULL) {
        index = NULL;
        free(n);
        return;
    }
    while (index->next != n) {
        index = index->next;
    }
    index->next = n->next;
    free(n);
}

char wordList[100000][1024] = { 0 };
int wordCount = 0;

int isOverlap(char* c) {
    for (int i = 0; i < wordCount; i++) {
        //printf("%s %s\n", wordList[i], c);
        if (!strcmp(wordList[i],c)) {
            return 1;
        }
    }
    return 0;
}

node* currentNode;
player* selectPlayer;
int playerCount = 0;
int isPlaying = 0;

int main()
{
    WSADATA wsaData;
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

    HANDLE hThread = CreateThread(NULL, 0, makeSevThread, NULL, 0, NULL);
    CloseHandle(hThread);

    while (1) {
        player *p = (player*)malloc(sizeof(player));
        p->conn = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (isPlaying) {
            char message[] = "already playing...";
            send(p->conn, message, strlen(message), NULL);
            closesocket(p->conn);
        }
        else {
            hThread = CreateThread(NULL, 0, makeThread, (void*)insertNode(p), 0, NULL);
            CloseHandle(hThread);
            playerCount++;
            printf("%d\n", playerCount);
        }
    }

    closesocket(listenSocket);

    WSACleanup();
}

DWORD WINAPI makeThread(void* data) {
    node* n = (node*)data;
    player* p = n->p;
    SOCKET socket = p->conn;

    printf("Connect Someone\n");
    //printf("%d\n", &socket);

    strcpy(p->name, "emtpy");

    char messageBuffer[BUFSIZE];
    int receiveBytes;
    int first = 1;
    while (receiveBytes = recv(socket, messageBuffer, BUFSIZE, 0)) {
        if (receiveBytes > 0) {
            if (first) {
                first = 0;
                strcpy(p->name,messageBuffer);
                char message[] = "";
                strcat(message,"your name is ");
                strcat(message, p->name);
                send(socket, message, strlen(message), 0);
                printf("Someone change name : %s\n", (*p).name);
            } else if (isPlaying) {
                char message[1024] = "";

                printf("TRACE - Receive message : %s (%d bytes)\n", messageBuffer, receiveBytes);

                if (selectPlayer == p) {
                    char* currentWord = wordList[wordCount - 1];
                    printf("%d\n", wordCount);

                    printf("%s %d %c\n", currentWord, strlen(currentWord), currentWord[strlen(currentWord) - 1]);
                    if (currentWord[strlen(currentWord) - 1] != messageBuffer[0]) {
                        strcpy(message,"Sorry, but you should input a word starting with the last letter of the last word.");
                        p->life--;
                    } else if (wordcheck(messageBuffer)<1) {
                        strcpy(message,"Sorry, but there is no such word in the api.");
                        p->life--;
                    } else if (isOverlap(messageBuffer)) {
                        strcpy(message, "Sorry, but this word is already using.");
                        p->life--;
                    } else {
                        strcpy(message,"Great!");
                        strcpy(wordList[wordCount++],messageBuffer);
                        currentNode = currentNode->next;
                        if (currentNode == NULL) {
                            currentNode = head;
                        }
                        selectPlayer = currentNode->p;
                    }
                } else {
                    //strcpy(message,"Sorry, it`s not your turn.");
                }
                node* index = head;
                while (index != NULL) {
                    player* other = index->p;
                    if ((*other).conn && other != p) {
                        char message[1024] = "";
                        strcat(message, p->name);
                        strcat(message, ": ");
                        strcat(message, messageBuffer);
                        int sendBytes = send(other->conn, message, strlen(message), 0);

                        if (sendBytes > 0) {
                            printf("TRACE - Send message : %s (%d bytes)\n", message, sendBytes);
                        }
                    }
                    index = index->next;
                }

                if (p->life <= 0) {
                    isPlaying = 0;
                    node* index = head;
                    while (index != NULL) {
                        player* other = index->p;
                        if ((*other).conn) {
                            char message[1024] = "";
                            strcat(message, p->name);
                            strcat(message, " is dead.. game over");
                            int sendBytes = send(other->conn, message, strlen(message), 0);

                            if (sendBytes > 0) {
                                printf("TRACE - Send message : %s (%d bytes)\n", message, sendBytes);
                            }
                        }
                        index = index->next;
                    }
                }

                if (selectPlayer != p) {
                    send(selectPlayer->conn, "your turn.", strlen("your turn."), 0);
                }
                printf("%s\n", message);
                send(p->conn, message, strlen(message), 0);
            } else {
                printf("TRACE - Receive message : %s (%d bytes)\n", messageBuffer, receiveBytes);

                node* index = head;
                while (index != NULL) {
                    player* other = index->p;
                    if ((*other).conn && other != p) {
                        char message[1024] = "";
                        strcat(message, p->name);
                        strcat(message, ": ");
                        strcat(message, messageBuffer);
                        int sendBytes = send(other->conn, message, strlen(message), 0);

                        if (sendBytes > 0) {
                            printf("TRACE - Send message : %s (%d bytes)\n", message, sendBytes);
                        }
                    }
                    index = index->next;
                }
            }
        }
        else {
            break;
        }
    }
    printf("%s LEAVE\n", (*p).name);
    closesocket(socket);
    deleteNode(n);
    free(p);
    playerCount--;
    printf("%d\n", playerCount);
    return 0;
}

DWORD WINAPI makeSevThread(void* data) {
    srand((unsigned int)time(NULL));
    char command = 0;
    while (scanf("%c",&command) != EOF) {
        switch (command) {
        case 's':
            if (isPlaying) continue;
            if (playerCount < 2) continue;
            int playerIndex = rand() % playerCount;

            strcpy(wordList[0],wordrandom());
            wordCount = 1;

            printf("%s\n", wordList[wordCount - 1]);

            currentNode = head;
            for (int i = 0; i < playerIndex; i++) {
                currentNode = currentNode->next;
                if (currentNode == NULL) {
                    currentNode = head;
                }
            }
            selectPlayer = currentNode->p;

            node* index = head;
            while (index != NULL) {
                player* other = index->p;

                if (other->conn) {
                    char message[1024] = "";
                    strcat(message, "Game is Started. Prompt is '");
                    strcat(message, wordList[wordCount - 1]);
                    strcat(message, "'. ");
                    strcat(message, selectPlayer->name);
                    strcat(message, ", start word relay.");

                    send(other->conn, message, strlen(message), 0);
                    
                    other->life = 5;
                }

                index = index->next;
            }
            isPlaying = 1;

            break;
        case 'e':
            isPlaying = 0;
            node* i = head;
            while (i != NULL) {
                player* other = i->p;
                if ((*other).conn) {
                    char message[1024] = "Game End of server";
                    int sendBytes = send(other->conn, message, strlen(message), 0);

                    if (sendBytes > 0) {
                        printf("TRACE - Send message : %s (%d bytes)\n", message, sendBytes);
                    }
                }
                i = i->next;
            }
            break;
        }
    }
    return 0;
}
