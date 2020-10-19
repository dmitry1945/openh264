#include "TCPClient.h"

TCPClient::TCPClient()
{
	sock = -1;
	port = 0;
	address = "";
}

bool TCPClient::setup(string address, int port)
{
	if (sock == -1)
	{
//		sock = socket(AF_INET, SOCK_DCCP, 0);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1)
		{
			cout << "Could not create socket. Error -" << strerror(errno) << endl;
		}
	}
	if ((signed)inet_addr(address.c_str()) == -1)
	{
		struct hostent* he;
		struct in_addr** addr_list;
		if ((he = gethostbyname(address.c_str())) == NULL)
		{
			herror("gethostbyname");
			cout << "Failed to resolve hostname\n";
			return false;
		}
		addr_list = (struct in_addr**)he->h_addr_list;
		for (int i = 0; addr_list[i] != NULL; i++)
		{
			server.sin_addr = *addr_list[i];
			break;
		}
	}
	else
	{
		server.sin_addr.s_addr = inet_addr(address.c_str());
	}

	int sndbuf;
	sndbuf = 200000;      /* a prime number */
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
	sndbuf = 200000;      /* a prime number */
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &sndbuf, sizeof(sndbuf));

	int rcvBufferSize;
	unsigned int sockOptSize = sizeof(rcvBufferSize);
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvBufferSize, &sockOptSize);
	printf("initial socket receive buf %d\n", rcvBufferSize);


	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return false;
	}
	return true;
}

bool TCPClient::Send(string data)
{
	if(sock != -1) 
	{
		if( send(sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
		{
			cout << "Send failed : " << data << endl;
			return false;
		}
	}
	else
		return false;
	return true;
}

bool TCPClient::Send(void* data, int len, int id)
{
    if (sock != -1)
    {
		rxtx_packet* packet = (rxtx_packet*)this->packet_data;
		rxtx_packet::fill((uint8_t*)data, len, packet);

		int error_code;
		socklen_t error_code_size = sizeof(error_code);
		getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
		if (error_code != 0)
		{
			return false;
		}
		//if (send(sock, data, len, 0) < 0)
		if (send(sock, this->packet_data, len + offsetof(rxtx_packet, data), 0) < 0)
		{
            cout << "ERROR:     Send failed : len = " << len << endl;
            return false;
        }
	}
    else
        return false;
    return true;
}

string TCPClient::receive(int size)
{
  	char buffer[size];
	memset(&buffer[0], 0, sizeof(buffer));

  	string reply;
	if( recv(sock , buffer , size, 0) < 0)
  	{
	    	cout << "receive failed!" << endl;
		return nullptr;
  	}
	buffer[size-1]='\0';
  	reply = buffer;
  	return reply;
}

string TCPClient::read()
{
  	char buffer[1] = {};
  	string reply;
  	while (buffer[0] != '\n') {
    		if( recv(sock , buffer , sizeof(buffer) , 0) < 0)
    		{
      			cout << "receive failed!" << endl;
			return nullptr;
    		}
		reply += buffer[0];
	}
	return reply;
}

void TCPClient::exit()
{
    close( sock );
	sock = -1;
}
