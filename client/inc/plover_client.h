#include <gtk/gtk.h>
#include <regex.h>
#include "iniparser.h"
#include "dictionary.h"
#include "b64.h"
//include "encode.h"
//#include "decode.h"
//#include "aria.h"

#define	MAX_FCNT	20
#define	HOSTNAME	"localhost"
#define	EXCHANGE	"aa"
//#define	HOSTNAME	"plover-server"
//#define	EXCHANGE	"amq.direct"
#define	PORT 		5672
#define	VHOST		"PLOVER_VHOST"
#define	USERNAME	"plover"
#define	PASSWORD	"Whdms9500!"

const char *INI_UUID;


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
Fdata_Storage fDs[MAX_FCNT];		// 파일기준의 data구조체 //

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

void func_CreateIni (void);
int  func_ParseIni (char * ini_name);
int  func_SetRabbit();
int  func_Send();
int  func_VerChk();
int  func_Usrchk();
int  func_Uuid();
int  func_Detect (gchar *path);
int  compile_regex		(regex_t *r, const char *regex_text);
char match_regex_jnfg	(regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf);
char match_regex_d		(regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf);
char match_regex_p		(regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf);
void check_kind_of_data (const char *to_match, char *filepath, struct dirent *file, struct stat buf);
int  func_pdf2txt (char *filepath);
int  func_gtk_dialog_modal (int type, GtkWidget *widget, char *message);
int  func_file_eraser (int type);
void ARIA (unsigned char *);
int  BXLog (const char *, int , int , const char *, ...);
void func_Refresh_ScrollWindow();

// enrollment_window //
void e_enroll_btn_clicked			(GtkButton *e_enroll_btn,			gpointer *data);
void e_department_btn_clicked	(GtkButton *e_department_btn,		gpointer *data);
void e_jobtitle_cbxtext_changed	(GtkWidget *e_jobtitle_cbxtext,	gpointer *data);
void e_name_entry_activate		(GtkEntry  *e_name_entry, 			gpointer *data);

static GtkTreeModel	*e_create_and_fill_model (void);
static GtkWidget	*e_create_view_and_model (void);

gboolean	e_view_selection_func (GtkTreeSelection 	*selection,
										GtkTreeModel    		*model,
										GtkTreePath    		*path,
										gboolean        		 path_currently_selected,
										gpointer        		 userdata);

void dept_ok_btn_clicked_w	(GtkButton *dept_ok_btn,		gpointer *data);
void dept_ok_btn_clicked_e	(GtkButton *dept_ok_btn,		gpointer *data);
void dept_close_btn_clicked	(GtkButton *dept_close_btn,	gpointer *data);
/* end of enrollment_window */

// main_window //
void m_window_destroy();
void m_detect_btn_clicked		(GtkButton *m_detect_btn,		gpointer *data);
void m_setting_btn_clicked	(GtkButton *m_setting_btn,		gpointer *data);
/* end of main_window */

// detect_window #df //
void d_detect_btn_clicked		(GtkButton *d_detect_btn,		gpointer *data);
void d_option_btn_clicked		(GtkButton *d_option_btn,		gpointer *data);
void d_folder_btn_clicked		(GtkButton *d_folder_btn,		gpointer *data);
void d_close_btn_clicked		(GtkButton *d_close_btn,		gpointer *data);
void d_detect_entry_activate	(GtkEntry  *d_detect_entry,	gpointer *data);

gboolean	d_view_selection_func (GtkTreeSelection 	*selection,
										GtkTreeModel    		*model,
										GtkTreePath     		*path,
										gboolean        		 path_currently_selected,
										gpointer        		 userdata);
										
static GtkTreeModel	*d_create_and_fill_model (void);
static GtkWidget	*d_create_view_and_model (void);
/* end of detect_window */

// setting_window #sf //
void s_ok_btn_clicked			(GtkButton *s_ok_btn,		gpointer *data);
void s_cloese_btn_clicked		(GtkButton *s_cloese_btn,	gpointer *data);
void s_folder_btn_clicked		(GtkButton *s_folder_btn,	gpointer *data);
void s_usrchg_btn_clicked		(GtkButton *s_usrchg_btn,	gpointer *data);
void s_detect_entry_activate	(GtkEntry  *s_detect_entry,	gpointer *data);
void s_ip_entry_activate		(GtkEntry  *s_ip_entry,	gpointer *data);
void s_port_entry_activate	(GtkEntry  *s_port_entry,	gpointer *data);
/* end of setting_window */
