#include <gtk/gtk.h>
#include <cairo.h>

typedef struct {
    GdkPixbuf *pixbuf;
    cairo_surface_t *surface;  // Поверхность для рисования
    int width;
    int height;
    char *image_path;          // Путь к исходному файлу
} DrawData;

// Функция отрисовки для области рисования
static void draw_function(GtkDrawingArea *area, cairo_t *cr,
                          int width, int height, gpointer user_data) {
    DrawData *data = (DrawData *)user_data;

    if (!data->surface) {
        return;
    }

    // Рисуем сохраненную поверхность (включая все нарисованные элементы)
    cairo_set_source_surface(cr, data->surface, 0, 0);
    cairo_paint(cr);
}

// Инициализация поверхности для рисования
static void init_drawing_surface(DrawData *data) {
    if (data->pixbuf) {
        data->width = gdk_pixbuf_get_width(data->pixbuf);
        data->height = gdk_pixbuf_get_height(data->pixbuf);

        // Создаем поверхность того же размера, что и изображение
        data->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                   data->width,
                                                   data->height);

        // Рисуем исходное изображение на поверхность
        cairo_t *cr = cairo_create(data->surface);
        gdk_cairo_set_source_pixbuf(cr, data->pixbuf, 0, 0);
        cairo_paint(cr);
        cairo_destroy(cr);
    }
}

// Функция для рисования прямоугольника НА ПОВЕРХНОСТИ
static void draw_rectangle_on_surface(DrawData *data) {
    if (!data->surface) return;

    // Получаем контекст из поверхности
    cairo_t *cr = cairo_create(data->surface);

    // Рисуем прямоугольник
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);  // Красный цвет
    cairo_set_line_width(cr, 2.0);
    cairo_rectangle(cr, 50, 50, 100, 100);
    cairo_stroke(cr);

    cairo_destroy(cr);
}

// Обработчик сохранения
static void save_callback(GtkButton *button, gpointer user_data) {
    DrawData *data = (DrawData *)user_data;

    if (!data->surface) {
        g_printerr("Нет данных для сохранения\n");
        return;
    }

    // Используем путь из структуры данных
    const char *image_path = data->image_path;

    // Создаем новый путь для сохранения (можно добавить суффикс "_edited")
    char *save_path = g_strdup_printf("%s_edited.png", image_path);

    // Сохраняем поверхность в PNG файл
    cairo_status_t status = cairo_surface_write_to_png(data->surface, save_path);

    if (status == CAIRO_STATUS_SUCCESS) {
        g_print("Изображение сохранено: %s\n", save_path);
    } else {
        g_printerr("Ошибка сохранения: %s\n", cairo_status_to_string(status));
    }

    g_free(save_path);

    GApplication *app = g_application_get_default();
    if (app != NULL) {
        g_application_quit(app);
    }
    printf("Выходим");
    exit(0);
}

// Освобождение ресурсов
static void free_draw_data(gpointer user_data) {
    DrawData *data = (DrawData *)user_data;
    if (data->pixbuf) {
        g_object_unref(data->pixbuf);
    }
    if (data->surface) {
        cairo_surface_destroy(data->surface);
    }
    if (data->image_path) {
        g_free(data->image_path);
    }
    g_free(data);
}

// Функция-обработчик нажатия клавиш
static void on_key_pressed(GtkEventControllerKey *controller,
                               guint keyval,
                               guint keycode,
                               GdkModifierType state,
                               gpointer user_data) {
    //DrawData *data = (DrawData *)user_data;

    if (keyval == GDK_KEY_Escape) {
        g_print("Нажата клавиша Esc\n");
        GtkWidget *window = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(controller));
        gtk_window_close(GTK_WINDOW(window));
    }
}

static void on_mouse_pressed(GtkGestureClick *gesture,
                             gint n_press,
                             double x,
                             double y,
                             gpointer user_data) {

    g_print("on_mouse_pressed\n");

}

// Обработчик активации приложения
static void activate(GtkApplication *app, gpointer user_data) {
    const char *image_path = g_object_get_data(G_OBJECT(app), "image-path");

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Редактор скриншота");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);

//    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
//    gtk_widget_set_margin_start(box, 12);
//    gtk_widget_set_margin_end(box, 12);
//    gtk_widget_set_margin_top(box, 12);
//    gtk_widget_set_margin_bottom(box, 12);

    // Создаем горизонтальный бокс для картинки и области рисования
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_vexpand(box, TRUE);

    // Загружаем изображение
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, &error);
    if (!pixbuf) {
        g_printerr("Ошибка загрузки изображения: %s\n", error->message);
        g_error_free(error);
        return;
    }

    // Создаем структуру данных
    DrawData *draw_data = g_new0(DrawData, 1);
    draw_data->pixbuf = pixbuf;
    draw_data->image_path = g_strdup(image_path);  // Копируем путь

    // Инициализируем поверхность для рисования
    init_drawing_surface(draw_data);

    // ⚠️ ВАЖНО: Рисуем прямоугольник НА ПОВЕРХНОСТИ, а не в draw_function
    draw_rectangle_on_surface(draw_data);

    // Создаем область рисования
    GtkWidget *drawing_area = gtk_drawing_area_new();
    //gtk_widget_set_size_request(drawing_area, 200, -1); // Ширина 200 пикселей, высота автоматическая
    gtk_widget_set_hexpand(drawing_area, TRUE);
    gtk_widget_set_vexpand(drawing_area, TRUE);
    //gtk_box_append(GTK_BOX(box), drawing_area);

    // Устанавливаем функцию отрисовки
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area),
                                   draw_function, draw_data,
                                   free_draw_data);
    gtk_box_append(GTK_BOX(box), drawing_area);

    // Добавляем обработчик клика мыши
    GtkGesture *click_gesture = gtk_gesture_click_new();
    g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_mouse_pressed), draw_data);
    gtk_widget_add_controller(GTK_WIDGET(drawing_area), GTK_EVENT_CONTROLLER(click_gesture));

    // Добавляем обработчик клавиатуры
    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), NULL);
    gtk_widget_add_controller(GTK_WIDGET(window), key_controller);


    // Кнопка сохранения
    GtkWidget *btnSave = gtk_button_new_with_label("Сохранить в файл и выйти");
    g_signal_connect(btnSave, "clicked", G_CALLBACK(save_callback), draw_data);
    gtk_widget_set_halign(btnSave, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(box), btnSave);

    // Кнопка
    GtkWidget *btnCopy = gtk_button_new_with_label("Копировать");
    //g_signal_connect(btnCopy, "clicked", NULL, NULL);//G_CALLBACK(take_screenshot), NULL);
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