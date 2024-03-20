#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define MAX_CLIENTS 30
#define DEFAULT_BUFLEN 4096
#define BURGER 5
#define SPRITE 1
#define FRIES  3


#pragma comment(lib, "ws2_32.lib") // Winsock library
#pragma warning(disable:4996) // отключаем предупреждение _WINSOCK_DEPRECATED_NO_WARNINGS

SOCKET server_socket;

vector<string> history;

int main() {

	string sprite = "sprite";
	string burger = "burger";
	string fries = "fries";
	int waitTime = 0;
	string waitMsg;
	string resultMsg = "pick up your order! it's done!";

	system("title Server");

	puts("Start server... DONE.");
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		return 1;
	}

	// create a socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket: %d", WSAGetLastError());
		return 2;
	}
	// puts("Create socket... DONE.");

	// prepare the sockaddr_in structure
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	// bind socket
	if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 3;
	}

	// puts("Bind socket... DONE.");

	// слушать входящие соединения
	listen(server_socket, MAX_CLIENTS);


	// размер нашего приемного буфера, это длина строки
	// набор дескрипторов сокетов
	// fd means "file descriptors"
	fd_set readfds; // https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-fd_set
	SOCKET client_socket[MAX_CLIENTS] = {};

	while (true) {
		// очистить сокет fdset
		FD_ZERO(&readfds);

		// добавить главный сокет в fdset
		FD_SET(server_socket, &readfds);

		// добавить дочерние сокеты в fdset
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			SOCKET s = client_socket[i];
			if (s > 0) {
				FD_SET(s, &readfds);
			}
		}

		// дождитесь активности на любом из сокетов, тайм-аут равен NULL, поэтому ждите бесконечно
		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		// если что-то произошло на мастер-сокете, то это входящее соединение
		SOCKET new_socket; // новый клиентский сокет
		sockaddr_in address;
		int addrlen = sizeof(sockaddr_in);
		if (FD_ISSET(server_socket, &readfds)) {
			if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
				perror("accept function error");
				return 5;
			}





			// добавить новый сокет в массив сокетов
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					cout << "Client #" << i << " is connected\n";
					break;
				}
			}
		}

		// если какой-то из клиентских сокетов отправляет что-то
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			SOCKET s = client_socket[i];
			// если клиент присутствует в сокетах чтения
			if (FD_ISSET(s, &readfds))
			{
				// получить реквизиты клиента
				getpeername(s, (sockaddr*)&address, (int*)&addrlen);

				// проверьте, было ли оно на закрытие, а также прочитайте входящее сообщение
				// recv не помещает нулевой терминатор в конец строки (в то время как printf %s предполагает, что он есть)

				// Получить реквизиты клиента и сообщение
				char client_message[DEFAULT_BUFLEN];
				int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
				client_message[client_message_length] = '\0';

				for (char& c : client_message) { //преобразование текста к нижнему регистру
					c = tolower(c);
				}

				string temp = client_message;
				temp += "\n";
				history.push_back(temp);

				cout << "client's message: " << client_message << endl;
				// Проверяем, содержится ли сообщение в одном из строк
				bool isSprite = (strstr(client_message, sprite.c_str()) != nullptr);
				bool isBurger = (strstr(client_message, burger.c_str()) != nullptr);
				bool isFries = (strstr(client_message, fries.c_str()) != nullptr);

				// Если сообщение содержит какое-либо из слов, добавляем соответствующее время ожидания

				if (isSprite) {
					waitTime += SPRITE;
				}

				if (isBurger) {
					waitTime += BURGER;
				}

				if (isFries) {
					waitTime += FRIES;
				}
				waitMsg = "wait " + to_string(waitTime) + " seconds, we are preparing your order";
				send(client_socket[i], waitMsg.c_str(), waitMsg.size(), 0);



				Sleep(waitTime * 1000);

				send(client_socket[i], resultMsg.c_str(), resultMsg.size(), 0);





			}
		}
	}

	WSACleanup();
}