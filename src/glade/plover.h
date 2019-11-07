#include <gtk/gtk.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include "utils.h"
#include "utils.c"
#include "b64.h"
#include "encode.c"
#include "decode.c"

#define	HOSTNAME	"localhost"
#define	EXCHANGE	"aa"
//#define	HOSTNAME	"plover-server"
//#define	EXCHANGE	"amq.direct"
#define	PORT 		5672
#define	VHOST		"PLOVER_VHOST"
#define	USERNAME	"plover"
#define	PASSWORD	"Whdms9500!"

#pragma pack(push, 1)
typedef struct _Udata_Storage
{
	char uuid[37];			// UUID //	
	char uname[20];			// 사용자 이름 //
	char ujob[20];			// 사용자 직급 //
	char udept[30];			// 사용자 부서 //
	
}Udata_Storage;
Udata_Storage uDs;

typedef struct _Fdata_Storage
{
	char uuid[37];			// UUID //
	char fname[100];			// 파일 이름 //
	uint jcnt;				// 주민번호 개수 //
	uint dcnt;				// 운전면허 개수 //
	uint fgcnt;				// 외국인등록번호 개수 //
	uint pcnt;				// 여권번호 개수 //
	uint fsize;				// 파일 크기 //
	char stat[20];			// 파일 상태 //
	char fpath[300];			// 파일 경로 //

}Fdata_Storage;
Fdata_Storage fDs[50];		// 파일기준의 data구조체 //

typedef struct _SFdata_Storage
{
	char uuid[37];			// UUID //
	char fname[100];			// 파일 이름 //
	uint fsize;				// 파일 크기 //
	char stat[20];			// 파일 상태 //
	char fpath[300];			// 파일 경로 //

}SFdata_Storage;
SFdata_Storage sfDs;		// 선택파일 data구조체 //
#pragma pack(pop)


static int  status, port;
static char *routingkey, *username, *password;
static const char *vhost;
static amqp_socket_t *socket = NULL;
static amqp_connection_state_t conn;
static amqp_bytes_t reply_to_queue;

int func_SetRabbit();
int func_Send();
int func_VerChk();
int func_Usrchk();
int func_Uuid();
int func_Detect (gchar *path);
