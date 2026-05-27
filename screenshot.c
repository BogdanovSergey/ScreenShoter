#include <gtk/gtk.h>
#include <libportal/portal.h>
#include <libportal/portal-helpers.h>
#include <gio/gio.h>


// Обработчик завершения операции
static void on_screenshot_ready(GObject *source, GAsyncResult *res, gpointer user_data) {
    g_autoptr(GError) error = NULL;
    XdpPortal *portal = XDP_PORTAL(source);

    // Получаем URI сохраненного изображения
    g_autofree gchar *uri = xdp_portal_take_screenshot_finish(portal, res, &error);
    if (error != NULL) {
        g_warning("Ошибка при создании скриншота: %s", error->message);
        return;
    }

    if (uri == NULL || *uri == '\0') {
        g_print("Скриншот не был сделан пользователем\n");
        return;
    }

    g_print("Скриншот сохранен: %s\n", uri);

    // Конвертируем URI в путь
    g_autofree gchar *path = g_filename_from_uri(uri, NULL, NULL);
    if (path) {
        g_print("Путь к файлу: %s\n", path);

        // Можно скопировать файл куда-то
        g_autofree gchar *new_path = g_build_filename(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES), "screenshot.png", NULL);
        g_file_copy(g_file_new_for_path(path), g_file_new_for_path(new_path), G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error);

        if (error) {
            g_warning("Ошибка копирования: %s", error->message);
        } else {
            g_print("Скриншот скопирован, открываем для редактирования: %s\n", new_path);

            char command[512] = "./screenshot_editor";
            snprintf(command, sizeof(command), "./screenshot_editor %s &", new_path);
            system(command);
        }
    }


    // exit
    GApplication *app = g_application_get_default();
    if (app != NULL) {
        g_application_quit(app);
    }
    printf("Выходим");
    exit(0);
}

// Функция для запуска процесса создания скриншота
static void take_screenshot(GtkButton *button, gpointer user_data) {
    // Создаем объект портала
    g_autoptr(XdpPortal) portal = xdp_portal_new();
    if (portal == NULL) {
        g_warning("Не удалось создать объект портала");
        return;
    }

    // Запрашиваем скриншот
    xdp_portal_take_screenshot(
        portal,
        NULL,
        XDP_SCREENSHOT_FLAG_INTERACTIVE,
        NULL,
        on_screenshot_ready,
        NULL);
}

// Обработчик активации приложения
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "GTK4 Скриншот");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    // Создаем содержимое окна
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);

    GtkWidget *label = gtk_label_new("Нажмите кнопку для создания скриншота");
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *button = gtk_button_new_with_label("Сделать скриншот");
    g_signal_connect(button, "clicked", G_CALLBACK(take_screenshot), NULL);
    gtk_box_append(GTK_BOX(box), button);

    gtk_window_set_child(GTK_WINDOW(window), box);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.screenshot", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}