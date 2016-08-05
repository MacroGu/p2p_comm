#include"protocal.h"
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32")

#include <string>
#include <thread>
#include <iostream>
#include <sstream>


UserList ClientList;//�ͻ��ڵ��б�
SOCKET PrimaryUDP;
std::string UserName = "";//��½�û���
std::string ServerIP = "45.115.147.200";//������IP
bool RecvedACK;//�յ���Ϣ��Ӧ��

void WinsockInit();
SOCKET CreateSocket(int type);
void BindSocket(SOCKET sock);
void ConnectToServer(SOCKET sock,std::string username,std::string serverip);
void  OutputMessage();
void ParseCommand(char *CommandLine);
bool sendMessage(char *userName,char *Message);
void RecvThreadProc(); 


int main(int argc,char**argv)
{
	WinsockInit();
	PrimaryUDP=CreateSocket(SOCK_DGRAM);
	BindSocket(PrimaryUDP);


	std::stringstream ss;
	ss << ::GetTickCount();
	ss >> UserName;
	UserName = "PC_" + UserName ;

	ConnectToServer(PrimaryUDP, UserName, ServerIP);
	std::thread threadHandle = std::thread(RecvThreadProc);

	OutputMessage();
	for(;;)
	{
		char Command[MAX_COMMAND];
		gets_s(Command);
		ParseCommand(Command);
	}

	return 0; 

}

void WinsockInit()
{
	WSADATA wsadata;
	if(WSAStartup(MAKEWORD(2,2),&wsadata)!=0)
	{
		printf("Winsock cann't startup!\n");
		exit(0);
	}
	else
	{
		printf("Using %s (Status: %s)\n",wsadata.szDescription,wsadata.szSystemStatus);
	}
}

SOCKET CreateSocket(int type)
{
	SOCKET sock=socket(AF_INET,type,0);
	if(sock<0)
	{
		printf("Create socket error!\n");
	}
	return sock;
}

void BindSocket(SOCKET sock)
{
	sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port = htons(7000);
	sin.sin_addr.S_un.S_addr=htonl(INADDR_ANY);

	 bind(sock, (struct sockaddr*)&sin, sizeof(sin)); 		// todo �ж��Ƿ�ɹ�
	{

	}
}

void ConnectToServer(SOCKET sock,std::string username,std::string serverip)
{
	sockaddr_in remote;
	remote.sin_family=AF_INET;
	remote.sin_port=htons(SERVER_PORT);
	remote.sin_addr.S_un.S_addr=inet_addr(serverip.c_str());

	stCommMsg sendbuf;
	sendbuf.uiMsgType = LOGIN;
	memcpy(sendbuf.cMyName,username.c_str(),username.length());

	int send_count;//�˱������ڵ���
	send_count = sendto(sock, (const char*)&sendbuf, sendbuf.getSize(), 0, (const sockaddr*)&remote, sizeof(remote));
}

void  OutputMessage()
{
	printf("You can input you command:\n");
	printf("Command Type:\"send\",\"exit\",\"getu\" \n");
	printf("Example: send Username Message\n");
	printf("         exit\n");
	printf("         getu\n");
	printf("\n");
}

//��������
void ParseCommand(char *CommandLine)
{

	if(strlen(CommandLine)<4)
	{
		printf("CommandLine error!\n");
	}
	char Command[10];
	strncpy(Command,CommandLine,4);
	Command[4]='\0';

	if(strcmp(Command,"exit")==0)
	{
		stCommMsg sendbuf;
		sendbuf.uiMsgType = LOGOUT;
		strncpy(sendbuf.cMyName, UserName.c_str(), MAX_NAME_SIZE);

		sockaddr_in server;
		server.sin_family=AF_INET;
		server.sin_port=htons(SERVER_PORT);
		server.sin_addr.S_un.S_addr=inet_addr(ServerIP.c_str());

		sendto(PrimaryUDP,(const char*)&sendbuf, sendbuf.getSize(),0,(const sockaddr*)&server,sizeof(server));
		shutdown(PrimaryUDP,2);
		closesocket(PrimaryUDP);
		exit(0);
	}
	else if(strcmp(Command,"getu")==0)
	{
		stCommMsg sendbuf;
		sendbuf.uiMsgType=GETALLUSER;

		sockaddr_in server;
		server.sin_family=AF_INET;
		server.sin_port=htons(SERVER_PORT);
		server.sin_addr.S_un.S_addr=inet_addr(ServerIP.c_str());

		sendto(PrimaryUDP, (const char*)&sendbuf, sendbuf.getSize(), 0, (const sockaddr*)&server, sizeof(server));
	}
	else if(strcmp(Command,"send")==0)
	{
		char sendname[20];
		char message[MAX_COMMAND];

		int i = 0;
		for( i=5;;i++)
		{
			if(CommandLine[i]!=' ')
				sendname[i-5]=CommandLine[i];
			else
			{
				sendname[i-5]='\0';
				break;
			}
		}

		strcpy(message,&(CommandLine[i+1]));

		if(sendMessage(sendname,message))
			printf("Send OK!\n");
		else
			printf("Send Failure!\n");
	}
	else
		printf("No this command!\n");

}

/*p2p���庯��----������Ϣ*/

//���̣�����ֱ����ĳ���ͻ�������IP������Ϣ�������ǰû�С��򶴡�������Ϣ�޷����ͣ����Ͷ˵ȴ���ʱ��
//��ʱ�󣬷��Ͷ˷������󵽷�����Ҫ�󡰴򶴡���Ҫ�����������ÿͻ��򱾻����ʹ���Ϣ��
//�ظ�MAXRETRY��
bool sendMessage(char *userName,char *Message)
{
	bool FindUser=false;
	unsigned int UserIP;
	unsigned short UserPort;
	char message[MAX_COMMAND];

	for (auto ClientList_iter = ClientList.begin(); ClientList_iter != ClientList.end(); ++ClientList_iter)
	{
		if (strcmp((*ClientList_iter)->userName, userName) == 0)
		{
			UserIP = (*ClientList_iter)->uiIP;
			UserPort = (*ClientList_iter)->usPORT;
			FindUser = true;
		}
	}
	
	if(!FindUser)
		return false;

	strcpy(message,Message);
	for(int trytime=0;trytime<MAXRETRY;trytime++)
	{
		RecvedACK=false;
		sockaddr_in remote;
		remote.sin_family=AF_INET;
		remote.sin_port=htons(UserPort);
		remote.sin_addr.S_un.S_addr=htonl(UserIP);
		
		stCommMsg MessageHead;
		MessageHead.uiMsgType = P2PMESSAGE;
		MessageHead.p2pMsg.uiSendLen = (int)strlen(message)+1;
		strcpy(MessageHead.p2pMsg.cP2PCommUserName,UserName.c_str());
		//����p2p��Ϣͷ
		int send_count=sendto(PrimaryUDP,(const char*)&MessageHead,sizeof(MessageHead),0,(const sockaddr*)&remote,sizeof(remote));
		//����p2p��Ϣ��
		send_count=sendto(PrimaryUDP,(const char*)&message,MessageHead.p2pMsg.uiSendLen,0,(const sockaddr*)&remote,sizeof(remote));
		
		//�ȴ�������Ϣ�̸߳�RecvedACK��־
		for(int i=0;i<10;i++)
		{
			if(RecvedACK)
				return true;
			else
				Sleep(300);
		}
		
		//���û�н��յ�Ŀ�������Ļ�Ӧ����ΪĿ�������Ķ˿�
		//ӳ��û�д򿪣���ô�������󵽷�����Ҫ�󡰴򶴡���
		sockaddr_in server;
		server.sin_family=AF_INET;
		server.sin_port=htons(SERVER_PORT);
		server.sin_addr.S_un.S_addr=inet_addr(ServerIP.c_str());

		stCommMsg transMessage;
		transMessage.uiMsgType = P2PTRANS;
		strcpy(transMessage.cToName,userName);

		sendto(PrimaryUDP,(const char*)&transMessage,sizeof(transMessage),0,(const sockaddr*)&server,sizeof(server));
		Sleep(100);//�ȴ��Է��ȷ�����Ϣ
	}
	return false;
}
		
//������Ϣ�߳�
void RecvThreadProc()
{
	sockaddr_in remote;
	int nLen=sizeof(remote);
	char buf[MAX_PACKET_SIZE];

	for(;;)
	{
		int ret = recvfrom(PrimaryUDP, (char*)buf, MAX_PACKET_SIZE, 0, (sockaddr*)&remote, &nLen);
		stCommMsg* recvbuf = (stCommMsg*)buf;
		if(ret<=0)
		{	
			printf("recv error!\n");
			continue;
		}
		else
		{
			unsigned int mMsgType = recvbuf->uiMsgType;
			switch (mMsgType)
			{
			case GETALLUSER:
				{

					unsigned int usercount = recvbuf->userNums;
					ClientList.clear();
					printf("Have %d users logined server!\n", usercount);
					for (int i = 0; i < usercount; i++)
					{
						stUserListNode* user = new stUserListNode();
						memcpy(user->userName, recvbuf->userList[i].userName, MAX_NAME_SIZE);
						user->uiIP = recvbuf->userList[i].uiIP;
						user->usPORT = recvbuf->userList[i].usPORT;
						ClientList.push_back(user);
						printf("Username: %s\n", user->userName);
						in_addr temp;
						temp.S_un.S_addr = htonl(user->uiIP);
						printf("UserIP: %s\n", inet_ntoa(temp));
						printf("UserPort: %d\n", user->usPORT);
						printf("\n");
					}
					break;
				}
			case P2PMESSAGE:
				{
					char *recvmessage = new char[recvbuf->p2pMsg.uiSendLen];
					int recv = recvfrom(PrimaryUDP, recvmessage, MAX_PACKET_SIZE, 0, (sockaddr*)&remote, &nLen);
					recvmessage[recv-1]='\0';
					if(recv<=0)
						printf("Recv Message Error!\n");
					else
					{
						in_addr tmp;
						tmp.S_un.S_addr=remote.sin_addr.S_un.S_addr;
						printf("Message is from:%s(%s)\n", recvbuf->p2pMsg.cP2PCommUserName, inet_ntoa(tmp));
						printf("%s\n",recvmessage);

						stCommMsg sendbuf;
						sendbuf.uiMsgType = P2PMESSAGEACK;
						sendto(PrimaryUDP, (const char*)&sendbuf, sendbuf.getSize(), 0, (const sockaddr*)&remote, sizeof(remote));
					}
					delete []recvmessage;
					break;
				}
			case P2PMESSAGEACK://�յ���Ϣ��Ӧ��
				{
					RecvedACK=true;
					break;
				}
			case P2PSOMEONEWANTTOCALLYOU:
				{
					//���յ��������ָ��IP��
					printf("Recv p2psomeonewanttocallyou data(from server)!\n");

					sockaddr_in remote;
					remote.sin_family=AF_INET;
					remote.sin_port = htons(recvbuf->transMsg.usPORT);
					remote.sin_addr.S_un.S_addr = htonl(recvbuf->transMsg.uiIP);

					//UDP hole punching
					stCommMsg holeMessage;
					holeMessage.uiMsgType=P2PPUNCH;
					sendto(PrimaryUDP,(const char*)&holeMessage,sizeof(holeMessage),0,(const sockaddr*)&remote,sizeof(remote));
					break;
				}
			case P2PPUNCH:
				{
					// �Է����͵Ĵ���Ϣ������
					printf("Recv P2Ptransack data(from client)!\n");
					printf("Finish UDP hole punching!\n");
					break;
				}
			}
		}
	}

	return ;
}
