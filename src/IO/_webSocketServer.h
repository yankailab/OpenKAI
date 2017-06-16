/*
 * _webSocketServer.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef SRC_NETWORK_WEBSOCKETSERVER_H_
#define SRC_NETWORK_WEBSOCKETSERVER_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"
#include "../IO/_IOBase.h"
#include "_TCPsocket.h"

#define N_LISTEN 1
#define N_SOCKET 1

namespace kai
{

class _webSocketServer: public _ThreadBase
{
public:
	_webSocketServer();
	virtual ~_webSocketServer();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void complete(void);
	bool draw(void);
	_TCPsocket* getFirstSocket(void);

private:
	bool handler(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_webSocketServer*) This)->update();
		return NULL;
	}

public:
	uint16_t	m_listenPort;
	int			m_nListen;
	string		m_strStatus;

	int m_socket;
	struct sockaddr_in m_webSocketServerAddr;

	list<_TCPsocket*> m_lSocket;
	int			m_nSocket;

};

}

#endif


/*

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "websocket.h"

#define PORT 8088
#define BUF_LEN 0xFFFF
#define PACKET_DUMP

uint8_t gBuffer[BUF_LEN];

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int safeSend(int clientSocket, const uint8_t *buffer, size_t bufferSize)
{
    #ifdef PACKET_DUMP
    printf("out packet:\n");
    fwrite(buffer, 1, bufferSize, stdout);
    printf("\n");
    #endif
    ssize_t written = send(clientSocket, buffer, bufferSize, 0);
    if (written == -1) {
        close(clientSocket);
        perror("send failed");
        return EXIT_FAILURE;
    }
    if (written != bufferSize) {
        close(clientSocket);
        perror("written not all bytes");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void clientWorker(int clientSocket)
{
    memset(gBuffer, 0, BUF_LEN);
    size_t readedLength = 0;
    size_t frameSize = BUF_LEN;
    enum wsState state = WS_STATE_OPENING;
    uint8_t *data = NULL;
    size_t dataSize = 0;
    enum wsFrameType frameType = WS_INCOMPLETE_FRAME;
    struct handshake hs;
    nullHandshake(&hs);

    #define prepareBuffer frameSize = BUF_LEN; memset(gBuffer, 0, BUF_LEN);
    #define initNewFrame frameType = WS_INCOMPLETE_FRAME; readedLength = 0; memset(gBuffer, 0, BUF_LEN);

    while (frameType == WS_INCOMPLETE_FRAME) {
        ssize_t readed = recv(clientSocket, gBuffer+readedLength, BUF_LEN-readedLength, 0);
        if (!readed) {
            close(clientSocket);
            perror("recv failed");
            return;
        }
        #ifdef PACKET_DUMP
        printf("in packet:\n");
        fwrite(gBuffer, 1, readed, stdout);
        printf("\n");
        #endif
        readedLength+= readed;
        assert(readedLength <= BUF_LEN);

        if (state == WS_STATE_OPENING) {
            frameType = wsParseHandshake(gBuffer, readedLength, &hs);
        } else {
            frameType = wsParseInputFrame(gBuffer, readedLength, &data, &dataSize);
        }

        if ((frameType == WS_INCOMPLETE_FRAME && readedLength == BUF_LEN) || frameType == WS_ERROR_FRAME) {
            if (frameType == WS_INCOMPLETE_FRAME)
                printf("buffer too small");
            else
                printf("error in incoming frame\n");

            if (state == WS_STATE_OPENING) {
                prepareBuffer;
                frameSize = sprintf((char *)gBuffer,
                                    "HTTP/1.1 400 Bad Request\r\n"
                                    "%s%s\r\n\r\n",
                                    versionField,
                                    version);
                safeSend(clientSocket, gBuffer, frameSize);
                break;
            } else {
                prepareBuffer;
                wsMakeFrame(NULL, 0, gBuffer, &frameSize, WS_CLOSING_FRAME);
                if (safeSend(clientSocket, gBuffer, frameSize) == EXIT_FAILURE)
                    break;
                state = WS_STATE_CLOSING;
                initNewFrame;
            }
        }

        if (state == WS_STATE_OPENING) {
            assert(frameType == WS_OPENING_FRAME);
            if (frameType == WS_OPENING_FRAME) {
                // if resource is right, generate answer handshake and send it
                if (strcmp(hs.resource, "/echo") != 0) {
                    frameSize = sprintf((char *)gBuffer, "HTTP/1.1 404 Not Found\r\n\r\n");
                    safeSend(clientSocket, gBuffer, frameSize);
                    break;
                }

                prepareBuffer;
                wsGetHandshakeAnswer(&hs, gBuffer, &frameSize);
                freeHandshake(&hs);
                if (safeSend(clientSocket, gBuffer, frameSize) == EXIT_FAILURE)
                    break;
                state = WS_STATE_NORMAL;
                initNewFrame;
            }
        } else {
            if (frameType == WS_CLOSING_FRAME) {
                if (state == WS_STATE_CLOSING) {
                    break;
                } else {
                    prepareBuffer;
                    wsMakeFrame(NULL, 0, gBuffer, &frameSize, WS_CLOSING_FRAME);
                    safeSend(clientSocket, gBuffer, frameSize);
                    break;
                }
            } else if (frameType == WS_TEXT_FRAME) {
                uint8_t *recievedString = NULL;
                recievedString = malloc(dataSize+1);
                assert(recievedString);
                memcpy(recievedString, data, dataSize);
                recievedString[ dataSize ] = 0;

                prepareBuffer;
                wsMakeFrame(recievedString, dataSize, gBuffer, &frameSize, WS_TEXT_FRAME);
                free(recievedString);
                if (safeSend(clientSocket, gBuffer, frameSize) == EXIT_FAILURE)
                    break;
                initNewFrame;
            }
        }
    } // read/write cycle

    close(clientSocket);
}

int main(int argc, char** argv)
{
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        error("create socket failed");
    }

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(PORT);
    if (bind(listenSocket, (struct sockaddr *) &local, sizeof(local)) == -1) {
        error("bind failed");
    }

    if (listen(listenSocket, 1) == -1) {
        error("listen failed");
    }
    printf("opened %s:%d\n", inet_ntoa(local.sin_addr), ntohs(local.sin_port));

    while (TRUE) {
        struct sockaddr_in remote;
        socklen_t sockaddrLen = sizeof(remote);
        int clientSocket = accept(listenSocket, (struct sockaddr*)&remote, &sockaddrLen);
        if (clientSocket == -1) {
            error("accept failed");
        }

        printf("connected %s:%d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
        clientWorker(clientSocket);
        printf("disconnected\n");
    }

    close(listenSocket);
    return EXIT_SUCCESS;
}

 */


/*
 <!DOCTYPE html>
<html>
<head>
    <title>Websocket client</title>
    <link href="http://netdna.bootstrapcdn.com/twitter-bootstrap/2.3.1/css/bootstrap-combined.min.css" rel="stylesheet">
    <script src="http://code.jquery.com/jquery.js"></script>
</head>
<body>
    <div class="container">
        <h1 class="page-header">Websocket client</h1>

        <form action="" class="form-inline" id="connectForm">
            <div class="input-append">
                <input type="text" class="input-large" value="ws://localhost:8088/echo" id="wsServer">
                <button class="btn" type="submit" id="connect">Connect</button>
                <button class="btn" disabled="disabled" id="disconnect">Disconnect</button>
            </div>
        </form>
        <form action="" id="sendForm">
            <div class="input-append">
                <input class="input-large" type="text" placeholder="message" id="message" disabled="disabled">
                <button class="btn btn-primary" type="submit" id="send" disabled="disabled">send</button>
            </div>
        </form>
        <hr>
        <ul class="unstyled" id="log"></ul>
    </div>
    <script type="text/javascript">
        $(document).ready(function() {
            var ws;

            $('#connectForm').on('submit', function() {
                if ("WebSocket" in window) {
                    ws = new WebSocket($('#wsServer').val());
                    ws.onopen = function() {
                        $('#log').append('<li><span class="badge badge-success">websocket opened</span></li>');
                        $('#wsServer').attr('disabled', 'disabled');
                        $('#connect').attr('disabled', 'disabled');
                        $('#disconnect').removeAttr('disabled');
                        $('#message').removeAttr('disabled').focus();
                        $('#send').removeAttr('disabled');
                    };

                    ws.onerror = function() {
                        $('#log').append('<li><span class="badge badge-important">websocket error</span></li>');
                    };

                    ws.onmessage = function(event) {
                        $('#log').append('<li>recieved: <span class="badge">' + event.data + '</span></li>');
                    };

                    ws.onclose = function() {
                        $('#log').append('<li><span class="badge badge-important">websocket closed</span></li>');
                        $('#wsServer').removeAttr('disabled');
                        $('#connect').removeAttr('disabled');
                        $('#disconnect').attr('disabled', 'disabled');
                        $('#message').attr('disabled', 'disabled');
                        $('#send').attr('disabled', 'disabled');
                    };
                } else {
                    $('#log').append('<li><span class="badge badge-important">WebSocket NOT supported in this browser</span></li>');
                }

                return false;
            });
            $('#sendForm').on('submit', function() {
                var message = $('#message').val();
                ws.send(message);
                $('#log').append('<li>sended: <span class="badge">' + message + '</span></li>');

                return false;
            });
            $('#disconnect').on('click', function() {
                ws.close();

                return false;
            });
        });
    </script>
</body>
</html>
 */
