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
#pragma warning(disable:4996) // ��������� �������������� _WINSOCK_DEPRECATED_NO_WARNINGS

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

	// ������� �������� ����������
	listen(server_socket, MAX_CLIENTS);


	// ������ ������ ��������� ������, ��� ����� ������
	// ����� ������������ �������
	// fd means "file descriptors"
	fd_set readfds; // https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-fd_set
	SOCKET client_socket[MAX_CLIENTS] = {};

	while (true) {
		// �������� ����� fdset
		FD_ZERO(&readfds);

		// �������� ������� ����� � fdset
		FD_SET(server_socket, &readfds);

		// �������� �������� ������ � fdset
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			SOCKET s = client_socket[i];
			if (s > 0) {
				FD_SET(s, &readfds);
			}
		}

		// ��������� ���������� �� ����� �� �������, ����-��� ����� NULL, ������� ����� ����������
		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		// ���� ���-�� ��������� �� ������-������, �� ��� �������� ����������
		SOCKET new_socket; // ����� ���������� �����
		sockaddr_in address;
		int addrlen = sizeof(sockaddr_in);
		if (FD_ISSET(server_socket, &readfds)) {
			if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
				perror("accept function error");
				return 5;
			}





			// �������� ����� ����� � ������ �������
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					cout << "Client #" << i << " is connected\n";
					break;
				}
			}
		}

		// ���� �����-�� �� ���������� ������� ���������� ���-��
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			SOCKET s = client_socket[i];
			// ���� ������ ������������ � ������� ������
			if (FD_ISSET(s, &readfds))
			{
				// �������� ��������� �������
				getpeername(s, (sockaddr*)&address, (int*)&addrlen);

				// ���������, ���� �� ��� �� ��������, � ����� ���������� �������� ���������
				// recv �� �������� ������� ���������� � ����� ������ (� �� ����� ��� printf %s ������������, ��� �� ����)

				// �������� ��������� ������� � ���������
				char client_message[DEFAULT_BUFLEN];
				int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
				client_message[client_message_length] = '\0';

				for (char& c : client_message) { //�������������� ������ � ������� ��������
					c = tolower(c);
				}

				string temp = client_message;
				temp += "\n";
				history.push_back(temp);

				cout << "client's message: " << client_message << endl;
				// ���������, ���������� �� ��������� � ����� �� �����
				bool isSprite = (strstr(client_message, sprite.c_str()) != nullptr);
				bool isBurger = (strstr(client_message, burger.c_str()) != nullptr);
				bool isFries = (strstr(client_message, fries.c_str()) != nullptr);

				// ���� ��������� �������� �����-���� �� ����, ��������� ��������������� ����� ��������

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