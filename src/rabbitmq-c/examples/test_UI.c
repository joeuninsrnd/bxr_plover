#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <regex.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <assert.h>
#include <dirent.h>

#define MAX_ERROR_MSG 0x1000

typedef struct Data_storage
{
    char jumin;
    char passport;
    char driver;

}data_storage;

static data_storage ds;

/* Compile the regular expression described by "regex_text" into
   "r". */

int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);

    if (status != 0)
    {
    char error_message[MAX_ERROR_MSG];

    regerror (status, r, error_message, MAX_ERROR_MSG);

    printf ("Regex error compiling '%s': %s\n", regex_text, error_message);

    return 1;
    }

    return 0;
}

/*
  Match the string in "to_match" against the compiled regular
  expression in "r".
 */

char match_regex (regex_t * r, const char * to_match)
{
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 100;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    //버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교//
    while (1)
    {
        int nomatch = regexec (r, p, n_matches, m, 0);

        if (nomatch)
        {
            return 0;
        }

        else
        {
            int i;
            for (i = 0; i < n_matches; i++)
            {
                if (m[i].rm_so == -1)
                {
                    break;
                }

                if (i == 0) //주민번호 검사 통과한 부분
                {
                    ds.jumin++; //검출된 주민등록번호의 수
                }
            }
        }

        p += m[0].rm_eo;
    }
}

int scan_dir(const char *path)
{
    DIR* dp = NULL;
    struct dirent* file = NULL;
    struct stat buf;
    char filename[4096];
    FILE* fp = NULL;
    regex_t r;
    const char * regex_text;
    const char * find_text;
    char buffer[5000];

    if ((dp = opendir(path))== NULL)
    {
        printf("폴더를 열수 없습니다.\n");

        return -1;
    }

    while ((file = readdir(dp)) != NULL)
    {
        //filename에 현재 path넣기//￣
        sprintf(filename, "%s/%s", path, file->d_name);
        lstat(filename, &buf);

        //폴더//
        if(S_ISDIR(buf.st_mode))
        {
            // .이거하고 ..이거 제외//
            if ((!strcmp(file->d_name, ".")) || (!strcmp(file->d_name, "..")))
            {
                continue;
            }

            //안에 폴더로 재귀함수//
            scan_dir(filename);
        }

        //파일//
        else if(S_ISREG(buf.st_mode))
        {
            printf("File name: %s\n", file->d_name);
            fp = fopen(filename, "r");

            if(NULL == fp)
            {
                printf("파일을 열수 없습니다.\n");
                return 1;
            }

            //각 개인정보의 정규식//
            regex_text = "([0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-4][0-9]{6})";

            //정규식 컴파일//
            compile_regex (& r, regex_text);

            //버퍼 크기만큼 읽고 find_text에 넣어서 정규식검사로 이동//
            while (feof(fp) == 0)
            {
                fread(buffer, sizeof(char), sizeof(buffer), fp);
                find_text = buffer;
                match_regex (& r, find_text);
            }

            //메모리관리(초기화), 파일닫기//
            memset(buffer, 0, sizeof(buffer));
            regfree (& r);
            fclose(fp);
        }
    }

    closedir(dp);
    return  0;
}

int detect_func()
{
  char *hostname;
  int port, status;
  char *exchange;
  char *routingkey;

  amqp_socket_t *socket = NULL;
  amqp_connection_state_t conn;
  amqp_bytes_t reply_to_queue;

  char path[4096] = "/home/joeun/testd";
  scan_dir(path);


  hostname = "127.0.0.1";
  port = atoi("5672");
  exchange = "aa";
  routingkey = "ka";

  /*
         establish a channel that is used to connect RabbitMQ server
  */

  conn = amqp_new_connection();

  socket = amqp_tcp_socket_new(conn);
  if (!socket) {
        //die("creating TCP socket");
        printf("creating TCP socket\n");
  }

  status = amqp_socket_open(socket, hostname, port);
  if (status) {
        //die("opening TCP socket");
        printf("opening TCP socket\n");
  }

/*
  die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                                                           "guest", "guest"),
                                        "Logging in");*/
  amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
                                                           "guest", "guest");

  amqp_channel_open(conn, 1);

  //die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");
  amqp_get_rpc_reply(conn);

  /*
         create private reply_to queue
  */

  {
        amqp_queue_declare_ok_t *r = amqp_queue_declare(
                conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);


        //die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
        amqp_get_rpc_reply(conn);

        reply_to_queue = amqp_bytes_malloc_dup(r->queue);
        if (reply_to_queue.bytes == NULL) {
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
        if (props.reply_to.bytes == NULL) {
          fprintf(stderr, "Out of memory while copying queue name");
          return 1;
        }
        props.correlation_id = amqp_cstring_bytes("1");

        /*
          publish
        */

        /*die_on_error(amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange),
                                                                        amqp_cstring_bytes(routingkey), 0, 0,
                                                                        &props, amqp_cstring_bytes(&ds.jumin)), "Publishing"); //구조체 포인터부분
        */
        amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange),
                                                                                amqp_cstring_bytes(routingkey), 0, 0,
                                                                                &props, amqp_cstring_bytes(&ds.jumin));

        amqp_bytes_free(props.reply_to);
  }

  /*
        wait an answer
  */

  {
        amqp_basic_consume(conn, 1, reply_to_queue, amqp_empty_bytes, 0, 1, 0,
                                           amqp_empty_table);
        //die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");
        amqp_bytes_free(reply_to_queue);

        {
          amqp_frame_t frame;
          int result;

          amqp_basic_deliver_t *d;
          amqp_basic_properties_t *p;
          size_t body_target;
          size_t body_received;

          for (;;) {
                amqp_maybe_release_buffers(conn);
                result = amqp_simple_wait_frame(conn, &frame);
                printf("Result: %d\n", result);
                if (result < 0) {
                  break;
                }

                printf("Frame type: %u channel: %u\n", frame.frame_type, frame.channel);
                if (frame.frame_type != AMQP_FRAME_METHOD) {
                  continue;
                }

                printf("Method: %s\n", amqp_method_name(frame.payload.method.id));
                if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD) {
                  continue;
                }

                d = (amqp_basic_deliver_t *)frame.payload.method.decoded;
                printf("Delivery: %u exchange: %.*s routingkey: %.*s\n",
                           (unsigned)d->delivery_tag, (int)d->exchange.len,
                           (char *)d->exchange.bytes, (int)d->routing_key.len,
                           (char *)d->routing_key.bytes);

                result = amqp_simple_wait_frame(conn, &frame);
                if (result < 0) {
                  break;
                }

                if (frame.frame_type != AMQP_FRAME_HEADER) {
                  fprintf(stderr, "Expected header!");
                  abort();
                }
                p = (amqp_basic_properties_t *)frame.payload.properties.decoded;
                if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
                  printf("Content-type: %.*s\n", (int)p->content_type.len,
                                 (char *)p->content_type.bytes);
                }
                printf("----\n");

                body_target = (size_t)frame.payload.properties.body_size;
                body_received = 0;

                while (body_received < body_target) {
                  result = amqp_simple_wait_frame(conn, &frame);
                  if (result < 0) {
                        break;
                  }

                  if (frame.frame_type != AMQP_FRAME_BODY) {
                        fprintf(stderr, "Expected body!");
                        abort();
                  }

                  body_received += frame.payload.body_fragment.len;
                  assert(body_received <= body_target);

                  /*amqp_dump(frame.payload.body_fragment.bytes,
                                        frame.payload.body_fragment.len);*/
                }

                if (body_received != body_target) {
                  /* Can only happen when amqp_simple_wait_frame returns <= 0 */
                  /* We break here to close the connection */
                  break;
                }

                /* everything was fine, we can quit now because we received the reply */
                break;
          }
        }
  }

  /*
         closing
  */

/*
  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS),
                                        "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS),
                                        "Closing connection");
  die_on_error(amqp_destroy_connection(conn), "Ending connection");
*/
  amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);


  return 0;
}

GtkWidget *g_btn_detect;
GtkWidget *g_cbt;
GtkWidget *g_clb;


int main(int argc, char *argv[])
{
    GtkBuilder *gtkBuilder;
    GtkWidget *window;
    gtk_init(&argc, &argv);

    gtkBuilder = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilder, "tglade.glade", NULL);
    window = GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "appwindow1"));
    gtk_builder_connect_signals(gtkBuilder, NULL);


    g_object_unref(G_OBJECT(gtkBuilder));
    gtk_widget_show(window);
    gtk_main();

    return 0;
}

void on_btn_detect_clicked()
{
    detect_func();
}

