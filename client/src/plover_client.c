/******************************************************************************/
/*  Source      : main.cpp                                    */
/*  Description :                                             */
/*  Rev. History: Ver   Date    Description                   */
/*----------------------------------------------------------------------------*/
/*                1.0   2019-12 Initial version               */
/******************************************************************************/
#include "plover_common.h"
#include "plover_client.h"

#define	PROGRESS_SIZE	102400		//100k
#define	MAX_ERROR_MSG	0x1000
#define	ERASER_SIZE		512			//1k
#define	ERASER_ENC_SIZE	896			//1k

clock_t start, end;
/*- Log define ------------*/
char	*LogName;
#define LOGFILE LogName
#define ERR LOGFILE,1,__LINE__
#define INF LOGFILE,2,__LINE__
#define WRN LOGFILE,3,__LINE__
#define DBG LOGFILE,4,__LINE__

#define DEF "./plover_client.log",1,__LINE__

static char	*find_text;
static gchar *dpath;				// default 경로
static gchar *path;					// 검사 파일경로
static gchar *name;					// 등록 유저이름
static gchar *job;					// 등록 직급이름
static gchar *vs_dept;				// 등록 부서이름
static gdouble percent = 0.0;		// Progress Bar
static char message[1024] = {0,};	// Progress Bar Message
static char *buffer = NULL;		// 검출돌고있는 파일 버퍼
static int	chk_fcnt = -1;		// 검출파일 총 개수 0부터 1개
static char	chk_fname[100];		// 정규식돌고있는 파일이름
static char	chk_uuid[40];			// INI UUID 확인
static char	set_uuid[40];			// UUID 저장
static int	chk_df = 0;			// chk data flag
static const char	*chk_ver;		// chk version
static gchar	*fname;
static GtkTreeViewColumn	*dcol;
static GtkCellRenderer		*drenderer;


/*---------------------------------------------------------------------------*/
/* Related to Gtk and Glade                                  */
/*---------------------------------------------------------------------------*/
GtkWidget				*main_window,
						*enrollment_window,
						*detect_window,
						*department_window,
						*setting_window,
						*window;

GtkScrolledWindow	*d_scrolledwindow,
						*dept_scrolledwindow;
						
GtkEntry				*e_name_entry,
						*e_jobtitle_entry,
						*e_department_entry,
						*d_detect_entry,
						*s_detect_entry,
						*s_ip_entry,
						*s_port_entry;

GtkWidget 			*m_userinfo_label,
						*m_verion_label,
						*d_progressbar,
						*d_progressbar_status,
						*d_view,
						*e_jobtitle_cbxtext,
						*e_verion_label,
						*filechooserdialog;

GtkTreeStore			*dtreestore;

GtkTreeIter			 diter;

GtkBuilder			*builder;
/* end of relating to Gtk and Glade */


/*---------------------------------------------------------------------------*/
/* Create IniFile                                            */
/*---------------------------------------------------------------------------*/
void func_CreateIni(void)
{
	FILE *ini ;

	if ((ini = fopen ("/home/joeun/bxr_plover/client/lib/plover.ini", "w")) == NULL)
	{
		BXLog (DBG, "iniparser: Can not create plover.ini\n");

		return ;
	}

	fprintf (ini,
	"[USERINFO]\n"
	"\n"
	"UUID	=	%s;\n"
	"\n", uDs.uuid);

	fclose (ini);
}
/* end of func_MakeIni(); */


/*---------------------------------------------------------------------------*/
/* Parse IniFile                                             */
/*---------------------------------------------------------------------------*/
int  func_ParseIni (char * ini_name)
{
	dictionary *ini;

	/* Some temporary variables to hold query results */

	ini = iniparser_load (ini_name);
	if (ini == NULL)
	{
		BXLog (DBG, "Can not parse file: %s\n", ini_name);
		return -1 ;
	}
	iniparser_dump (ini, stderr);

	/* Get attributes */
	INI_UUID = iniparser_getstring (ini, "USERINFO:UUID", NULL);
	strcpy (chk_uuid ,INI_UUID);
	BXLog (DBG, "********** INI FILE **********.\n");
	BXLog (DBG, "INI UUID:	[%s]\n", INI_UUID ? INI_UUID : "UNDEF");

	//i = iniparser_getint (ini, "a:b", -1);
	//printf ("a:	[%d]\n", i);

	iniparser_freedict (ini);

	return 0 ;
}
/* end of func_ParseIni(); */


/*---------------------------------------------------------------------------*/
/* Version Check                                             */
/*---------------------------------------------------------------------------*/
int func_VerChk()
{
	func_Send();
	//gtk_label_set_text (GTK_LABEL (e_verion_label), chk_ver); // fuc_send()의 flag(0)에서 chk_ver에 버전data넣어야함
	chk_df = 1; // 사용자 확인해야함

	return chk_df;
}
/* end of func_VerChk(); */


/*---------------------------------------------------------------------------*/
/* User Check     1=없다 2=있다                                         */
/*---------------------------------------------------------------------------*/
int func_UsrChk()
{
	int tmp;

	chk_df = 1;
	func_Send();
	tmp = strcmp (chk_uuid, uDs.uuid); // 클라이언트 흐름 다시 생각해보기. 실행부터

	if (tmp == 0)
	{
		BXLog (DBG, "사용자님 안녕하세요!\n");
		chk_df = 2;
	}
	else
	{
		BXLog (DBG, "사용자 등록을 해주세요!\n");
		chk_df = 1;
	}
	 

	if (chk_df == 1)
	{
		gtk_widget_show (enrollment_window);	// 사용자 등록 창
		gtk_main();
	}
	if (chk_df == 2)
	{
		gtk_widget_show (main_window);			// 메인 창
		gtk_main();
	}
	
	return chk_df;
}
/* end of func_UsrChk(); */


/*---------------------------------------------------------------------------*/
/* User UUID Parsing                                         */
/*---------------------------------------------------------------------------*/
int func_Uuid()
{
	FILE *uidfp	= NULL;
	char *pstr	= NULL;
	char  strbuf[300];

	uidfp = fopen ("/etc/fstab", "r");

	if (NULL == uidfp)
	{
        BXLog (DBG, "Can not open fstab file...\n");
		return 1;
	}

	while (feof (uidfp) == 0)
	{
		pstr = fgets (strbuf, sizeof(strbuf), uidfp);

		if (pstr != 0) // \n만나면 문자 더이상 안 읽어서 안해주면 seg fault 뜸
		{
			if (pstr[0] == 'U' && pstr[42] == '/')
			{
				for (int i = 0; i < 36; i++)
				{ 
					set_uuid[i] = pstr[i+5];
				}
				strcpy (uDs.uuid, set_uuid);
				strcpy (sfDs.uuid, set_uuid);
				BXLog (DBG, "UUID: [%s]\n", uDs.uuid);
			}
		}
    }
    
    memset (strbuf, 0, sizeof(strbuf));
	fclose (uidfp);

	return 0;
}
/* end of func_Uuid(); */


/*---------------------------------------------------------------------------*/
/* Regex Compile                                             */
/*---------------------------------------------------------------------------*/
int compile_regex (regex_t *r, const char *regex_text)
{
	int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);

	if (status != 0)
	{
		char error_message[MAX_ERROR_MSG];

		regerror (status, r, error_message, MAX_ERROR_MSG);

		BXLog (DBG, "Regex error compiling '%s': %s\n", regex_text, error_message);

		return 1;
	}

	return 0;
}
/* end of compile_regex(); */


/*---------------------------------------------------------------------------*/
/* Jumin and Foreign Check                                   */
/*---------------------------------------------------------------------------*/
char match_regex_jnfg (regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	/* "P" is a pointer into the string which points to the end of the
	previous match. */
	const char *p = to_match;
	const int n_matches = 1000;		// "N_matches" is the maximum number of matches allowed. //
	/* "M" contains the matches found. */
	regmatch_t m[n_matches];

	// 버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교 //
	while (1)
	{
		int nomatch = regexec (r, p, n_matches, m, 0);

		if (nomatch != 1)
		{
			for (int i = 0; i < n_matches; i++)
			{
				int start;

				if (m[i].rm_so == -1)
				{
					break;
				}

				start = m[i].rm_so + (p - to_match);

				// 주민번호, 외국인등록번호 정규식 검사 통과
				if (i == 0)
				{
					int chk = 0, jtmp = 0, fgtmp = 0, sum = 0;
					char buf_tmp[15];

					// 주민번호, 외국인등록번호 유효성 검사
					for (int j = 0; j < 14; j++)
					{
						buf_tmp[j] = *(to_match + start + j);
						buf_tmp[j] -= 48;
					}

					sum = buf_tmp[0]*2 + buf_tmp[1]*3 + buf_tmp[2]*4 + buf_tmp[3]*5 + buf_tmp[4]*6 + buf_tmp[5]*7
					+ buf_tmp[7]*8 + buf_tmp[8]*9 + buf_tmp[9]*2 + buf_tmp[10]*3 + buf_tmp[11]*4 + buf_tmp[12]*5;

					chk = buf_tmp[13];
					jtmp = 11 - (sum % 11);	// 주민번호
					fgtmp = 13 - (sum % 11);	// 외국인번호

					if (jtmp >= 10)
					{
						jtmp -= 10;
					}

					if (fgtmp >= 10)
					{
						fgtmp -= 10;
					}

					// 주민번호 유효성 통과
					if (jtmp == chk)
					{
						int res = strcmp (chk_fname, file->d_name); // 같은파일 = 0

						if (res != 0)
						{
							chk_fcnt++;
							percent += 0.1;
						}

						// 읽고있는중인 파일 이름 저장
						strcpy (chk_fname, file->d_name);

						// 검출된 주민등록번호의 수
						fDs[chk_fcnt].jcnt++;

						// data 구조체에 저장
						strcpy (fDs[chk_fcnt].fpath, filepath);
						strcpy (fDs[chk_fcnt].fname, file->d_name);
						fDs [chk_fcnt].fsize = buf.st_size;
						strcpy (fDs[chk_fcnt].stat, "일반");

					}

					// 외국인등록번호 유효성 통과
					if (fgtmp == chk)
					{
						int res = strcmp (chk_fname, file->d_name); // 같은파일 = 0

						if (res != 0)
						{
							chk_fcnt++;
							percent += 0.1;
						}

					// 읽고있는중인 파일 이름 저장
					strcpy (chk_fname, file->d_name);

					// 검출된 외국인등록번호의 수
					fDs[chk_fcnt].fgcnt++;

					// data 구조체에 저장
					strcpy (fDs[chk_fcnt].fpath, filepath);
					strcpy (fDs[chk_fcnt].fname, file->d_name);
					fDs[chk_fcnt].fsize = buf.st_size;
					strcpy (fDs[chk_fcnt].stat, "일반");

					}
				}
			}
		}

		else
		{
			//BXLog (DBG, "No more matches. %d\n", nomatch);

			return 0;
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_jnfg(); */


/*---------------------------------------------------------------------------*/
/* Driver Check                                              */
/*---------------------------------------------------------------------------*/
char match_regex_d (regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	const char *p = to_match;
	const int n_matches = 1000;		// "N_matches" is the maximum number of matches allowed.
	regmatch_t m[n_matches];

	// 버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교
	while (1)
	{
		int nomatch = regexec(r, p, n_matches, m, 0);

		if (nomatch != 1)
		{
			for (int i = 0; i < n_matches; i++)
			{
				if (m[i].rm_so == -1)
				{
				    break;
				}

				// 운전면허 정규식 검사 통과
				if (i == 0)
				{
					int res = strcmp (chk_fname, file->d_name); // 같은파일 = 0

					if (res != 0)
					{
						chk_fcnt++;
						percent += 0.1;

					}

					// 읽고있는중인 파일 이름 저장
					strcpy (chk_fname, file->d_name);

					// 검출된 운전면허의 수
					fDs[chk_fcnt].dcnt++;

					// data 구조체에 저장
					strcpy (fDs[chk_fcnt].fpath, filepath);
					strcpy (fDs[chk_fcnt].fname, file->d_name);
					fDs[chk_fcnt].fsize = buf.st_size;
					strcpy (fDs[chk_fcnt].stat, "일반");

				}
			}
		}
		else
		{
			//BXLog (DBG, "No more matches.\n");

			return 0;
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_d(); */


/*---------------------------------------------------------------------------*/
/* Passport Check                                            */
/*---------------------------------------------------------------------------*/
char match_regex_p (regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	const char *p = to_match;
	const int n_matches = 1000;		// "N_matches" is the maximum number of matches allowed.
	regmatch_t m[n_matches];

	// 버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교
	while (1)
	{
		int nomatch = regexec (r, p, n_matches, m, 0);

		if (nomatch != 1)
		{
			for (int i = 0; i < n_matches; i++)
			{
				if (m[i].rm_so == -1)
				{
					break;
				}

				// 운전면허 정규식 검사 통과
				if (i == 0)
				{
					int res = strcmp (chk_fname, file->d_name); // 같은파일 = 0

					if (res != 0)
					{
						chk_fcnt++;
						percent += 0.1;
					}

					// 읽고있는중인 파일 이름 저장
					strcpy (chk_fname, file->d_name);

					// 검출된 운전면허의 수
					fDs[chk_fcnt].pcnt++;

					// data 구조체에 저장
					strcpy (fDs[chk_fcnt].fpath, filepath);
					strcpy (fDs[chk_fcnt].fname, file->d_name);
					fDs[chk_fcnt].fsize = buf.st_size;
					strcpy (fDs[chk_fcnt].stat, "일반");

				}
			}
		}
		else
		{
			//BXLog (DBG, "No more matches.\n");

			return 0;
		}

		p += m[0].rm_eo;
	}
}
/* end of match_regex_p(); */


/*---------------------------------------------------------------------------*/
/* Regex Check kind of data                                  */
/*---------------------------------------------------------------------------*/
void check_kind_of_data (const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	regex_t r;
	const char *regex_text;

	/*
	switch(data_flag) //나중에 민감정보 종류 선택 검출 할 때 사용
	{
		case 1:
			regex_text = "([0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-4][0-9]{6})"; //주민번호 정규식
			compile_regex(&r, regex_text); //정규식 컴파일
			match_regex_j(&r, to_match, filepath, file, buf);
			break;
		case 2:
			regex_text = "[0-9]{2}-[0-9]{6}-[0-9]{2}"; //운전면허 정규식
			compile_regex(&r, regex_text); //정규식 컴파일
			//match_regex_d(&r, to_match, filepath, file, buf);
	}
	*/

	// 주민번호, 외국인등록번호 정규식 //
	regex_text = "[0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-4][0-9]{6}";
	compile_regex(&r, regex_text); // 정규식 컴파일 //
	match_regex_jnfg(&r, to_match, filepath, file, buf);

	// 운전면허 정규식 //
	regex_text = "[0-9]{2}-[0-9]{6}-[0-9]{2}";
	compile_regex (&r, regex_text); // 정규식 컴파일 //
	match_regex_d (&r, to_match, filepath, file, buf);

	// 여권번호 정규식 //
	regex_text = "[a-zA-Z]{1}[0-9]{8}";
	compile_regex (&r, regex_text); // 정규식 컴파일 //
	match_regex_p (&r, to_match, filepath, file, buf);

	return;
}
/* end of check_kind_of_data(); */


/*---------------------------------------------------------------------------*/
/* Pdf to txt                                                */
/*---------------------------------------------------------------------------*/
int func_pdf2txt(char *filepath)
{
	FILE *fp;
	char commande[60] = "pdftotext -enc UTF-8 ";
    char namebuf[50];
    long length = 0;

	strcat(commande,filepath);
	system(commande);

	strcpy (namebuf, filepath);
	namebuf[strlen(namebuf) - 4] = '\0';
	strcat (namebuf, ".txt");

	fp = fopen (namebuf, "r");
	fseek (fp, 0, SEEK_END);
	length = ftell (fp);
	rewind (fp);

	fread (find_text, 1, length, fp);

	fclose (fp);
	remove (namebuf);

	return 0;
}
/* end of pdf to txt 변환 */


/*---------------------------------------------------------------------------*/
/* Scanning Folder and File                                      */
/*---------------------------------------------------------------------------*/
int func_Detect (gchar *path)
{
	DIR *dp = NULL;
	FILE *fp, *cp = NULL;
	struct dirent *file = NULL;
	struct stat buf;
	char filepath[300];
	long lSize;
	char commande[60] = {0,}, ftype[60] = {0,};
	int ftypesig;

	memset (message, 0x00, strlen(message));
	if ((dp = opendir(path)) == NULL)
	{
		BXLog (DBG, "[%s] Can not open folder...\n", file->d_name);;
		return -1;
	}
	while ((file = readdir(dp)) != NULL)
	{
		// filepath에 현재 path넣기
		sprintf (filepath, "%s/%s", path, file->d_name);
		lstat (filepath, &buf);
		// 폴더 //
		if (S_ISDIR (buf.st_mode))
		{
			// .이거하고 ..이거 제외
			if ((!strcmp (file->d_name, ".")) || (!strcmp (file->d_name, "..")))
			{
				continue;
			}
			// 안에 폴더로 재귀함수
			func_Detect(filepath);
		}
		// 파일
		else if (S_ISREG (buf.st_mode))
		{
			BXLog (DBG, "[%s] Start File Detecting...\n", file->d_name);

			fp = fopen (filepath, "r");

			BXLog (DBG, "[%s] Open FILE\n", file->d_name);
			if (NULL == fp)
			{
				BXLog (DBG, "[%s] Can not open file...\n", file->d_name);
				return 1;
			}
			strcpy (commande, "file -b ");
			strcat (commande, filepath);
			cp = popen (commande, "r");

			if(!cp)
			{
				printf("error [%d:%s]\n", errno, strerror(errno));
				return -1;
			}
			fread ((void *)ftype, sizeof(char), 60, cp); // 파일 종류 저장
			//printf("ftype: %s\n", ftype);


			if (ftype[0] == 'P')
			{
				ftypesig = 1;
			}

			// 파일 크기만큼 버퍼에 저장 후 find_text에 넣어서 정규식검사로 이동
			fseek (fp, 0, SEEK_END);
			lSize = ftell (fp);
			rewind (fp);

			buffer = (char *) malloc (sizeof(char) *lSize);

			fread(buffer, 1, lSize, fp);

			find_text = buffer; //그냥 버퍼는 못쓰나 확인

			switch(ftypesig)
			{
				case 0:
					break;	

				case 1:
					func_pdf2txt(filepath);
					break;
			}

			check_kind_of_data (find_text, filepath, file, buf);

			// 메모리관리(초기화), 파일
			fclose (fp);
			free(buffer);
			BXLog (DBG, "[%s] Close FILE\n", file->d_name);
			BXLog (DBG, "[%s] End File Detecting...\n", file->d_name);
			
			func_Refresh_ScrollWindow();

			memset( message, 0x00, strlen(message));
			sprintf( message, "%.0f%% Complete", percent * 100.0);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), percent);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);

			while (gtk_events_pending ()) 
					gtk_main_iteration (); 

			chk_fname[0] = 0; // 초기화
		}
	}

	closedir (dp);
	BXLog (DBG, "Close DIR\n");

	return  0;
}
/* end of func_Detect(); */


/*---------------------------------------------------------------------------*/
/* Oppening RabbitMQ Socket, Channel                         */
/*---------------------------------------------------------------------------*/
// RabbitMQ 소켓, 채널 열기
int func_SetRabbit()
{
	port		=	PORT;
	vhost		=	VHOST;
	username	=	USERNAME;
	password	=	PASSWORD;

	/*
	 establish a channel that is used to connect RabbitMQ server
	*/
	conn = amqp_new_connection();

	socket = amqp_tcp_socket_new (conn);
	if (!socket)
	{
		die ("creating TCP socket");
	}
	status = amqp_socket_open (socket, HOSTNAME, port);
	
	if (status)
	{
		die("opening TCP socket");
	}

	die_on_amqp_error (amqp_login (conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
																		"guest", "guest"),
																		"Logging in");
	/*die_on_amqp_error(amqp_login(conn, vhost, 200, 131072, 0, AMQP_SASL_METHOD_PLAIN,
																		username, password),
																		"Logging in");*/
					
	amqp_channel_open (conn, 1);
	die_on_amqp_error (amqp_get_rpc_reply (conn), "Opening channel");

	/*
	 create private reply_to queue
	*/
	{
		amqp_queue_declare_ok_t *r = amqp_queue_declare (
			conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);

		die_on_amqp_error (amqp_get_rpc_reply (conn), "Declaring queue");

		reply_to_queue = amqp_bytes_malloc_dup (r->queue);
		if (reply_to_queue.bytes == NULL)
		{
			BXLog (DBG, "Out of memory while copying queue name");
			return 1;
		}
	}

	return 0;
}


/*---------------------------------------------------------------------------*/
/* Sendding RabbitMQ Message                                 */
/*---------------------------------------------------------------------------*/
int func_Send()
{
	static char *enc;
	size_t in_len = 0;
	//routingkey = "ka"; // TRCODE

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
			BXLog (DBG, "Out of memory while copying queue name");

			return 1;
		}
		props.correlation_id = amqp_cstring_bytes ("1");

		/*
		  publish // 어떤데이터를 보낼지 chk_df로 구분 #data_sending
		*/
		switch (chk_df)
		{
			case 0:	// 버전 확인 //
				printf ("##### 버전 확인 #####\n");
				//routingkey = "BPVCHK0R"; // TRCODE
				routingkey = "ka";
				enc = "Version Check";
				die_on_error (amqp_basic_publish (conn, 1, amqp_cstring_bytes (EXCHANGE),
									amqp_cstring_bytes (routingkey), 0, 0,
									&props, amqp_cstring_bytes (enc)), "Publishing");
				break;

			case 1:	// 사용자 확인 //
				printf ("##### 사용자 확인 #####\n");
				routingkey = "ka"; // TRCODE
				enc = "User Check";
				die_on_error (amqp_basic_publish (conn, 1, amqp_cstring_bytes (EXCHANGE),
									amqp_cstring_bytes (routingkey), 0, 0,
									&props, amqp_cstring_bytes (enc)), "Publishing");
				break;
			
			case 2:	// 사용자 등록 //
				BXLog (DBG, "##### 사용자 등록 #####\n");
				//routingkey = "BPDEPT0R"; // TRCODE
				routingkey = "ka";
				in_len = sizeof(uDs);
				enc = b64_encode ((unsigned char *)&uDs, in_len, enc);
				//printf ("[enc_data: %s]\n", enc);
				//printf ("[UUID: %s, %s/%s/%s]\n\n", uDs.uuid, uDs.udept, uDs.uname, uDs.ujob);
				
				die_on_error (amqp_basic_publish (conn, 1, amqp_cstring_bytes (EXCHANGE),
									amqp_cstring_bytes (routingkey), 0, 0,
									&props, amqp_cstring_bytes (enc)), "Publishing");
				break;

			case 3:	// 검출 결과 //
				BXLog (DBG, "##### 검출 결과  #####\n");
				routingkey = "ka"; // TRCODE

				for (int i = 0; i <= chk_fcnt; i++)
				{
					strcpy (fDs[i].uuid, uDs.uuid);
					in_len = sizeof(fDs[i]);
					//printf("fds[%d]: %ld\n", i ,in_len); //구조체 크기확인
					enc = b64_encode ((unsigned char *)&fDs[i], in_len, enc);
					//printf ("[enc_data: %s]\n", enc);
					//printf ("[UUID: %s, cnt: %d, jumin: %d, driver: %d, forign: %d, pass: %d, fsize: %d, fstat: %s, fpath: %s]\n\n",
								//fDs[i].uuid, i, fDs[i].jcnt, fDs[i].dcnt, fDs[i].fgcnt, fDs[i].pcnt, fDs[i].fsize, fDs[i].stat, fDs[i].fpath);

					die_on_error (amqp_basic_publish (conn, 1, amqp_cstring_bytes (EXCHANGE),
									amqp_cstring_bytes (routingkey), 0, 0,
									&props, amqp_cstring_bytes (enc)), "Publishing");
				}
				break;

			case 4:	// 파일 삭제 //
				BXLog (DBG, "##### 파일 삭제  #####\n");
				routingkey = "ka"; // TRCODE
				in_len = sizeof(sfDs);
				enc = b64_encode ((unsigned char *)&sfDs, in_len, enc);
				//printf ("[enc_data: %s]\n", enc);
				
				die_on_error (amqp_basic_publish (conn, 1, amqp_cstring_bytes (EXCHANGE),
									amqp_cstring_bytes (routingkey), 0, 0,
									&props, amqp_cstring_bytes (enc)), "Publishing");
				break;
				
			case 5:	// 파일 암호화 //
				BXLog (DBG, "##### 파일 암호화  #####\n");
				routingkey = "ka"; // TRCODE //
				in_len = sizeof(sfDs);
				enc = b64_encode ((unsigned char *)&sfDs, in_len, enc);
				//printf ("[enc_data: %s]\n", enc);

				die_on_error (amqp_basic_publish (conn, 1, amqp_cstring_bytes (EXCHANGE),
									amqp_cstring_bytes (routingkey), 0, 0,
									&props, amqp_cstring_bytes (enc)), "Publishing");
				break;

		}

		amqp_bytes_free (props.reply_to);
	}
	
	
	/*
	wait an answer //응답도 case이용해서 flag별로 구분해야 할듯
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
															
	//die_on_error(amqp_destroy_connection(conn), "Ending connection");

	return TRUE;
}
/* end of func_Send(); */


/*---------------------------------------------------------------------------*/
/* gtk_dialog_modal                                          */
/*---------------------------------------------------------------------------*/
int func_gtk_dialog_modal (int type, GtkWidget *widget, char *message)
{
	GtkWidget *dialog, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_MODAL;
	int	rtn = GTK_RESPONSE_REJECT;

	switch (type)
	{
		case 0 :
			dialog = gtk_dialog_new_with_buttons ("plover", GTK_WINDOW (widget), flags, 
						("_OK"), GTK_RESPONSE_ACCEPT, NULL );
			break;

		case 1 :
			dialog = gtk_dialog_new_with_buttons ("plover", GTK_WINDOW (widget), flags,
						("_OK"), GTK_RESPONSE_ACCEPT, 
						("_Cancel"), GTK_RESPONSE_REJECT, NULL);
			break;
			
		case 2 :
			dialog = gtk_dialog_new_with_buttons ("plover", GTK_WINDOW (widget), flags, 
						("_OK"), GTK_RESPONSE_ACCEPT, NULL );

		default :
			break;
	}

	label = gtk_label_new (message);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_widget_show_all (dialog);

	rtn = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	return (rtn);	
}
/* end of fgtk_dialog_modal(); */


/*---------------------------------------------------------------------------*/
/* Delete File                                               */
/*---------------------------------------------------------------------------*/
int func_file_eraser (int type)
{
	FILE *fp;
	int mode = R_OK | W_OK;
	char MsgTmp[5] = {0,};
	gdouble size = 0.0;
	char *msize;
	
	percent = 0.0;
	if (access (sfDs.fpath, mode) != 0 )
	{
		func_gtk_dialog_modal (0, window, "\n    파일이 삭제 가능한 상태가 아닙니다.    \n");
	}
	else
		{
			msize = malloc (ERASER_SIZE);
			fp = fopen (sfDs.fpath, "w");

			while (gtk_events_pending ()) 
					gtk_main_iteration (); 

			memset (message, 0x00, strlen (message));
			sprintf (message, "%.0f%% Complete", percent);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), percent / 100.0);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR (d_progressbar), message);

			for( int i = 0 ; i < type ; i++ )
			{
				size = 0.0;

				while (size < sfDs.fsize)
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

					while (gtk_events_pending ()) 
							gtk_main_iteration (); 

					fwrite( msize, 1, ((sfDs.fsize/size))>0?ERASER_SIZE:(sfDs.fsize%ERASER_SIZE), fp);
					size += ERASER_SIZE;
					percent = (size+(sfDs.fsize*i))/(sfDs.fsize*type)*100.0;
					if( (int)size % PROGRESS_SIZE == 0 )
					{
						memset( message, 0x00, strlen(message));
						sprintf( message, "%.0f%% Complete", percent);
					//				gchar *message = g_strdup_printf ("%.0f%% Complete", percent);
						gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), percent / 100.0);
						gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);
					}
				}

				fseek( fp, 0L, SEEK_SET );
			}

			sprintf( message, "%.0f%% Complete", 100.0);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), 100.0);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);
	
			fclose(fp);
			free(msize);
		}

	remove (sfDs.fpath);
	func_gtk_dialog_modal (0, window, "\n    삭제가 완료되었습니다.    \n");

	chk_df = 4;

	return (TRUE);
}
// end of func_file_eraser(); //


/*---------------------------------------------------------------------------*/
/* ARIA Encryption                                           */
/*---------------------------------------------------------------------------*/
void func_ARIA ()
{
	FILE *fp;
	int mok = 0, nam = 0, asize = 16;
	uint cur = 0, sum = 0;
	gdouble size = 0;
	long lSize = 0;
	char message[1134] = {0,};
	unsigned char *buff = NULL, *temp = NULL;
	static unsigned char *aribuf = NULL;
	percent = 0.0;
	

	if (sfDs.fpath[0] == 0x00)
	{
		func_gtk_dialog_modal (0, window, "\n    대상파일이 선택되지 않았습니다.    \n");
	}
	else
	{
		sprintf (message, 
			"\n 아래 파일을 암호화 하시겠습니까?\n    [ %s ]    \n", sfDs.fpath);
    
		if (func_gtk_dialog_modal(1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			int res = 0;

			BXLog (DBG, "[%s] Start File Encrypt...\n", sfDs.fname);
			fp = fopen (sfDs.fpath, "r");
			if (NULL == fp)
			{
				BXLog (DBG, "[%s] Can not open file...\n", sfDs.fname);
				return;
			}

			fseek (fp, 0, SEEK_END);
			lSize = ftell (fp);
			rewind (fp);

			buff = (unsigned char *) malloc (sizeof(char) *lSize);
			temp = buff;

			while ((cur = fread (&buff[sum], sizeof(char), lSize - cur, fp)) > 0 )
			{
				sum += cur;
			}

			if (sum != lSize)
			{
				BXLog (DBG, "[%s] Can not open file...\n", sfDs.fname);
			}

			mok = sfDs.fsize / 16;
			nam = sfDs.fsize % 16;
			aribuf = (unsigned char *) malloc (sizeof(char) *asize);

			for (int i = 0; i <= mok; i++)
			{
				size += 16;
				percent = size / sfDs.fsize * 100.0;
				if ((int)size % PROGRESS_SIZE == 0 )
				{
					while (gtk_events_pending ()) 
						gtk_main_iteration ();

					memset( message, 0x00, strlen(message));
					sprintf( message, "%.0f%% Complete", percent);
					gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (d_progressbar), percent / 100.0);
					gtk_progress_bar_set_text (GTK_PROGRESS_BAR (d_progressbar), message);
				}

				if ( i != mok)
				{
					memcpy (aribuf, buff, asize);

					ARIA (aribuf);
					memset (buff, 0, asize);
					memcpy (buff, aribuf, asize);
					memset (aribuf, 0, asize);
					buff += 16;
				}
				else
				{
					memcpy (aribuf, buff, nam);
					ARIA (aribuf);
					memset (buff, 0, nam);
					memcpy (buff, aribuf, nam);
					memset (aribuf, 0, nam);
				}
			}

			fclose (fp);
			remove(sfDs.fpath);

			fp = fopen (sfDs.fpath, "w+");
			buff = temp;
			fwrite (buff, lSize, 1, fp);
			BXLog (DBG, "[%s] End File Encrypt...\n", sfDs.fname);
			func_gtk_dialog_modal (0, window, "\n    암호화가 완료되었습니다.    \n");

			for (int i = 0; i <= chk_fcnt; i++)
			{
				res = strcmp (fname, fDs[i].fname);

				if (res == 0)
				{
					strcpy (fDs[i].stat, "암호화");
					BXLog (DBG, "결과: [%d]번째 파일[%s]가 [%s] 되었습니다.\n", i, fDs[i].fname, fDs[i].stat);
				}
			}

			func_Refresh_ScrollWindow();
			//printf ("[UUID: %s], [파일이름: %s], [파일크기: %d], [파일상태: %s], [파일경로: %s]\n", sfDs.uuid, sfDs.fname, sfDs.fsize, sfDs.stat, sfDs.fpath);

			
			sprintf( message, "%.0f%% Complete", 100.0);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), 100.0);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);

			strcpy (sfDs.stat, "암호화");
			free (buff);
			fclose (fp);
			chk_df = 5;
			BXLog (DBG, "[%s] Close FILE\n", sfDs.fname);
			chk_fname[0] = 0; // 초기화 //
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    취소 되었습니다.    \n");
		}
	}

	return;
}
// end of func_ARIA (); //


/*---------------------------------------------------------------------------*/
/* BXLog start                                               */
/*---------------------------------------------------------------------------*/
int BXLog (const char *logfile, int logflag, int logline, const char *fmt, ...)
{
	int fd, len;
	struct	timeval	t;
	struct tm *tm;
	static char fname[128];
	static char sTmp[1024 * 2], sFlg[5];

	va_list ap;

	//if( LOGMODE < logflag ) return 0;

	switch (logflag)
	{
		case	0 :
		case	1 :
			sprintf (sFlg, "E");
			break;

		case	2 :
			sprintf (sFlg, "I");
			break;

		case	3 :
			sprintf (sFlg, "W");
			break;

		case	4 :
		default   :
#ifndef _BXDBG
			return 0;
#endif
			sprintf( sFlg, "D" );

			break;
	}

	memset (sTmp, 0x00, sizeof (sTmp));
	gettimeofday (&t, NULL);
	tm = localtime (&t.tv_sec);

	/* [HHMMSS ssssss flag __LINE__] */
	len = sprintf (sTmp, "[%5d:%08x/%02d%02d%02d %06ld/%s:%4d:%4d]",
			getpid(), (unsigned int) pthread_self(),
			tm->tm_hour, tm->tm_min, tm->tm_sec, t.tv_usec, 
			sFlg, errno, logline );

	va_start (ap, fmt);
	vsprintf ((char *) &sTmp[len], fmt, ap);
	va_end (ap);

	sprintf (fname, "%s.%02d%02d", logfile, tm->tm_mon+1, tm->tm_mday);
	if (access (fname, 0) != 0)
	{
		fd = open (fname, O_WRONLY|O_CREAT, 0660);
	}
	else
	{
		fd = open (fname, O_WRONLY|O_APPEND, 0660);
	}

	if (fd >= 0)
	{
		write(fd, sTmp, strlen(sTmp));
		close(fd);
	}

	return 0;

}/* End of BXLog() */


/*---------------------------------------------------------------------------*/
/* Refresh ScrollWindow                                      */
/*---------------------------------------------------------------------------*/
void func_Refresh_ScrollWindow()
{
	gtk_container_remove (GTK_CONTAINER (d_scrolledwindow), d_view);	// 다 지우기
	d_view = d_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (d_scrolledwindow), d_view);
	gtk_widget_show_all ((GtkWidget *) d_scrolledwindow);

	return;
}
/* func_Refresh_ScrollWindow() */


/*---------------------------------------------------------------------------*/
/* main_window function                                      */
/*---------------------------------------------------------------------------*/
void m_detect_btn_clicked (GtkButton *m_detect_btn, gpointer *data)
{
	func_Refresh_ScrollWindow();
	gtk_widget_show (detect_window);

	return;
}

void m_setting_btn_clicked (GtkButton *m_setting_btn, gpointer *data)
{
	gtk_widget_show (setting_window);

	return;
}

void m_window_destroy()
{
	gtk_main_quit();

	return;
}
/* end of main_window function */


/*---------------------------------------------------------------------------*/
/* detect_window function                                    */
/*---------------------------------------------------------------------------*/
void d_detect_entry_activate (GtkEntry *d_detect_entry, gpointer *data)
{
	path = (gchar *)gtk_entry_get_text (d_detect_entry);
	//g_print ("선택한 폴더 위치: %s\n", path);

	return;
}


void d_folder_btn_clicked (GtkButton *d_folder_btn, gpointer *data)
{
    filechooserdialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW (data), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
			("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all (filechooserdialog);
    
	gint resp = gtk_dialog_run (GTK_DIALOG (filechooserdialog));

    if (resp == GTK_RESPONSE_ACCEPT)
    {
		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
    } 

	gtk_entry_set_text (GTK_ENTRY (data), path);


	gtk_widget_destroy (filechooserdialog);

	//g_print ("선택한 폴더 위치: %s\n", path);

	return;
}

	/*---------------------------------------------------------------------------*/
	/* treeview function                                         */
	/*---------------------------------------------------------------------------*/
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
d_view_selection_func 	(GtkTreeSelection	*selection,
							GtkTreeModel			*model,
							GtkTreePath			*path,
							gboolean				 path_currently_selected,
							gpointer				 userdata)
{
	GtkTreeIter iter;
	gchar *stat, *fpath;
	uint fsize;
	
	if (gtk_tree_model_get_iter (model, &iter, path))
	{
		if (!path_currently_selected)
		{
			// set select data
			gtk_tree_model_get (model, &iter, d_treeview_filename, 	&fname,	-1);
			gtk_tree_model_get (model, &iter, d_treeview_size,		&fsize,	-1);
			gtk_tree_model_get (model, &iter, d_treeview_stat,		&stat,		-1);
			gtk_tree_model_get (model, &iter, d_treeview_fileloca,	&fpath,	-1);

			// input data in structure
			strcpy (sfDs.fname, fname);
			sfDs.fsize = fsize;
			strcpy (sfDs.stat, stat);
			strcpy (sfDs.fpath, fpath);

			//g_print ("파일위치: [%s], 파일크기: [%d] 선택.\n", sfDs.fpath, sfDs.fsize);

		}
		else
		{
			//g_print ("파일위치: [%s] 선택 해제.\n", sfDs.fpath);
		}
	}

	return TRUE; /* allow selection state to change */
}


static GtkTreeModel *
d_create_and_fill_model (void)
{
	dtreestore = gtk_tree_store_new (NUM_COLS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
					G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);

	for (int i = 0; i <= chk_fcnt; i++)
	{
		gtk_tree_store_append (dtreestore, &diter, NULL);
		gtk_tree_store_set (dtreestore, &diter,
					  d_treeview_num, i + 1,
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

	return GTK_TREE_MODEL (dtreestore);
}

static GtkWidget *
d_create_view_and_model (void)
{
	GtkTreeViewColumn	*col;
	GtkCellRenderer		*renderer;
	GtkWidget		*d_view;
	GtkTreeModel		*model;
	GtkTreeSelection	*selection;
	
	d_view = gtk_tree_view_new();

	// Column #컬럼명 //
	col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title (col, "번호");

	// pack tree view column into tree view //
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);

	renderer = gtk_cell_renderer_text_new();

	// pack cell renderer into tree view column //
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_num);

	// --- Column #파일 이름 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일 이름");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_filename);

	// --- Column #주민번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "주민번호");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_jcnt);

	// --- Column #운전면허 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "운전면허");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_dcnt);
	
	// --- Column #외국인등록번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "외국인등록");
	gtk_tree_view_append_column (GTK_TREE_VIEW(d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_fgcnt);
	
	// --- Column #여권번호 개수 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "여권번호");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_pcnt);

	// --- Column #상태 --- //
	dcol = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (dcol, "상태");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), dcol);
	drenderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (dcol, drenderer, TRUE);
	gtk_tree_view_column_add_attribute (dcol, drenderer, "text", d_treeview_stat);
	
	// --- Column #파일 크기 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일 크기");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_size);
	
	// --- Column #파일 위치 --- //
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title (col, "파일 위치");
	gtk_tree_view_append_column (GTK_TREE_VIEW (d_view), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_add_attribute (col, renderer, "text", d_treeview_fileloca);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (d_view));
	
	gtk_tree_selection_set_select_function (selection, d_view_selection_func, NULL, NULL);



	model = d_create_and_fill_model();

	gtk_tree_view_set_model (GTK_TREE_VIEW (d_view), model);

	g_object_unref (model); // destroy model automatically with view //

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection (GTK_TREE_VIEW (d_view)),
							  GTK_SELECTION_SINGLE);

	return d_view;
}
	/* end of treeview function */


void d_detect_btn_clicked (GtkButton *d_detect_btn, gpointer *data)
{
	double chktime = 0;
	chk_df = 3;
	chk_fcnt = -1; 						// 파일개수 count 초기화
	memset (&fDs, 0, sizeof(fDs));		// 구조체 초기화
	percent = 0.0;

	memset( message, 0x00, strlen(message));
	sprintf( message, "%.0f%% Complete", 0.0);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), 0.0);
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);

	while (gtk_events_pending ()) 
					gtk_main_iteration (); 

	if (path == 0x00)
	{
		func_gtk_dialog_modal (0, window, "\n    [파일/폴더]이 선택되지 않았습니다.    \n");

		return;
	}

	BXLog (DBG, "Start func_Detect()...\n");
	start = time(NULL);
	func_Detect (path);
	end =time(NULL);
	chktime = (double)(end - start);
	BXLog (DBG, "End func_detect()...\n");

	while (gtk_events_pending ()) 
						gtk_main_iteration (); 

	memset( message, 0x00, strlen(message));
	sprintf( message, "%.0f%% Complete", 100.0);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d_progressbar), 100.0);
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR(d_progressbar), message);

	while (gtk_events_pending ()) 
						gtk_main_iteration (); 

	func_Send();

	func_Refresh_ScrollWindow();
	BXLog (DBG, "Total func_detect() Time: [%.3f]초, Average: [%.3f]초\n", chktime, chktime/chk_fcnt);

	func_gtk_dialog_modal (0, window, "\n    검출이 완료되었습니다.    \n");

	return;
}

void d_option_btn_clicked (GtkButton *d_option_btn, gpointer *data)
{
	gtk_widget_show (setting_window);
	
	return;
}

void d_encrypt_btn_clicked (GtkButton *d_encrypt_btn, gpointer *data)
{
	func_ARIA();

	func_Refresh_ScrollWindow();
	//printf ("[UUID: %s], [파일이름: %s], [파일크기: %d], [파일상태: %s], [파일경로: %s]\n", sfDs.uuid, sfDs.fname, sfDs.fsize, sfDs.stat, sfDs.fpath);

	strcpy (sfDs.stat, "암호화");
	func_Send();

	return;
}

void d_delete_btn_clicked (GtkButton *d_delete_btn, gpointer *data)
{
	memset (message, 0x00, strlen (message));
	if (sfDs.fpath[0] == 0x00)
	{
		func_gtk_dialog_modal (0, window, "\n    대상파일이 선택되지 않았습니다.    \n");
	}
	else
	{
		sprintf (message, 
			"\n    삭제 후에 복구가 불가능 합니다.\n    아래 파일을 삭제하시겠습니까?\n    [ %s ]    \n", sfDs.fpath);
    
		if (func_gtk_dialog_modal(1, window, message) == GTK_RESPONSE_ACCEPT)
		{
			int res = 0;

			func_file_eraser (3);

			for (int i = 0; i <= chk_fcnt; i++)
			{
				res = strcmp (fname, fDs[i].fname);

				if (res == 0)
				{
					strcpy (fDs[i].stat, "삭제");
					BXLog (DBG, "결과: [%d]번째 파일[%s]가 [%s] 되었습니다.", i, fDs[i].fname, fDs[i].stat);
				}
			}

			func_Refresh_ScrollWindow();

			strcpy (sfDs.stat, "삭제");
			func_Send();
		}
		else
		{
			func_gtk_dialog_modal (0, window, "\n    취소 되었습니다.    \n");
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (d_progressbar), 0);
		}
	}

	return;
}

void d_close_btn_clicked (GtkButton *d_close_btn, gpointer *data)
{
	gtk_widget_hide (GTK_WIDGET (data));

	return;
}

void detect_window_destroy (GtkWidget *detect_window, gpointer *data)
{
	gtk_widget_destroy (GTK_WIDGET (detect_window));

	return;
}
/* end of detect_window function */


/*---------------------------------------------------------------------------*/
/* enrollment_window function                                */
/*---------------------------------------------------------------------------*/
void e_name_entry_activate (GtkEntry *e_name_entry, gpointer *data)
{
	name = (gchar *)gtk_entry_get_text (e_name_entry);
	strcpy (uDs.uname, name);
	BXLog (DBG, "%s\n", name);
	
	return;
}

void e_jobtitle_cbxtext_changed	(GtkWidget *e_jobtitle_cbxtext, gpointer *data)
{
	job = (gchar *)gtk_entry_get_text (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (e_jobtitle_cbxtext))));
	strcpy (uDs.ujob, job);
	BXLog (DBG, "%s\n", uDs.ujob);

	return;
}


enum
{
	e_treeview_col,
	NUM_COL
} ;

gboolean
e_view_selection_func 	(GtkTreeSelection	*selection,
							GtkTreeModel			*model,
							GtkTreePath			*path,
							gboolean				 path_currently_selected,
							gpointer				 userdata)
{
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter (model, &iter, path))
	{
		gtk_tree_model_get (model, &iter, e_treeview_col, &vs_dept, -1);

		if (!path_currently_selected)
		{
			//g_print ("부서: [%s] 선택.\n", vs_dept);
		}
		else
		{
			//g_print ("부서: [%s] 선택 해제.\n", vs_dept);
		}

		strcpy (uDs.udept, vs_dept);

		g_free (vs_dept);
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
	gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set		(e_treestore, &parent,
								 e_treeview_col,		"대표이사",
								 -1);

	/*임원실 1*/
	gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set		(e_treestore, &parent,
								 e_treeview_col,		"임원실",
								 -1);

	/*대전지사 2*/
	gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set		(e_treestore, &parent,
								 e_treeview_col,		"대전지사",
								 -1);
					 
	/*경영혁신팀 3*/
	gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set		(e_treestore, &parent,
								 e_treeview_col,		"경영혁신팀",
								 -1);

	/*솔루션사업부 4*/
	gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set		(e_treestore, &parent,
								 e_treeview_col,		"솔루션사업부",
								 -1);
		/*솔루션사업부 영업팀 8*/
		gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
		gtk_tree_store_set		(e_treestore, &parent,
									 e_treeview_col,	"솔루션사업부 영업팀",
									 -1);
		/*솔루션사업부 사업팀 9*/
		gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
		gtk_tree_store_set		(e_treestore, &parent,
									 e_treeview_col,	"솔루션사업부 기술팀",
									 -1);

	/*보안인프라사업부 5*/
	gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set		(e_treestore, &parent,
								 e_treeview_col,		"보안인프라사업부",
								 -1);
		/*보안인프라사업부 영업팀 10*/
		gtk_tree_store_append	(e_treestore, &child1, &parent);
		gtk_tree_store_set		(e_treestore, &child1,
									 e_treeview_col,	"보안인프라사업부 영업팀",
									 -1);
		/*보안인프라사업부 기술팀 11*/
		gtk_tree_store_append	(e_treestore, &child1, &parent);
		gtk_tree_store_set		(e_treestore, &child1,
									 e_treeview_col,	"보안인프라사업부 기술팀",
									 -1);

	/*부설연구소 6*/
	gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set		(e_treestore, &parent,
								 e_treeview_col,		"부설연구소",
								 -1);
		/*부설연구소 개발1팀 12*/
		gtk_tree_store_append	(e_treestore, &child1, &parent);
		gtk_tree_store_set		(e_treestore, &child1,
									 e_treeview_col,	"부설연구소 개발1팀",
									 -1);
		/*부설연구소 개발2팀 13*/
		gtk_tree_store_append	(e_treestore, &child1, &parent);
		gtk_tree_store_set		(e_treestore, &child1,
									 e_treeview_col,	"부설연구소 개발2팀",
									 -1);

	/*특수사업부 7*/
	gtk_tree_store_append	(e_treestore, &parent, e_treeview_col);
	gtk_tree_store_set		(e_treestore, &parent,
								 e_treeview_col,		"특수사업부",
								 -1);
		/*특수사업부 자사품 TF팀 14*/
		gtk_tree_store_append	(e_treestore, &child1, &parent);
		gtk_tree_store_set		(e_treestore, &child1,
									 e_treeview_col,	"특수사업부 자사품 TF팀",
									 -1);
		/*특수사업부 컨설팅팀 15*/
		gtk_tree_store_append	(e_treestore, &child1, &parent);
		gtk_tree_store_set		(e_treestore, &child1,
									 e_treeview_col, "특수사업부 컨설팅팀",
									 -1);

	return GTK_TREE_MODEL (e_treestore);
}

static GtkWidget *
e_create_view_and_model (void)
{
	GtkTreeViewColumn	*e_col;
	GtkCellRenderer		*e_renderer;
	GtkWidget				*e_view;
	GtkTreeModel			*e_model;
	GtkTreeSelection	*e_selection;
	
	e_view = gtk_tree_view_new();

	// Column #부서 //
	e_col = gtk_tree_view_column_new();

	gtk_tree_view_column_set_title (e_col, "(주)조은아이앤에스 조직도");

	gtk_tree_view_append_column (GTK_TREE_VIEW (e_view), e_col);
	e_renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start (e_col, e_renderer, TRUE);
	
	gtk_tree_view_column_add_attribute(e_col, e_renderer, "text", e_treeview_col);
	
	e_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (e_view));
	
	gtk_tree_selection_set_select_function (e_selection, e_view_selection_func, NULL, NULL);
	
	e_model = e_create_and_fill_model();

	gtk_tree_view_set_model (GTK_TREE_VIEW (e_view), e_model);

	g_object_unref (e_model); // destroy model automatically with view //
	
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (e_view)),
                              GTK_SELECTION_SINGLE);

	return e_view;
}

void dept_ok_btn_clicked_w (GtkButton *dept_ok_btn,	gpointer *data)
{
	gtk_widget_hide (department_window);
}

void dept_ok_btn_clicked_e (GtkButton *dept_ok_btn,	gpointer *data)
{
	gtk_entry_set_text (GTK_ENTRY(data), uDs.udept);

	return;
}

void dept_close_btn_clicked (GtkButton *dept_close_btn,	gpointer *data)
{
	gtk_widget_hide (department_window);
}

void e_department_btn_clicked (GtkButton *e_department_btn,	gpointer *data)
{
	GtkWidget *e_view;
	
	e_view = e_create_view_and_model();
	gtk_container_add (GTK_CONTAINER (dept_scrolledwindow), e_view);
	gtk_widget_show_all ((GtkWidget *) department_window);
}

void e_enroll_btn_clicked (GtkButton *e_enroll_btn, gpointer *data)
{
	char *usrinfostr = malloc (sizeof(char) * 10);

	chk_df = 2;

	sprintf (usrinfostr, "%s %s", uDs.uname, uDs.ujob);

	gtk_widget_hide (enrollment_window);

	gtk_label_set_text (GTK_LABEL (m_userinfo_label), usrinfostr);
	gtk_widget_show (main_window);

	BXLog (DBG, "부서: %s 사용자: %s, 직급: %s \n", uDs.udept, uDs.uname, uDs.ujob);
	//printf("%s\n", usrinfostr);
	func_Send();

	gtk_main();
	free(usrinfostr);

	return;
}
/* end of enrollment_window */


/*---------------------------------------------------------------------------*/
/* setting_window function                                   */
/*---------------------------------------------------------------------------*/
void s_ip_entry_activate (GtkEntry  *s_ip_entry,	gpointer *data)
{
	char *hostname;
	hostname = (gchar *) gtk_entry_get_text (s_ip_entry);
	BXLog (DBG, "HOST NAME: %s\n", hostname);

	return;
}

void s_port_entry_activate (GtkEntry  *s_port_entry,	gpointer *data)
{
	char *port;
	port = (gchar *) gtk_entry_get_text (s_port_entry);
	BXLog (DBG, "PORT: %s\n", port);

	return;
}

void s_detect_entry_activate	(GtkEntry  *s_detect_entry,	gpointer *data)
{
	path = (gchar *)gtk_entry_get_text(s_detect_entry);
	BXLog (DBG, "선택한 디폴트 폴더 위치: %s\n", dpath);

	return;
}

void s_folder_btn_clicked (GtkButton *s_folder_btn,	gpointer *data)
{
    filechooserdialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW (data), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
			("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all (filechooserdialog);
    
	gint resp = gtk_dialog_run (GTK_DIALOG (filechooserdialog));

    if (resp == GTK_RESPONSE_ACCEPT)
    {
	gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));
    }

	dpath = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooserdialog));

	gtk_entry_set_text (GTK_ENTRY (data), dpath);


	gtk_widget_destroy (filechooserdialog);

	BXLog (DBG, "선택한 디폴트 폴더 위치: %s\n", dpath);

	return;
}

void s_usrchg_btn_clicked (GtkButton *s_usrchg_btn,	gpointer *data)
{
	gtk_widget_show (enrollment_window);

	return;
}


void s_ok_btn_clicked (GtkButton *s_ok_btn,		gpointer *data)
{
	gtk_widget_hide (GTK_WIDGET (data));

	return;
}

void s_cloese_btn_clicked (GtkButton *setting_window, gpointer *data)
{
	gtk_widget_hide (GTK_WIDGET (data));

	return;
}
/* end of setting_window function */


/*---------------------------------------------------------------------------*/
/* main                                                      */
/*---------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
	int chkini;

	LogName = basename(argv[0]);
	gtk_init(&argc, &argv);

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "main.glade", NULL);

	main_window				= GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
	enrollment_window		= GTK_WIDGET (gtk_builder_get_object (builder, "enrollment_window"));
	department_window		= GTK_WIDGET (gtk_builder_get_object (builder, "department_window"));
	detect_window			= GTK_WIDGET (gtk_builder_get_object (builder, "detect_window"));
	setting_window			= GTK_WIDGET (gtk_builder_get_object (builder, "setting_window"));
	d_progressbar 			= GTK_WIDGET (gtk_builder_get_object (builder, "d_progressbar"));

	d_scrolledwindow		= GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder, "d_scrolledwindow"));
	dept_scrolledwindow	= GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder, "dept_scrolledwindow"));

	m_userinfo_label 		= GTK_WIDGET (gtk_builder_get_object (builder, "m_userinfo_label"));

	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (enrollment_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (department_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (detect_window), GTK_WIN_POS_CENTER);
	gtk_window_set_position (GTK_WINDOW (setting_window), GTK_WIN_POS_CENTER);


	// 닫기x 버튼을 hide로 바꾸기, -버튼 활성화 하고 싶으면 glade에서 modal 해제
	g_signal_connect (detect_window,			"delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	g_signal_connect (setting_window,		"delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	g_signal_connect (enrollment_window,	"delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);

	gtk_builder_connect_signals (builder, NULL);

	func_Uuid();			// 사용자 UUID
	
	// Ini File Check
	chkini = func_ParseIni ("/home/joeun/bxr_plover/client/lib/plover.ini");
	if (chkini != 0)
	{
		func_CreateIni();
		BXLog (DBG, "ini 파일 생성!\n");
		chkini = func_ParseIni ("/home/joeun/bxr_plover/client/lib/plover.ini");
    }

	func_SetRabbit();	// 서버와 연결
	func_VerChk();		// 버전 확인
	func_UsrChk();		// 사용자 확인

	g_object_unref(builder);

	gtk_widget_show(window);
	gtk_main();

	return 0;
}
