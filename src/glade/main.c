#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <gtk/gtk.h>

#include <regex.h>

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include "utils.h"
#include "utils.c"

#include "b64.h"
#include "encode.c"
#include "decode.c"

#define	MAX_ERROR_MSG	0x1000
#define	MAX_CNTF	50		// 최대 검출 파일 개수 //
#define	ERASER_SIZE	512		//1k
#define	ERASER_ENC_SIZE	896		//1k


typedef struct _Uuid_Storage
{
	char uuid[36];			// UUID //

}Uuid_Storage;
Uuid_Storage uIds;

typedef struct _Udata_Storage
{
	Uuid_Storage uIds;			// UUID //
	char uname[10];			// 사용자 이름 //
	char ujob[10];			// 사용자 직급 //
	char udept[20];			// 사용자 부서 //
	
}Udata_Storage;
Udata_Storage uDs;

typedef struct _Fdata_Storage
{
<<<<<<< HEAD
	Uuid_Storage uIds;			// UUID //
	char fname[100];			// 파일 이름 //
	uint jcnt;				// 주민번호 개수 //
	uint dcnt;				// 운전면허 개수 //
	uint fgcnt;				// 외국인등록번호 개수 //
	uint pcnt;				// 여권번호 개수 //
	uint fsize;				// 파일 크기 //
=======
	char fname[100];		// 파일 이름 //
	uint jcnt;			// 주민번호 개수 //
	uint dcnt;			// 운전면허 개수 //
	uint fgcnt;			// 외국인등록번호 개수 //
	uint pcnt;			// 여권번호 개수 //
	uint fsize;			// 파일 크기 //
>>>>>>> 9c219d17cd396ccdc27873d33828c4d8f9765a42
	char stat[20];			// 파일 상태 //
	char fpath[300];		// 파일 경로 //

}Fdata_Storage;
Fdata_Storage fDs[MAX_CNTF];		// 파일기준의 data구조체 //

static gchar *path;			// 검사 파일경로 //
static gchar *name;			// 등록 유저이름 //
static gchar *job;			// 등록 직급이름 //
static gchar *vs_dept;			// 등록 부서이름 //

static int	cntf = 0;		// 파일개수 cnt //
static char	chk_fname[100];		// 정규식돌고있는 파일이름 //
static char	chk_fpath[1024];	// 검출 결과에서 선택한 파일경로 //
static uint	chk_fsize;		// 검출 결과에서 선택한 파일크기 //
static int	chk_tf;			// chk_true or false //
//uint 	data_flag = 1;			// 민감정보 종류 확인 flag //

GtkWidget		*main_window,
			*m_userinfo_label,
			*enrollment_window,
			*e_jobtitle_cbxtext,
			*detect_window,
			*setting_window,
			*department_window,
			*d_progressbar_status,
			*d_progressbar,
			*window;
						
GtkEntry		*e_name_entry,
			*e_jobtitle_entry,
			*e_department_entry,
			*d_detect_entry;

GtkScrolledWindow	*d_scrolledwindow,
			*dept_scrolledwindow;

int func_send();
int func_uuid();

// enrollment_window #ef //
void e_enroll_btn_clicked	(GtkButton *e_enroll_btn,	gpointer *data);
void e_department_btn_clicked	(GtkButton *e_department_btn,	gpointer *data);
void e_jobtitle_cbxtext_changed	(GtkWidget *e_jobtitle_cbxtext, gpointer *data);
void dept_ok_btn_clicked_w	(GtkButton *dept_ok_btn,	gpointer *data);
void dept_ok_btn_clicked_e	(GtkButton *dept_ok_btn,	gpointer *data);

void dept_close_btn_clicked	(GtkButton *dept_close_btn,	gpointer *data);

void e_name_entry_activate	(GtkEntry *e_name_entry,	gpointer *data);

static GtkTreeModel	*e_create_and_fill_model (void);
static GtkWidget	*e_create_view_and_model (void);

gboolean	e_view_selection_func (GtkTreeSelection *selection,
					GtkTreeModel    *model,
					GtkTreePath     *path,
					gboolean         path_currently_selected,
					gpointer         userdata);
/* end of enrollment_window */

// main_window #mf //
void m_window_destroy();
void m_detect_btn_clicked	(GtkButton *m_detect_btn,	gpointer *data);
void m_setting_btn_clicked	(GtkButton *m_setting_btn,	gpointer *data);
/* end of main_window */

// detect_window #df //
void d_detect_btn_clicked	(GtkButton *d_detect_btn,	gpointer *data);
void d_option_btn_clicked	(GtkButton *d_option_btn,	gpointer *data);
void d_folder_btn_clicked	(GtkButton *d_folder_btn,	gpointer *data);
void d_close_btn_clicked	(GtkButton *d_close_btn,	gpointer *data);
void d_detect_entry_activate	(GtkEntry  *d_detect_entry,	gpointer *data);

gboolean	d_view_selection_func (GtkTreeSelection *selection,
					GtkTreeModel    *model,
					GtkTreePath     *path,
					gboolean         path_currently_selected,
					gpointer         userdata);
										
static GtkTreeModel	*d_create_and_fill_model (void);
static GtkWidget	*d_create_view_and_model (void);
/* end of detect_window */

// setting_window #sf //
void s_cloese_btn_clicked	(GtkButton *s_cloese_btn,	gpointer *data);
/* end of setting_window */

// 사용자 UUID parsing //
int func_uuid()
{
	FILE *uidfp = NULL;
    char strbuf[300];
    char *pstr = NULL;

	uidfp = fopen("/etc/fstab", "r");

	if (NULL == uidfp)
	{
        printf("fstab 파일을 열수 없습니다.\n");
		return 1;
	}

	while (feof (uidfp) == 0)
	{
		pstr = fgets( strbuf, sizeof(strbuf), uidfp);

		if(pstr != 0) // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸 //
		{
			if(pstr[0] == 'U' && pstr[42] == '/')
			{
				for (int i = 0; i < 36; i++)
				{
					uIds.uuid[i] = pstr[i+5];
					uDs.uIds.uuid[i] = pstr[i+5];
					fDs->uIds.uuid[i] = pstr[i+5];
				}
				printf("[%s]\n", uIds.uuid);
				printf("[%s]\n", uDs.uIds.uuid);
				printf("[%s]\n", fDs->uIds.uuid);
			}
		}
    }
    
    memset(strbuf, 0, sizeof(strbuf));
	fclose(uidfp);

	return 0;
}
// end of func_uuid(); //


// 계정이 있는지 확인: TRUE(1)=있다 FALSE(0)=없다 #cu //
int func_chk_user()
{
	chk_tf = FALSE;
	
	return chk_tf;
}
/* end of func_chk_user(); */


// Base64 encoding #b64 //
char *b64_encode (const unsigned char *src, size_t len, char *enc);
/* end of char *b64_encode(); */

//Compile the regular expression described by "regex_text" into "r"//
int compile_regex (regex_t *r, const char *regex_text)
{
	int status = regcomp(r, regex_text, REG_EXTENDED | REG_NEWLINE);

	if (status != 0)
	{
	char error_message[MAX_ERROR_MSG];

	regerror(status, r, error_message, MAX_ERROR_MSG);

	printf("Regex error compiling '%s': %s\n", regex_text, error_message);

	return 1;
	}

	return 0;
}
/* end of compile_regex(); */


//Match the string in "to_match" against the compiled regular expression in "r"//
// 주민등록번호, 외국인등록번호 정규식 #jfr //
char match_regex_jnfg (regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	/* "P" is a pointer into the string which points to the end of the
	previous match. */
	const char *p = to_match;
	/* "N_matches" is the maximum number of matches allowed. */
	const int n_matches = 100;
	/* "M" contains the matches found. */
	regmatch_t m[n_matches];

	// 버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교 //
	while (1)
	{
		int nomatch = regexec(r, p, n_matches, m, 0);

		if (nomatch)
		{
			printf("No more matches.\n");
			return 0;
		}

		else
		{
			for (int i = 0; i < n_matches; i++)
			{
				int start;

				if (m[i].rm_so == -1)
				{
					break;
				}

				start = m[i].rm_so + (p - to_match);

				// 주민번호, 외국인등록번호 정규식 검사 통과 //
				if (i == 0)
				{
					int chk = 0, jtmp = 0, fgtmp = 0, sum = 0;
					char buf_tmp[15] = {0,};

					// 주민번호, 외국인등록번호 유효성 검사 //
					for (int j = 0; j < 14; j++)
					{
						buf_tmp[j] = *(to_match + start + j);
						buf_tmp[j] -= 48;
					}

					sum = buf_tmp[0]*2 + buf_tmp[1]*3 + buf_tmp[2]*4 + buf_tmp[3]*5 + buf_tmp[4]*6 + buf_tmp[5]*7
					+ buf_tmp[7]*8 + buf_tmp[8]*9 + buf_tmp[9]*2 + buf_tmp[10]*3 + buf_tmp[11]*4 + buf_tmp[12]*5;

					chk = buf_tmp[13];
					jtmp = 11 - (sum % 11); // 주민번호 //
					fgtmp = 13 - (sum % 11); // 외국인번호 //

					if (jtmp >= 10)
					{
					jtmp -= 10;
					}

					if (fgtmp >= 10)
					{
					fgtmp -= 10;
					}

					// 주민번호 유효성 통과 //
					if (jtmp == chk)
					{
						int res = strcmp(chk_fname, file->d_name); // 같은파일 = 0 //

					if (res != 0)
					{
						cntf++;
					}

					// 읽고있는중인 파일 이름 저장 //
					strcpy(chk_fname, file->d_name);

					// 검출된 주민등록번호의 수 //
					fDs[cntf].jcnt++;

					// data 구조체에 저장 //
					strcpy(fDs[cntf].fpath, filepath);
					strcpy(fDs[cntf].fname, file->d_name);
					fDs[cntf].fsize = buf.st_size;
					strcpy(fDs[cntf].stat, "일반");

					printf("num: %d, jcnt: %d, dcnt: %d, fgcnt: %d, file_path: %s, file_name: %s, file_size: %dbyte\n",
						cntf, fDs[cntf].jcnt, fDs[cntf].dcnt, fDs[cntf].fgcnt, fDs[cntf].fpath, fDs[cntf].fname, fDs[cntf].fsize);
					}

					// 외국인등록번호 유효성 통과 //
					if (fgtmp == chk)
					{
					int res = strcmp(chk_fname, file->d_name); // 같은파일 = 0 //

					if (res != 0)
					{
						cntf++;
					}

					// 읽고있는중인 파일 이름 저장 //
					strcpy(chk_fname, file->d_name);

					// 검출된 외국인등록번호의 수 //
					fDs[cntf].fgcnt++;

					// data 구조체에 저장 //
					strcpy(fDs[cntf].fpath, filepath);
					strcpy(fDs[cntf].fname, file->d_name);
					fDs[cntf].fsize = buf.st_size;
					strcpy(fDs[cntf].stat, "일반");

					printf("num: %d, jcnt: %d, dcnt: %d, fgcnt: %d, pcnt: %d, file_path: %s, file_name: %s, file_size: %dbyte\n",
						cntf, fDs[cntf].jcnt, fDs[cntf].dcnt, fDs[cntf].fgcnt, fDs[cntf].pcnt, fDs[cntf].fpath, fDs[cntf].fname, fDs[cntf].fsize);
					}
				}
			}
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_jnfg(); */

// 운전면허 정규식 #dr //
char match_regex_d (regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	/* "P" is a pointer into the string which points to the end of the
	previous match. */
	const char *p = to_match;
	/* "N_matches" is the maximum number of matches allowed. */
	const int n_matches = 100;
	/* "M" contains the matches found. */
	regmatch_t m[n_matches];

	//버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교//
	while (1)
	{
		int nomatch = regexec(r, p, n_matches, m, 0);

		if (nomatch)
		{
			printf("No more matches.\n");
			return 0;
		}

		else
		{
			for (int i = 0; i < n_matches; i++)
			{
				if (m[i].rm_so == -1)
				{
				    break;
				}

				//운전면허 정규식 검사 통과//
				if (i == 0)
				{
					int res = strcmp(chk_fname, file->d_name); //같은파일 = 0 //

					if (res != 0)
					{
						cntf++;
					}

					// 읽고있는중인 파일 이름 저장 //
					strcpy(chk_fname, file->d_name);

					// 검출된 운전면허의 수 //
					fDs[cntf].dcnt++;

					// data 구조체에 저장 //
					strcpy(fDs[cntf].fpath, filepath);
					strcpy(fDs[cntf].fname, file->d_name);
					fDs[cntf].fsize = buf.st_size;
					strcpy(fDs[cntf].stat, "일반");

					printf("num: %d, jcnt: %d, dcnt: %d, fgcnt: %d, pcnt: %d, file_path: %s, file_name: %s, file_size: %dbyte\n",
						cntf, fDs[cntf].jcnt, fDs[cntf].dcnt, fDs[cntf].fgcnt, fDs[cntf].pcnt, fDs[cntf].fpath, fDs[cntf].fname, fDs[cntf].fsize);
				}
			}
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_d(); */


// 여권번호 정규식 #pr //
char match_regex_p (regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	/* "P" is a pointer into the string which points to the end of the
	previous match. */
	const char *p = to_match;
	/* "N_matches" is the maximum number of matches allowed. */
	const int n_matches = 100;
	/* "M" contains the matches found. */
	regmatch_t m[n_matches];

	// 버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교 //
	while (1)
	{
		int nomatch = regexec(r, p, n_matches, m, 0);

		if (nomatch)
		{
			printf("No more matches.\n");
			return 0;
		}

		else
		{
			for (int i = 0; i < n_matches; i++)
			{
				if (m[i].rm_so == -1)
				{
					break;
				}

				// 운전면허 정규식 검사 통과 //
				if (i == 0)
				{
					int res = strcmp(chk_fname, file->d_name); // 같은파일 = 0 //

					if (res != 0)
					{
						cntf++;
					}

					// 읽고있는중인 파일 이름 저장 //
					strcpy(chk_fname, file->d_name);

					// 검출된 운전면허의 수 //
					fDs[cntf].pcnt++;

					// data 구조체에 저장 //
					strcpy(fDs[cntf].fpath, filepath);
					strcpy(fDs[cntf].fname, file->d_name);
					fDs[cntf].fsize = buf.st_size;
					strcpy(fDs[cntf].stat, "일반");

					printf("num: %d, jcnt: %d, dcnt: %d, fgcnt: %d, pcnt: %d, file_path: %s, file_name: %s, file_size: %dbyte\n",
						cntf, fDs[cntf].jcnt, fDs[cntf].dcnt, fDs[cntf].fgcnt, fDs[cntf].pcnt, fDs[cntf].fpath, fDs[cntf].fname, fDs[cntf].fsize);
				}
			}
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_p(); */

// data 종류 확인 //
void check_kind_of_data (const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	regex_t r;
	const char *regex_text;

	/*
	switch(data_flag) //나중에 민감정보 종류 선택 검출 할 때 사용
	{
		case 1:
			regex_text = "([0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-4][0-9]{6})"; //주민번호 정규식//
			compile_regex(&r, regex_text); //정규식 컴파일//
			match_regex_j(&r, to_match, filepath, file, buf);

			break;

		case 2:
			regex_text = "[0-9]{2}-[0-9]{6}-[0-9]{2}"; //운전면허 정규식//
			compile_regex(&r, regex_text); //정규식 컴파일//
			//match_regex_d(&r, to_match, filepath, file, buf);
	}
	*/

	// 주민번호, 외국인등록번호 정규식 //
	regex_text = "([0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-4][0-9]{6})";
	compile_regex(&r, regex_text); // 정규식 컴파일 //
	match_regex_jnfg(&r, to_match, filepath, file, buf);

	// 운전면허 정규식 //
	regex_text = "[0-9]{2}-[0-9]{6}-[0-9]{2}";
	compile_regex(&r, regex_text); // 정규식 컴파일 //
	match_regex_d(&r, to_match, filepath, file, buf);

	// 여권번호 정규식 //
	regex_text = "[a-zA-Z]{2}[0-9]{7}";
	compile_regex(&r, regex_text); // 정규식 컴파일 //
	match_regex_p(&r, to_match, filepath, file, buf);
}
/* end of check_kind_of_data(); */

// 폴더, 파일 스캔 후 검출 #detect //
int func_detect (gchar *path)
{
	DIR *dp = NULL;
	FILE *fp = NULL;
	struct dirent *file = NULL;
	struct stat buf;
	char filepath[300];
	char buffer[5000];
	const char *find_text;

	if ((dp = opendir(path)) == NULL)
	{
		printf("폴더를 열수 없습니다.\n");

		return -1;
	}

	while ((file = readdir(dp)) != NULL)
	{
	// filepath에 현재 path넣기 //
	sprintf(filepath, "%s/%s", path, file->d_name);
	lstat(filepath, &buf);

		// 폴더 //
		if (S_ISDIR(buf.st_mode))
		{
			// .이거하고 ..이거 제외 //
			if ((!strcmp(file->d_name, ".")) || (!strcmp(file->d_name, "..")))
			{
			continue;
			}

			// 안에 폴더로 재귀함수 //
			func_detect(filepath);
		}

		// 파일 //
		else if (S_ISREG(buf.st_mode))
		{
			fp = fopen(filepath, "r");

			if (NULL == fp)
			{
				printf("파일을 열수 없습니다.\n");
				return 1;
			}

			// 버퍼 크기만큼 읽고 find_text에 넣어서 정규식검사로 이동 //
			while (feof(fp) == 0)
			{
				fread(buffer, sizeof(char), sizeof(buffer), fp);
				find_text = buffer;
				check_kind_of_data(find_text, filepath, file, buf);
				find_text = NULL;
			}

			// 메모리관리(초기화), 파일닫기 //
			memset(buffer, 0, sizeof(buffer));
			fclose(fp);
			printf("Close FILE\n");
			chk_fname[0] = 0; // 초기화 //
		}
	}
	
	closedir(dp);

	printf("Close DIR\n");

	func_send();

	return  0;
}
/* end of func_detect(); */

// 전송 #send //
int func_send()
{
	char *hostname;
	int port, status;
	char *exchange;
	char *routingkey;
	static char *enc;

	amqp_socket_t *socket = NULL;
	amqp_connection_state_t conn;
	amqp_bytes_t reply_to_queue;

	hostname = "127.0.0.1";
	port = atoi("5672");
	exchange = "aa";
	routingkey = "ka";
	
	/*
	 establish a channel that is used to connect RabbitMQ server
	*/
	conn = amqp_new_connection();

	socket = amqp_tcp_socket_new(conn);
	
	if (!socket)
	{
	die("creating TCP socket");
	}

	status = amqp_socket_open(socket, hostname, port);
	
	if (status)
	{
	die("opening TCP socket");
	}


	die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
																		"guest", "guest"),
																		"Logging in");

					
	amqp_channel_open(conn, 1);

	die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");


	/*
	 create private reply_to queue
	*/
	{
		amqp_queue_declare_ok_t *r = amqp_queue_declare(
			conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
			
			
		die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");

		
		reply_to_queue = amqp_bytes_malloc_dup(r->queue);
		if (reply_to_queue.bytes == NULL)
		{
			fprintf(stderr, "Out of memory while copying queue name");
			return 1;
		}
	}

	/*
	 send the message
	*/
	{
		/*
		  set properties
		*/
		amqp_basic_properties_t props;
		props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG |
					   AMQP_BASIC_DELIVERY_MODE_FLAG | AMQP_BASIC_REPLY_TO_FLAG |
					   AMQP_BASIC_CORRELATION_ID_FLAG;
		props.content_type = amqp_cstring_bytes("text/plain");
		props.delivery_mode = 2; /* persistent delivery mode */
		props.reply_to = amqp_bytes_malloc_dup(reply_to_queue);
		
		if (props.reply_to.bytes == NULL)
		{
			fprintf(stderr, "Out of memory while copying queue name");
			return 1;
		}
		props.correlation_id = amqp_cstring_bytes("1");

		/*
		  publish // data넣어서 보내기 #input_data
		*/
		if(chk_tf == 1)
		{
			char message[1024];
			gdouble percent = 0.0;
			memset(message, 0x00, strlen(message));

			for(int i = 1; i <= cntf; i++)
			{
				percent = i / cntf * 100;
				size_t in_len = sizeof(fDs[i]);
				//printf("fds[%d]: %ld\n", i ,in_len); //구조체 크기확인
				enc = b64_encode((unsigned char *)&fDs[i], in_len, enc);
				printf("enc_data: %s\n", enc);
				printf("UUID: %s, cnt: %d, jumin: %d, driver: %d, forign: %d, pass: %d, fsize: %d, fstat: %s, fpath: %s\n",
							fDs->uIds.uuid, i, fDs[i].jcnt, fDs[i].dcnt, fDs[i].fgcnt, fDs[i].pcnt, fDs[i].fsize, fDs[i].stat, fDs[i].fpath);

				sprintf( message, "%.0f%% Complete", percent);
				gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), percent);
				gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);

				die_on_error(amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange),
								amqp_cstring_bytes(routingkey), 0, 0,
								&props, amqp_cstring_bytes(enc)), "Publishing");
			}
		}
		
		else
		{
			die_on_error(amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange),
							amqp_cstring_bytes(routingkey), 0, 0,
							&props, amqp_cstring_bytes(enc)), "Publishing");
		}
		amqp_bytes_free(props.reply_to);
	}
	
	
	/*
	wait an answer
	*/
	/*
	{
		amqp_basic_consume(conn, 1, reply_to_queue, amqp_empty_bytes, 0, 1, 0,
								amqp_empty_table);
		die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");


		{
			amqp_frame_t frame;
			int result;

			amqp_basic_deliver_t *d;
			amqp_basic_properties_t *p;
			size_t body_target;
			size_t body_received;

			for (;;)
			{
				amqp_maybe_release_buffers(conn);
				result = amqp_simple_wait_frame(conn, &frame);
				printf("Result: %d\n", result);
			
				if (result < 0)
				{
					break;
				}

				printf("Frame type: %u channel: %u\n", frame.frame_type, frame.channel);
				if (frame.frame_type != AMQP_FRAME_METHOD)
				{
					continue;
				}

				printf("Method: %s\n", amqp_method_name(frame.payload.method.id));
				if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD)
				{
					continue;
				}

				d = (amqp_basic_deliver_t *)frame.payload.method.decoded;
				printf("Delivery: %u exchange: %.*s routingkey: %.*s\n",
					   (unsigned)d->delivery_tag, (int)d->exchange.len,
					   (char *)d->exchange.bytes, (int)d->routing_key.len,
					   (char *)d->routing_key.bytes);

				result = amqp_simple_wait_frame(conn, &frame);
				if (result < 0)
				{
					break;
				}

				if (frame.frame_type != AMQP_FRAME_HEADER)
				{
					fprintf(stderr, "Expected header!");
					abort();
				}
				p = (amqp_basic_properties_t *)frame.payload.properties.decoded;
				if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG)
				{
					printf("Content-type: %.*s\n", (int)p->content_type.len,
						 (char *)p->content_type.bytes);
				}
				printf("----\n");

				body_target = (size_t)frame.payload.properties.body_size;
				body_received = 0;

				while (body_received < body_target)
				{
					result = amqp_simple_wait_frame(conn, &frame);
					
					if (result < 0)
					{
						break;
					}

					if (frame.frame_type != AMQP_FRAME_BODY)
					{
						fprintf(stderr, "Expected body!");
						abort();
					}

					body_received += frame.payload.body_fragment.len;
					assert(body_received <= body_target);

					amqp_dump(frame.payload.body_fragment.bytes,
								frame.payload.body_fragment.len);
				}

				if (body_received != body_target)
				{
					// Can only happen when amqp_simple_wait_frame returns <= 0 //
					// We break here to close the connection //
					break;
				}

				// everything was fine, we can quit now because we received the reply //
				break;
			}
		}
	}
	*/


	/*
	 closing
	*/
	//die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
															
	//die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
															
	die_on_error(amqp_destroy_connection(conn), "Ending connection");

	return TRUE;
}
/* end of func_send(); */

// gtk_dialog_modal //
int func_gtk_dialog_modal(int type, GtkWidget *widget, char *message)
{
	GtkWidget *dialog, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;
	int	rtn = GTK_RESPONSE_REJECT;

	switch(type)
	{
		case 0 :
			dialog = gtk_dialog_new_with_buttons("Dialog", GTK_WINDOW(widget), flags, 
						("_OK"), GTK_RESPONSE_ACCEPT, NULL );
			break;

		case 1 :
			dialog = gtk_dialog_new_with_buttons("Dialog", GTK_WINDOW(widget), flags,
						("_OK"), GTK_RESPONSE_ACCEPT, 
						("_Cancel"), GTK_RESPONSE_REJECT, NULL );
			break;

		default :
			break;
	}

	label = gtk_label_new(message);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_widget_show_all(dialog);

	rtn = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);
	return(rtn);	
}

// 삭제 #delete //
int func_file_eraser(int type)
{
	FILE *fp;
	int mode = R_OK | W_OK;
	char MsgTmp[5];
	gdouble size = 0.0;
	char *msize;

	if( access( chk_fpath, mode ) != 0 )
	{
		func_gtk_dialog_modal(0, window, "\n    파일이 삭제 가능한 상태가 아닙니다.    \n");
	}
	
	else
	{
		msize = malloc(ERASER_SIZE);
		fp = fopen(chk_fpath, "w");
		
		for( int i=0 ; i < type ; i++ )
		{
			size = 0;

			while (size<chk_fsize)
			{   
				switch(i)
				{
					case 0 :
						MsgTmp[0] = 'A';
						memset( msize, MsgTmp[0], ERASER_SIZE );
						break;
						
					case 1 :
						MsgTmp[0] = '^';
						memset( msize, MsgTmp[0], ERASER_SIZE );
						break;
						
					case 2 :
						srand(time(NULL));
						if( size < ERASER_SIZE )
							for( int j=0 ; j < ERASER_SIZE ; j++ )
								msize[j] = 'A' + (random() % 26);
						break;
						
					case 3 :
						MsgTmp[0] = 'Z';
						memset( msize, MsgTmp[0], ERASER_SIZE );
						break;
						
					case 4 :
						MsgTmp[0] = 'A';
						memset( msize, MsgTmp[0], ERASER_SIZE );
						break;
						
					case 5 :
						MsgTmp[0] = '^';
						memset( msize, MsgTmp[0], ERASER_SIZE );
						break;
						
					case 6 :
						srand(time(NULL));
						if( size < ERASER_SIZE )
							for( int j=0 ; j < ERASER_SIZE ; j++ )
								msize[j] = 'A' + (random() % 26);
						break;
						
					default :
						break;
				}
			}
			
			fseek( fp, 0L, SEEK_SET );
		}
		
		fclose(fp);
		free(msize);
	}

	remove( chk_fpath );
	func_gtk_dialog_modal(0, window, "\n    삭제가 완료되었습니다.    \n");
	
	return( TRUE );
}
// end of func_file_eraser(); //

// main_window function #mf //
void m_detect_btn_clicked (GtkButton *m_detect_btn, gpointer *data)
{
	gtk_widget_show(detect_window);
	
	return;
}

void m_setting_btn_clicked (GtkButton *m_setting_btn, gpointer *data)
{
	gtk_widget_show(setting_window);
	
	return;
}

void m_window_destroy()
{
	gtk_main_quit();
	
	return;
}
/* end of main_window function */



// detect_window function #df //
void d_detect_entry_activate (GtkEntry *d_detect_entry, gpointer *data)
{
	path = (gchar *)gtk_entry_get_text(d_detect_entry);
	g_print("선택한 폴더 위치: %s\n", path);
	
	return;
}


void d_folder_btn_clicked (GtkButton *d_folder_btn, gpointer *data)
{
	GtkWidget *d_filechooserdialog;
	
    d_filechooserdialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(data), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
			("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all(d_filechooserdialog);
    
	gint resp = gtk_dialog_run(GTK_DIALOG(d_filechooserdialog));

    if( resp == GTK_RESPONSE_ACCEPT)
    {
	gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d_filechooserdialog));
    } 
	path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d_filechooserdialog));

	gtk_entry_set_text(GTK_ENTRY (data), path);


	gtk_widget_destroy(d_filechooserdialog);

	g_print("선택한 폴더 위치: %s\n", path);

	return;
}

	// treeview function #tf//
enum
{
	d_treeview_num = 0,
	d_treeview_filename,
	d_treeview_jcnt,
	d_treeview_dcnt,
	d_treeview_fgcnt,
	d_treeview_pcnt,
	d_treeview_stat,
	d_treeview_size,
	d_treeview_fileloca,
	NUM_COLS
} ;

gboolean
d_view_selection_func 	(GtkTreeSelection *selection,
			 GtkTreeModel     *model,
			 GtkTreePath      *path,
			 gboolean          path_currently_selected,
			 gpointer          userdata)
{
	GtkTreeIter iter;
	gchar *vs_fpath;
	guint vs_fsize = 0;
	
	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		gtk_tree_model_get(model, &iter, d_treeview_fileloca, &vs_fpath, -1);
		gtk_tree_model_get(model, &iter, d_treeview_size, vs_fsize, -1);

		if (!path_currently_selected)
		{
			g_print ("%s 선택.\n", vs_fpath);
		}
		
		else
		{
			g_print ("%s 선택 해제.\n", vs_fpath);
		}
		
		strcpy(chk_fpath ,vs_fpath);
		chk_fsize = vs_fsize;
		
		g_free(vs_fpath);
		vs_fsize = 0;
	}

	return TRUE; /* allow selection state to change */
}


static GtkTreeModel *
d_create_and_fill_model (void)
{
	GtkTreeStore	*treestore;
	GtkTreeIter	iter;

	treestore = gtk_tree_store_new(NUM_COLS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
					G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);
	
	for(int i = 1; i <= cntf; i++)
	{
		gtk_tree_store_append(treestore, &iter, NULL);
		gtk_tree_store_set (treestore, &iter,
					  d_treeview_num, i,
					  d_treeview_filename,	fDs[i].fname,
					  d_treeview_jcnt,	fDs[i].jcnt,
					  d_treeview_dcnt,	fDs[i].dcnt,
					  d_treeview_fgcnt,	fDs[i].fgcnt,
					  d_treeview_pcnt,	fDs[i]. pcnt,
					  d_treeview_stat,	fDs[i].stat,
					  d_treeview_size,	fDs[i].fsize,
					  d_treeview_fileloca,	fDs[i].fpath,
					  -1);
	}

	return GTK_TREE_MODEL(treestore);
}

static GtkWidget *
d_create_view_and_model (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer		*renderer;
	GtkWidget		*view;
	GtkTreeModel		*model;
	GtkTreeSelection	*selection;
	
	view = gtk_tree_view_new();

	// Column #컬럼명 //
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title(col, "번호");

	// pack tree view column into tree view //
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	renderer = gtk_cell_renderer_text_new();

	// pack cell renderer into tree view column //
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_num);

	// --- Column #파일 이름 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "파일 이름");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_filename);

	// --- Column #주민번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "주민번호");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_jcnt);

	// --- Column #운전면허 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "운전면허");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_dcnt);
	
	// --- Column #외국인등록번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "외국인등록");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_fgcnt);
	
	// --- Column #여권번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "여권번호");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_pcnt);

	// --- Column #상태 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "상태");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_stat);
	
	// --- Column #파일 크기 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "파일 크기");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_size);
	
	// --- Column #파일 위치 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "파일 위치");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", d_treeview_fileloca);
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	
	gtk_tree_selection_set_select_function(selection, d_view_selection_func, NULL, NULL);



	model = d_create_and_fill_model();

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);

	g_object_unref(model); // destroy model automatically with view //

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)),
							  GTK_SELECTION_MULTIPLE);

	return view;
}
	/* end of treeview function */


void d_detect_btn_clicked (GtkButton *d_detect_btn, gpointer *data)
{
	GtkWidget *view;
	
	func_detect(path);
	
	view = d_create_view_and_model();
	gtk_container_add (GTK_CONTAINER(d_scrolledwindow), view);
	gtk_widget_show_all ((GtkWidget *)d_scrolledwindow);

	//strcpy(fDs[cntf].stat, "일반");

	return;
}

void d_option_btn_clicked (GtkButton *d_option_btn, gpointer *data)
{
	gtk_widget_show(setting_window);
	
	return;
}

void d_encrypt_btn_clicked (GtkButton *d_encrypt_btn, gpointer *data)//미구현//
{
	return;
}

void d_delete_btn_clicked (GtkButton *d_delete_btn, gpointer *data)
{
	char	message[1134];
	
	if( chk_fpath[0] == 0x00 )
	{
		func_gtk_dialog_modal(0, window, "\n    대상파일이 선택되지 않았습니다.    \n");
	}
	else
	{
		sprintf( message, 
			"\n    삭제 후에 복구가 불가능 합니다.\n    아래 파일을 삭제하시겠습니까?\n    [ %s ]    \n", chk_fpath);
    
		if( func_gtk_dialog_modal(1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			func_file_eraser(3);
		}
		else
		{
			printf("취소 되었습니다.\n");
		}
	}
	
	return;
}

void d_close_btn_clicked (GtkButton *d_close_btn, gpointer *data)
{
	gtk_widget_hide(GTK_WIDGET(data));
	
	return;
}

void detect_window_destroy (GtkWidget *detect_window, gpointer *data)
{
	gtk_widget_destroy(GTK_WIDGET(detect_window));
	
	return;
}
/* end of detect_window function */



// enrollment_window function #ef //

void e_name_entry_activate (GtkEntry *e_name_entry, gpointer *data)
{
	name = (gchar *)gtk_entry_get_text(e_name_entry);
	strcpy(uDs.uname, name);
	printf("%s\n", name);
	
	return;
}

void e_jobtitle_cbxtext_changed	(GtkWidget *e_jobtitle_cbxtext, gpointer *data)
{
	job = (gchar *)gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(e_jobtitle_cbxtext))));
	strcpy(uDs.ujob, job);
	printf("%s\n", uDs.ujob);

	return;
}


enum
{
	e_treeview_col,
	NUM_COL
} ;

gboolean
e_view_selection_func 	(GtkTreeSelection *selection,
			 GtkTreeModel     *model,
			 GtkTreePath      *path,
			 gboolean          path_currently_selected,
			 gpointer          userdata)
{
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		gtk_tree_model_get(model, &iter, e_treeview_col, &vs_dept, -1);

		if (!path_currently_selected)
		{
			g_print ("%s 선택.\n", vs_dept);
		}
		
		else
		{
			g_print ("%s 선택 해제.\n", vs_dept);
		}
		
		strcpy(uDs.udept, vs_dept);

		g_free(vs_dept);
	}

	return TRUE; /* allow selection state to change */
}

static GtkTreeModel *
e_create_and_fill_model (void)
{
	GtkTreeStore  *e_treestore;
	GtkTreeIter    parent, child1;

	e_treestore = gtk_tree_store_new (NUM_COL, G_TYPE_STRING);

	/*대표이사 0*/
	gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set(e_treestore, &parent,
					 e_treeview_col, "대표이사",
					 -1);

	/*임원실 1*/
	gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set(e_treestore, &parent,
					 e_treeview_col, "임원실",
					 -1);

	/*대전지사 2*/
	gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set(e_treestore, &parent,
					 e_treeview_col, "대전지사",
					 -1);
					 
	/*경영혁신팀 3*/
	gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set(e_treestore, &parent,
					 e_treeview_col, "경영혁신팀",
					 -1);

	/*솔루션사업부 4*/
	gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set(e_treestore, &parent,
					 e_treeview_col, "솔루션사업부",
					 -1);
		/*솔루션사업부 영업팀 8*/
		gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
		gtk_tree_store_set(e_treestore, &parent,
						 e_treeview_col, "솔루션사업부 영업팀",
						 -1);
		/*솔루션사업부 사업팀 9*/
		gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
		gtk_tree_store_set(e_treestore, &parent,
						 e_treeview_col, "솔루션사업부 기술팀",
						 -1);

	/*보안인프라사업부 5*/
	gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set(e_treestore, &parent,
					 e_treeview_col, "보안인프라사업부",
					 -1);
		/*보안인프라사업부 영업팀 10*/
		gtk_tree_store_append(e_treestore, &child1, &parent);
		gtk_tree_store_set(e_treestore, &child1,
						 e_treeview_col, "보안인프라사업부 영업팀",
						 -1);
		/*보안인프라사업부 기술팀 11*/
		gtk_tree_store_append(e_treestore, &child1, &parent);
		gtk_tree_store_set(e_treestore, &child1,
						 e_treeview_col, "보안인프라사업부 기술팀",
						 -1);

	/*부설연구소 6*/
	gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set(e_treestore, &parent,
					 e_treeview_col, "부설연구소",
					 -1);
		/*부설연구소 개발1팀 12*/
		gtk_tree_store_append(e_treestore, &child1, &parent);
		gtk_tree_store_set(e_treestore, &child1,
						 e_treeview_col, "부설연구소 개발1팀",
						 -1);
		/*부설연구소 개발2팀 13*/
		gtk_tree_store_append(e_treestore, &child1, &parent);
		gtk_tree_store_set(e_treestore, &child1,
						 e_treeview_col, "부설연구소 개발2팀",
						 -1);

	/*특수사업부 7*/
	gtk_tree_store_append(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set(e_treestore, &parent,
					 e_treeview_col, "특수사업부",
					 -1);
		/*특수사업부 자사품 TF팀 14*/
		gtk_tree_store_append(e_treestore, &child1, &parent);
		gtk_tree_store_set(e_treestore, &child1,
						 e_treeview_col, "특수사업부 자사품 TF팀",
						 -1);
		/*특수사업부 컨설팅팀 15*/
		gtk_tree_store_append(e_treestore, &child1, &parent);
		gtk_tree_store_set(e_treestore, &child1,
						 e_treeview_col, "특수사업부 컨설팅팀",
						 -1);
                     
	return GTK_TREE_MODEL(e_treestore);
}

static GtkWidget *
e_create_view_and_model (void)
{
	GtkTreeViewColumn	*e_col;
	GtkCellRenderer		*e_renderer;
	GtkWidget		*e_view;
	GtkTreeModel		*e_model;
	GtkTreeSelection	*e_selection;
	
	e_view = gtk_tree_view_new();

	// Column #부서 //
	e_col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title(e_col, "(주)조은아이앤에스 조직도");

	gtk_tree_view_append_column(GTK_TREE_VIEW(e_view), e_col);
	e_renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(e_col, e_renderer, TRUE);
	
	gtk_tree_view_column_add_attribute(e_col, e_renderer, "text", e_treeview_col);
	
	e_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(e_view));
	
	gtk_tree_selection_set_select_function(e_selection, e_view_selection_func, NULL, NULL);
	
	e_model = e_create_and_fill_model();

	gtk_tree_view_set_model(GTK_TREE_VIEW(e_view), e_model);

	g_object_unref(e_model); // destroy model automatically with view //
	
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(e_view)),
                              GTK_SELECTION_SINGLE);

	return e_view;
}

void dept_ok_btn_clicked_w	(GtkButton *dept_ok_btn,	gpointer *data)
{
	gtk_widget_hide(department_window);
}

void dept_ok_btn_clicked_e	(GtkButton *dept_ok_btn,	gpointer *data)
{
	gtk_entry_set_text(GTK_ENTRY(data), uDs.udept);

	return;
}

void dept_close_btn_clicked	(GtkButton *dept_close_btn,	gpointer *data)
{
	gtk_widget_hide(department_window);
}

void e_department_btn_clicked (GtkButton *e_department_btn,	gpointer *data)
{
	GtkWidget *e_view;
	
	e_view = e_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (dept_scrolledwindow), e_view);
	gtk_widget_show_all ((GtkWidget *)department_window);
}

void e_enroll_btn_clicked (GtkButton *e_enroll_btn, gpointer *data)
{
	char *usrinfostr = malloc(sizeof(char) * 10);
	sprintf(usrinfostr, "%s %s", uDs.uname, uDs.ujob);

	gtk_widget_hide(enrollment_window);

	gtk_label_set_text(GTK_LABEL (m_userinfo_label), usrinfostr);
	gtk_widget_show(main_window);
	chk_tf = TRUE;

	printf("부서: %s 사용자: %s, 직급: %s \n", uDs.udept, uDs.uname, uDs.ujob);
	//printf("%s\n", usrinfostr);

	gtk_main();
	
	return;
}


/* end of enrollment_window */



// setting_window function #sf //
void s_cloese_btn_clicked (GtkButton *setting_window, gpointer *data)
{
	gtk_widget_hide(GTK_WIDGET (data));
	
	return;
}
/* end of setting_window function */



// main #main //
int main (int argc, char *argv[])
{
	GtkBuilder	*builder;

	func_uuid();

	gtk_init(&argc, &argv);

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "main.glade", NULL);

	main_window		= GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
	enrollment_window	= GTK_WIDGET(gtk_builder_get_object(builder, "enrollment_window"));
	department_window	= GTK_WIDGET(gtk_builder_get_object(builder, "department_window"));
	detect_window		= GTK_WIDGET(gtk_builder_get_object(builder, "detect_window"));
	setting_window		= GTK_WIDGET(gtk_builder_get_object(builder, "setting_window"));
	d_progressbar 		= GTK_WIDGET(gtk_builder_get_object(builder, "d_progressbar"));
	d_scrolledwindow	= GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "d_scrolledwindow"));
	dept_scrolledwindow	= GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "dept_scrolledwindow"));
	m_userinfo_label	= GTK_WIDGET(gtk_builder_get_object(builder, "m_userinfo_label"));
	gtk_window_set_position(GTK_WINDOW(detect_window), GTK_WIN_POS_CENTER);

	// 닫기x 버튼을 hide로 바꾸기, -버튼 활성화 하고 싶으면 glade에서 modal 해제 //
	g_signal_connect(detect_window, "delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	g_signal_connect(setting_window, "delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	g_signal_connect(enrollment_window, "delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);

	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(builder);

	func_chk_user(chk_tf);

	if (chk_tf == FALSE)	// TRUE(1)=있다 //
	{
		gtk_widget_show(enrollment_window);
		gtk_main();
	}

	if (chk_tf == TRUE) 	// FALSE(0)=없다 //
	{
		gtk_widget_show(main_window);
		gtk_main();
	}

	gtk_widget_show(window); 

	return 0;
}
