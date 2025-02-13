// Christian Coulibaly
// Prof. Dave Ogle
// CSE 3461 SP 2025

#include <cjson/cJSON.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
void cleanup(int socketDesc, cJSON *jsonMessage, char *json_str) {
    if (socketDesc > 0) close(socketDesc);
    if (jsonMessage) cJSON_Delete(jsonMessage);
    if (json_str) cJSON_free(json_str);
}

int main(int argc, char *argv[]) {
  int portNum, socketDesc, flag, connectedSocketDesc;
  socklen_t fromLength;
  struct sockaddr_in server_address, from_address;
  char buffer[100];

  if (argc < 2) {
    printf("Usage is: server <portNumber> \n");
    return 1;
  }
  portNum = atoi(argv[1]);
  if ((socketDesc = socket(AF_INET, SOCK_STREAM, 0) < 0)) {
    printf("error on socket creation\n");
    return 1;
  }

  fromLength = sizeof(struct sockaddr_in);
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(portNum);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind(socketDesc, (struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    perror("bind");
    return 1;
  }
  listen(socketDesc, 5);

  while (1) {
    flag = 0;
    connectedSocketDesc =
        accept(socketDesc, (struct sockaddr *)&from_address, &fromLength);
    memset(buffer, 0, 100);
    int sizeOfMessage;
    int rc;
    if ((rc = read(connectedSocketDesc, &sizeOfMessage, sizeof(int))) <= 0) {
      printf("the other side closed the socket\nI will clean and wait for "
             "another connection\n");
      break;
    }
    printf("read %d bytes to get the filesize\n", rc);
    printf("the size of the message before converting is %d bytes\n",
           sizeOfMessage);
    sizeOfMessage = ntohl(sizeOfMessage);
    printf("the size of the message after converting is %d bytes\n",
           sizeOfMessage);

    int totalBytes = 0;
    char *charPtr = buffer;

    while (totalBytes < sizeOfMessage) {
      rc = read(connectedSocketDesc, charPtr, sizeOfMessage - totalBytes);
      if (rc <= 0) {
        printf("this is awkward... the other side quit while sending\n");
        flag = 1;
        break;
      }
      totalBytes += rc;
      charPtr += rc;
    }
    if (flag == 0) {
      cJSON *jsonResponse = cJSON_Parse(buffer);
      if (jsonResponse == NULL) {
        printf("error parsing JSON response\n");
        return 1;
      }

      cJSON *response =
          cJSON_GetObjectItemCaseSensitive(jsonResponse, "message");
      if (cJSON_IsString(response) && response->valuestring != NULL) {
        printf("Message: %s\n", response->valuestring);

        //response
        jsonResponse = cJSON_CreateObject();
        cJSON_AddStringToObject(jsonResponse, "response", "Message received and processed");
        char *json_str = cJSON_Print(jsonResponse);
        int messageSize = strlen(json_str);
        int convertedLength = htonl(messageSize);

        if (write(connectedSocketDesc, &convertedLength, sizeof(int)) < sizeof(int)) {
        printf("error sending message size\n");
        cleanup(socketDesc, jsonResponse, json_str);
        return 1;
    }

    if (write(connectedSocketDesc, json_str, messageSize) < messageSize) {
        printf("error sending message\n");
        cleanup(socketDesc, jsonResponse, json_str);
        return 1;
    }
      } else {
        printf("error deserializing json");
      }
      cJSON_Delete(jsonResponse);
      close(connectedSocketDesc);
    }
  }
}