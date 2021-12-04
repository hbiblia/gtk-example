// gcc window.c -o window `pkg-config --libs --cflags gtk4`
// Author: slam - hbiblia@gmail.com
// OS: Windows

#include <stdio.h>
#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Primera ventana en Gtk4 y GTK3");
  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.gtk", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}