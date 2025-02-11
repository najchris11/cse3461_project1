// Christian Coulibaly
// Prof. Dave Ogle
// CSE 3461 SP 2025

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cjson/cJSON.h>

#define BUFFER_LEN 1024

int main(int argc, char *argv[]) {
    int portNum;
    int socketDesc;
    struct sockaddr_in sin_addr;
    char message[BUFFER_LEN];
    char buffer[BUFFER_LEN];

  if (argc < 3) {
    printf("The format to run this program is client <portNumber> <ipaddress> \n");
    exit(1);
  }

    portNum = atoi(argv[1]);
    char *ipAddress = argv[2];
    if((socketDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error opening stream socket");
        exit(1);
    }

    sin_addr.sin_family = AF_INET;
    sin_addr.sin_port = htons(portNum);

    if (inet_pton(AF_INET, ipAddress, &(sin_addr.sin_addr)) < 0) {
        printf("failed to convert ip address\n");
        close(socketDesc);
        exit(1);
    }

    if(connect(socketDesc, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
        close(socketDesc);
        perror("error connecting stream socket");
        exit(1);
    }
    printf("Please type in your desired message (less than 1024 chars): ");
    scanf("%[^\n]s", message);

    cJSON *jsonMessage = cJSON_CreateObject();
    cJSON_AddStringToObject(jsonMessage, "message", message);
    char *json_str = cJSON_Print(jsonMessage);
    int messageSize = strlen(json_str);
    int convertedLength = htonl(messageSize);

    if (write(socketDesc, &convertedLength, sizeof(int)) < sizeof(int)) {
        printf("error sending message size\n");
        cJSON_free(json_str);
        cJSON_Delete(jsonMessage);
        close(socketDesc);
        exit(1);
    }
    if (write(socketDesc, json_str, messageSize) < messageSize) {
        printf("error sending message\n");
        cJSON_free(json_str);
        cJSON_Delete(jsonMessage);
        close(socketDesc);
        exit(1);
    }
    cJSON_free(json_str);


// listen
    int receivedLength;
    if (read(socketDesc, &receivedLength, sizeof(int)) < sizeof(int)) {
        printf("error receiving message size\n");
        cJSON_Delete(jsonMessage);
        close(socketDesc);
        exit(1);
    }
    receivedLength = ntohl(receivedLength);

    if (receivedLength >= BUFFER_LEN) {
        printf("error with buffer overflow for recd message\n");
        cJSON_Delete(jsonMessage);
        close(socketDesc);
        exit(1);
    }
    if(read(socketDesc, buffer, receivedLength) < receivedLength) {
        printf("error receiving message\n");
        cJSON_Delete(jsonMessage);
        close(socketDesc);
        exit(1);
    }
    jsonMessage = cJSON_Parse(buffer);
    if (jsonMessage == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error: %s\n", error_ptr);
        }
        cJSON_Delete(jsonMessage);
        close(socketDesc);
        exit(1);
    }

    cJSON *response = cJSON_GetObjectItemCaseSensitive(jsonMessage, "response");
    if(cJSON_IsString(response) && (response->valuestring != NULL)) {
        printf("Response: %s\n", response->valuestring);
    } else {
        printf("error deserializing json");
        close(socketDesc);
    cJSON_Delete(jsonMessage);
    exit(1);
    }

    close(socketDesc);
    cJSON_Delete(jsonMessage);
    return 0;
}