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




int chk_tf;

GtkWidget *detect_window;
GtkWidget *setting_window;

GtkWidget *m_detect_btn;
GtkWidget *m_setting_btn;

GtkWidget *d_option_btn;
GtkWidget *d_folder_btn;
GtkWidget *d_filechooserdialog;
GtkWidget *d_filechooserdialog_open_btn;

void on_m_detect_btn_clicked(GtkButton *m_detect_btn, gpointer *data);
void on_m_setting_btn_clicked(GtkButton *d_folder_btn, gpointer *data);

void on_d_option_btn_clicked(GtkButton *d_option_btn, gpointer *data);
void on_d_folder_btn_clicked(GtkButton *d_option_btn, gpointer *data);
void on_d_filechooserdialog_open_btn_clicked(GtkButton *d_filechooserdialog_open_btn, gpointer *data);

//계정이 있는지 확인: TRUE(1)=있다 FALSE(0)=없다
int chk_user()
{
	chk_tf = TRUE;
	
	return chk_tf;
}

void on_m_detect_btn_clicked(GtkButton *m_detect_btn, gpointer *data)
{
	gtk_widget_show(detect_window);
	
	return;
}

void on_m_setting_btn_clicked(GtkButton *m_setting_btn, gpointer *data)
{
	gtk_widget_show(setting_window);
	
	return;
}

void on_d_option_btn_clicked(GtkButton *d_option_btn, gpointer *data)
{
	gtk_widget_show(setting_window);
	
	return;
}

void on_d_folder_btn_clicked(GtkButton *d_folder_btn, gpointer *data)
{
	gtk_widget_show(d_filechooserdialog);
	
	return;
}

void on_d_filechooserdialog_open_btn_clicked(GtkButton *d_filechooserdialog_open_btn, gpointer *data)
{
	gtk_widget_hide(GTK_WIDGET(data));
}

int main(int argc, char *argv[])
{
    GtkBuilder	*builder;
    GtkWidget		*main_window,
					*enrollment_window;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "main.glade", NULL);

    main_window				= GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    enrollment_window		= GTK_WIDGET(gtk_builder_get_object(builder, "enrollment_window"));
    detect_window			= GTK_WIDGET(gtk_builder_get_object(builder, "detect_window"));
    setting_window			= GTK_WIDGET(gtk_builder_get_object(builder, "setting_window"));
    d_filechooserdialog	= GTK_WIDGET(gtk_builder_get_object(builder, "d_filechooserdialog"));
    
    gtk_builder_connect_signals(builder, NULL);
    
    m_detect_btn		= GTK_WIDGET(gtk_builder_get_object(builder, "m_detect_btn"));
    m_setting_btn	= GTK_WIDGET(gtk_builder_get_object(builder, "m_setting_btn"));
    d_option_btn		= GTK_WIDGET(gtk_builder_get_object(builder, "d_option_btn"));

    g_object_unref(builder);

	chk_user(chk_tf);
	
	if(chk_tf == FALSE)
	{
		gtk_widget_show(enrollment_window);
		gtk_main();
	}
	
	if(chk_tf == TRUE)
	{
		gtk_widget_show(main_window);
		gtk_main();
	}

    return 0;
}
