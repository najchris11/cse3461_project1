// Christian Coulibaly
// Prof. Dave Ogle
// CSE 3461 SP 2025

#include <cjson/cJSON.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define BUFFER_LEN 1024
 
void cleanup(int socketDesc, cJSON *jsonMessage, char *json_str) {
    if (socketDesc > 0) close(socketDesc);
    if (jsonMessage) cJSON_Delete(jsonMessage);
    if (json_str) cJSON_free(json_str);
}

int main(int argc, char *argv[]) {
    int portNum, socketDesc;
    struct sockaddr_in sin_addr;
    char message[BUFFER_LEN], buffer[BUFFER_LEN];

    if (argc < 3) {
        printf("Usage is: client <portNumber> <ipaddress> \n");
        return 1;
    }

    portNum = atoi(argv[1]);
    char *ipAddress = argv[2];

//socket setup
    if ((socketDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error opening stream socket");
        return 1;
    }

    sin_addr.sin_family = AF_INET;
    sin_addr.sin_port = htons(portNum);

    if (inet_pton(AF_INET, ipAddress, &(sin_addr.sin_addr)) <= 0) {
        printf("failed to convert IP address\n");
        cleanup(socketDesc, NULL, NULL);
        return 1;
    }

    if (connect(socketDesc, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("error connecting stream socket");
        cleanup(socketDesc, NULL, NULL);
        return 1;
    }

    printf("Please type in your desired message (less than 1024 chars): ");
    if (fgets(message, BUFFER_LEN, stdin) == NULL) {
        printf("error reading input\n");
        cleanup(socketDesc, NULL, NULL);
        return 1;
    }
    if (message[strlen(message) - 1] != '\n') {
    printf("warning: input exceeds 1023 characters and will be truncated!\n");

    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}
    message[strcspn(message, "\n")] = '\0';

    //prepping message for sending

    cJSON *jsonMessage = cJSON_CreateObject();
    cJSON_AddStringToObject(jsonMessage, "message", message);
    char *json_str = cJSON_Print(jsonMessage);
    int messageSize = strlen(json_str);
    int convertedLength = htonl(messageSize);

    if (write(socketDesc, &convertedLength, sizeof(int)) < sizeof(int)) {
        printf("error sending message size\n");
        cleanup(socketDesc, jsonMessage, json_str);
        return 1;
    }

    if (write(socketDesc, json_str, messageSize) < messageSize) {
        printf("error sending message\n");
        cleanup(socketDesc, jsonMessage, json_str);
        return 1;
    }
    
    cJSON_free(json_str);
    json_str = NULL;


// prepping for response
    int receivedLength;
    if (read(socketDesc, &receivedLength, sizeof(int)) < sizeof(int)) {
        printf("error receiving response size\n");
        cleanup(socketDesc, jsonMessage, NULL);
        return 1;
    }

    receivedLength = ntohl(receivedLength);
    if (receivedLength >= BUFFER_LEN) {
        printf("error: response size too large\n");
        cleanup(socketDesc, jsonMessage, NULL);
        return 1;
    }

    if (read(socketDesc, buffer, receivedLength) < receivedLength) {
        printf("error receiving response\n");
        cleanup(socketDesc, jsonMessage, NULL);
        return 1;
    }

    buffer[receivedLength] = '\0';

    cJSON *jsonResponse = cJSON_Parse(buffer);
    if (jsonResponse == NULL) {
        printf("error parsing JSON response\n");
        cleanup(socketDesc, jsonMessage, NULL);
        return 1;
    }

    cJSON *response = cJSON_GetObjectItemCaseSensitive(jsonResponse, "response");
    if (cJSON_IsString(response) && response->valuestring != NULL) {
        printf("Response: %s\n", response->valuestring);
    } else {
        printf("error deserializing json");
    }

    cJSON_Delete(jsonResponse);
    cleanup(socketDesc, jsonMessage, NULL);
    return 0;
}