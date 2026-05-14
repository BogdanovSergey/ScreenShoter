#include <gtk/gtk.h>

// Обработчик активации приложения
static void activate(GtkApplication *app, gpointer user_data) {
    const char *image_path = g_object_get_data(G_OBJECT(app), "image-path");
    GtkWidget *window = gtk_application_window_new(app);

    char *title = g_strdup_printf("Просмотр скриншота: %s", image_path);    
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
    g_free(title);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_widget_set_margin_top(box, 12);
    gtk_widget_set_margin_bottom(box, 12);

    GtkWidget *label = gtk_label_new(image_path);
    gtk_label_set_xalign(GTK_LABEL(label), 0.5f);
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *picture = gtk_picture_new_for_filename(image_path);
    gtk_picture_set_can_shrink(GTK_PICTURE(picture), TRUE);
    gtk_widget_set_hexpand(picture, TRUE);
    gtk_widget_set_vexpand(picture, TRUE);
    gtk_box_append(GTK_BOX(box), picture);

    GtkWidget *btnSave = gtk_button_new_with_label("Сохранить");
    g_signal_connect(btnSave, "clicked", NULL, NULL);//G_CALLBACK(take_screenshot), NULL);
    gtk_widget_set_halign(btnSave, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), btnSave);

    GtkWidget *btnCopy = gtk_button_new_with_label("Копировать");
    g_signal_connect(btnCopy, "clicked", NULL, NULL);//G_CALLBACK(take_screenshot), NULL);
    gtk_widget_set_halign(btnCopy, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), btnCopy);

    gtk_window_set_child(GTK_WINDOW(window), box);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    if (argc < 2) {
        g_printerr("Использование: %s <путь_к_файлу>\n", argv[0]);
        return 1;
    }

    if (!g_file_test(argv[1], G_FILE_TEST_EXISTS)) {
        g_printerr("Файл не найден: %s\n", argv[1]);
        return 1;
    }

    GtkApplication *app = gtk_application_new("com.example.screenshot.editor", G_APPLICATION_DEFAULT_FLAGS);
    g_object_set_data_full(G_OBJECT(app), "image-path", g_strdup(argv[1]), g_free);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Не передаем путь к изображению в GApplication, чтобы она не
    // пыталась обрабатывать его как "open file" запрос.
    int app_argc = 1;
    char *app_argv[] = { argv[0], NULL };
    int status = g_application_run(G_APPLICATION(app), app_argc, app_argv);
    g_object_unref(app);

    return status;
}