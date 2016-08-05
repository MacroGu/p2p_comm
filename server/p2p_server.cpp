#include "protocal.h"
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <thread>

UserList ClientList;//客户节点列表
int PrimaryUDP;

stUserListNode GetUser(char *userName);


int main(int argc, char **argv)
{
	PrimaryUDP = socket(AF_INET, SOCK_DGRAM, 0);
	if (PrimaryUDP < 0)
	{
		std::cout << "创建 socket 失败！" << std::endl;
		return 0;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(PrimaryUDP, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		std::cout << "bind socket failed!" << std::endl;
		return 0;
	}

	sockaddr_in sender;
	char buf[MAX_PACKET_SIZE] = {0};

	// 开始主循环
	// 1。读取客户端登陆和登出消息，记录客户端节点列表
	for ( ; ; )
	{
		stCommMsg* mRecvMsg = (stCommMsg*)buf;
		socklen_t nLen = sizeof (sender);
		int ret = recvfrom(PrimaryUDP, (char*)mRecvMsg , MAX_PACKET_SIZE, 0, (sockaddr*)&sender, &nLen);

		if (ret < 0)
		{
			std::cout << "recv error " << std::endl;
			continue;
		}
		else
		{
			unsigned int mMsgType = mRecvMsg->uiMsgType;
			switch (mMsgType)
			{
				case LOGIN:
					{
						std::cout << "has a client login, user name: " << mRecvMsg->cMyName << std::endl;
						stUserListNode* currentUser = new stUserListNode;
						memcpy(currentUser->userName, mRecvMsg->cMyName,MAX_NAME_SIZE);
						currentUser->uiIP = ntohl(sender.sin_addr.s_addr);
						currentUser->usPORT = ntohs(sender.sin_port);
						ClientList.push_back(currentUser);			//  do not exclude same name user

						char sendBuf[MAX_PACKET_SIZE] = {0};
						stCommMsg* sendbuf = (stCommMsg*) sendBuf;
						// 服务器应答消息
						sendbuf->uiMsgType = GETALLUSER;
						for (auto ClientList_iter = ClientList.begin(); ClientList_iter != ClientList.end(); ++ClientList_iter)
						{
							memcpy(sendbuf->userList[sendbuf->userNums].userName, (*ClientList_iter)->userName,MAX_NAME_SIZE);
							sendbuf->userList[sendbuf->userNums].uiIP = (*ClientList_iter)->uiIP;
							sendbuf->userList[sendbuf->userNums].usPORT = (*ClientList_iter)->usPORT;
							++sendbuf->userNums;
						}

						sendto(PrimaryUDP, (const char*)sendbuf, sendbuf->getSize(),0, 
								(const sockaddr*)&sender, sizeof(sender));
						break;
					}
				case LOGOUT:
					{
						std::cout << "has a client logout, name:" << mRecvMsg->cMyName << std::endl;
						for (auto ClientList_iter = ClientList.begin(); ClientList_iter != ClientList.end(); ++ClientList_iter)
						{
							if (strcmp((*ClientList_iter)->userName, mRecvMsg->cMyName) == 0)
							{
								ClientList_iter = ClientList.erase(ClientList_iter);
							}
						}

						break;
					}
				case GETALLUSER:
					{
						char sendBuf[MAX_PACKET_SIZE] = {0};
						stCommMsg* sendbuf = (stCommMsg*) sendBuf;
						// 服务器应答消息
						sendbuf->uiMsgType = GETALLUSER;
						for (auto ClientList_iter = ClientList.begin(); ClientList_iter != ClientList.end(); ++ClientList_iter)
						{
							memcpy(sendbuf->userList[sendbuf->userNums].userName, (*ClientList_iter)->userName,MAX_NAME_SIZE);
							sendbuf->userList[sendbuf->userNums].uiIP = (*ClientList_iter)->uiIP;
							sendbuf->userList[sendbuf->userNums].usPORT = (*ClientList_iter)->usPORT;
							++sendbuf->userNums;
						}

						sendto(PrimaryUDP, (const char*)sendbuf, sendbuf->getSize(),0, 
								(const sockaddr*)&sender, sizeof(sender));
						unsigned int nodecount = ClientList.size();
						std::cout << "want get all user list" << nodecount << std::endl;

						break;
					}
				case P2PTRANS:
					{
						//某个客户希望服务器向另一客户发送一“打洞”消息
						std::cout << mRecvMsg->cMyName << " wants to p2p with :" << mRecvMsg->cToName << std::endl;	
						stUserListNode user=GetUser(mRecvMsg->cToName);

						sockaddr_in remote;
						remote.sin_family=AF_INET;
						remote.sin_port=htons(user.usPORT);
						remote.sin_addr.s_addr=htonl(user.uiIP);

						in_addr temp;
						temp.s_addr=htonl(user.uiIP);
						printf("the address is %s,and the port is %d\n",inet_ntoa(temp),user.usPORT);

						stCommMsg mTransMsg;
						mTransMsg.uiMsgType = P2PSOMEONEWANTTOCALLYOU;
						mTransMsg.transMsg.uiIP = ntohl(sender.sin_addr.s_addr); 
						mTransMsg.transMsg.usPORT = ntohs(sender.sin_port);

						sendto(PrimaryUDP, (const char*)&mTransMsg, sizeof(stCommMsg),0,(const sockaddr*)&remote,sizeof(remote));

						break;
					}
			}
			
		}
	
	}


	return 0;
}

stUserListNode GetUser(char *userName)
{
	for (auto ClientList_iter = ClientList.begin(); ClientList_iter != ClientList.end(); ++ClientList_iter)
	{
		if (strcmp((*ClientList_iter)->userName, userName) == 0)
			return *(*ClientList_iter);
	}
	
	std::cout << "can not find user: " << userName << std::endl;

	exit(0);
}
