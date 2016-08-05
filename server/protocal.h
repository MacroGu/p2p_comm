#include <string>
#include <list>
#include <string.h>

#define SERVER_PORT 1688	//服务器端口
#define MAX_COMMAND 256
#define MAXRETRY 5

#define MAX_NAME_SIZE 15

/*p2p通信协议*/

//iMessageType值
#define LOGIN 1
#define LOGOUT 2
#define GETALLUSER 3
#define P2PTRANS 4//请求服务器“打洞”


//用于p2p客户端之间通信
#define P2PMESSAGE 100//发送消息
#define P2PMESSAGEACK 101 //收到消息的应答
#define P2PSOMEONEWANTTOCALLYOU 102//服务器向客户端发送，要求此客户端发送UDP打洞包
#define P2PPUNCH 103//客户端发送的打洞包，接收端应忽略此消息


#define MAX_PACKET_SIZE 1024

//手动调用构造函数，不分配内存
template<class _T1> 
inline	void constructInPlace(_T1  *_Ptr)
{
	new (static_cast<void*>(_Ptr)) _T1();
}
/// 声明变长指令
#define BUFFER_CMD(cmd,name,len) char buffer##name[len]={0};\
														cmd *name=(cmd *)buffer##name;constructInPlace(name);

//客户节点信息
struct stUserListNode
{
	char userName[MAX_NAME_SIZE];			// 节点的名字
	unsigned int uiIP;				//  节点的IP
	unsigned short usPORT;			// 节点的 PORT

	stUserListNode()
	{
		bzero(this, sizeof(*this));
	}
};

//Server向客户端发送打洞请求消息
struct stTransMsg
{
	unsigned int uiIP;			// 将要发送给客户端的 IP
	unsigned short usPORT;		// 将要发送给客户端的 PORT

	stTransMsg()
	{
		uiIP = 0;
		usPORT = 0;
	}
};

//用于客户端之间的通信
struct stP2PMsg
{
	unsigned int uiSendLen;				// p2p 发送的消息长度
	char cP2PCommUserName[MAX_NAME_SIZE];		// 记录发送消息的用户名

	stP2PMsg()
	{
		bzero(this, sizeof(*this));
	}
};

//通用消息格式
struct stCommMsg
{
	unsigned int uiMsgType;				// 客户端和服务器通信的类型
	char cMyName[MAX_NAME_SIZE];		// 本客户端自己的名字	
	char  cToName[MAX_NAME_SIZE];		// 想要p2p 通信的 客户端名字

	stTransMsg  	transMsg;			// 服务器转发的 
	stP2PMsg		p2pMsg;				// 客户端之间的p2p通信

	unsigned int   userNums;			// 全部的客户端数量
	stUserListNode userList[0];			//用于server向客户端发送客户列表

	stCommMsg()
	{
		bzero(this, sizeof(*this));
	}

	unsigned int getSize() const { return sizeof(*this) + userNums * sizeof(stUserListNode); }
};

using namespace std;
typedef list<stUserListNode*> UserList;
