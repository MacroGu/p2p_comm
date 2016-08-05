#include <string>
#include <list>
#include <string.h>

#define SERVER_PORT 1688	//�������˿�
#define MAX_COMMAND 256
#define MAXRETRY 5

#define MAX_NAME_SIZE 15

/*p2pͨ��Э��*/

//iMessageTypeֵ
#define LOGIN 1
#define LOGOUT 2
#define GETALLUSER 3
#define P2PTRANS 4//������������򶴡�


//����p2p�ͻ���֮��ͨ��
#define P2PMESSAGE 100//������Ϣ
#define P2PMESSAGEACK 101 //�յ���Ϣ��Ӧ��
#define P2PSOMEONEWANTTOCALLYOU 102//��������ͻ��˷��ͣ�Ҫ��˿ͻ��˷���UDP�򶴰�
#define P2PPUNCH 103//�ͻ��˷��͵Ĵ򶴰������ն�Ӧ���Դ���Ϣ


#define MAX_PACKET_SIZE 1024

//�ֶ����ù��캯�����������ڴ�
template<class _T1> 
inline	void constructInPlace(_T1  *_Ptr)
{
	new (static_cast<void*>(_Ptr)) _T1();
}
/// �����䳤ָ��
#define BUFFER_CMD(cmd,name,len) char buffer##name[len]={0};\
														cmd *name=(cmd *)buffer##name;constructInPlace(name);

//�ͻ��ڵ���Ϣ
struct stUserListNode
{
	char userName[MAX_NAME_SIZE];			// �ڵ������
	unsigned int uiIP;				//  �ڵ��IP
	unsigned short usPORT;			// �ڵ�� PORT

	stUserListNode()
	{
		bzero(this, sizeof(*this));
	}
};

//Server��ͻ��˷��ʹ�������Ϣ
struct stTransMsg
{
	unsigned int uiIP;			// ��Ҫ���͸��ͻ��˵� IP
	unsigned short usPORT;		// ��Ҫ���͸��ͻ��˵� PORT

	stTransMsg()
	{
		uiIP = 0;
		usPORT = 0;
	}
};

//���ڿͻ���֮���ͨ��
struct stP2PMsg
{
	unsigned int uiSendLen;				// p2p ���͵���Ϣ����
	char cP2PCommUserName[MAX_NAME_SIZE];		// ��¼������Ϣ���û���

	stP2PMsg()
	{
		bzero(this, sizeof(*this));
	}
};

//ͨ����Ϣ��ʽ
struct stCommMsg
{
	unsigned int uiMsgType;				// �ͻ��˺ͷ�����ͨ�ŵ�����
	char cMyName[MAX_NAME_SIZE];		// ���ͻ����Լ�������	
	char  cToName[MAX_NAME_SIZE];		// ��Ҫp2p ͨ�ŵ� �ͻ�������

	stTransMsg  	transMsg;			// ������ת���� 
	stP2PMsg		p2pMsg;				// �ͻ���֮���p2pͨ��

	unsigned int   userNums;			// ȫ���Ŀͻ�������
	stUserListNode userList[0];			//����server��ͻ��˷��Ϳͻ��б�

	stCommMsg()
	{
		bzero(this, sizeof(*this));
	}

	unsigned int getSize() const { return sizeof(*this) + userNums * sizeof(stUserListNode); }
};

using namespace std;
typedef list<stUserListNode*> UserList;
