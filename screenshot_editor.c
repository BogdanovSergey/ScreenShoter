#include <gtk/gtk.h>
#include <cairo.h>
#include <stdint.h>

// Типы фигур
typedef enum {
    POINT,
    LINE,
    ARROW,
    RECTANGLE,
    CIRCLE
} FigureType;

// Структура фигуры
typedef struct {
    FigureType type;
    double x1, y1; // Начальные координаты (в процентах от ширины/высоты или абсолютные)
    double x2, y2; // Конечные координаты (для линий и прямоугольников)
    double radius; // Радиус (для точек и кругов)
    double r, g, b; // Цвет
    double line_width; // Толщина линии
} Figure;

// Главная структура-хранилище
typedef struct {
    GdkPixbuf *pixbuf;
    cairo_surface_t *surface;  // Поверхность для рисования
    int width;
    int height;
    char *image_path;       // Путь к исходному файлу
    GArray *figures;        // массив нарисованных фигур
    FigureType draw_mode;   // Текущая выбранная фигура для рисования
    bool is_drawing;    // Флаг, что рисование началось
    double start_x, start_y; // Координаты при нажатии
    double end_x, end_y;     // Координаты при отпускании
} DrawData;

// Функция отрисовки для области рисования cairo
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
static void init_drawing_surface(DrawData *data, int width, int height) {
    if (!data->pixbuf) {
        return;
    }

    // Очищаем старую поверхность если есть
    if (data->surface) {
        cairo_surface_destroy(data->surface);
    }

    data->width = width;
    data->height = height;

    data->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                               width,
                                               height);

    // Рисуем исходное изображение на поверхность
    cairo_t *cr = cairo_create(data->surface);
    //gdk_cairo_set_source_pixbuf(cr, data->pixbuf, 0, 0);
    // Очищаем фон белым цветом
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // Масштабируем и рисуем изображение
    int img_width = gdk_pixbuf_get_width(data->pixbuf);
    int img_height = gdk_pixbuf_get_height(data->pixbuf);

    double scale_x = (double)width / img_width;
    double scale_y = (double)height / img_height;
    double scale = MIN(scale_x, scale_y);  // Сохраняем пропорции

    int new_width = img_width * scale;
    int new_height = img_height * scale;
    int offset_x = (width - new_width) / 2;
    int offset_y = (height - new_height) / 2;


    g_print("new_width new_height: %d x %d\n", new_width, img_height);
    // Применяем масштабирование
    cairo_scale(cr, scale, scale);
    gdk_cairo_set_source_pixbuf(cr, data->pixbuf, offset_x / scale, offset_y / scale);
    cairo_paint(cr);

    cairo_destroy(cr);

}

// Функция-обработчик изменения размера gtk_widget
static void on_drawing_area_resize(GtkDrawingArea *area,
                                   int width,
                                   int height,
                                   gpointer user_data) {
    DrawData *data = (DrawData *)user_data;

    g_print("Изменение размера: %d x %d\n", width, height);

    // Пересоздаем поверхность с новыми размерами
    init_drawing_surface(data, width, height);

    // Перерисовываем область
    gtk_widget_queue_draw(GTK_WIDGET(area));
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
    DrawData *data = (DrawData *)user_data;
    if (!data->surface) {
        g_printerr("Поверхность не инициализирована\n");
        return;
    }

    g_print("Нажал мышь: %.2f, %.2f, режим %d\n", x, y, data->draw_mode);
    data->start_x = x;
    data->start_y = y;
    data->is_drawing = TRUE;

    if(data->draw_mode == POINT) {
        g_print("WILL draw POINT\n");
    } else if(data->draw_mode == LINE) {
        g_print("WILL draw LINE\n");
    }

    // Получаем контекст из поверхности для рисования
    cairo_t *cr = cairo_create(data->surface);
    // Настраиваем цвет точки (например, красный)
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    // Рисуем точку (круг радиусом 5 пикселей)
    double radius = 5.0;
    cairo_arc(cr, x, y, radius, 0, 2 * G_PI);
    cairo_fill(cr);
    // Освобождаем контекст
    cairo_destroy(cr);
    // ⚠️ ВАЖНО: Запрашиваем перерисовку области рисования,
    // чтобы новая точка появилась на экране
    GtkWidget *area = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    gtk_widget_queue_draw(area);

}
static void on_mouse_released(GtkGestureClick *gesture,
                             gint n_press,
                             double x,
                             double y,
                             gpointer user_data) {
    DrawData *data = (DrawData *)user_data;
    if (!data->surface) {
        g_printerr("Поверхность не инициализирована\n");
        return;
    }

    g_print("Отпустил мышь: %.2f, %.2f, режим %d\n", x, y, data->draw_mode);
    data->end_x = x;
    data->end_y = y;
    data->is_drawing = FALSE;

}


static void on_radio_toggled(GtkCheckButton *check_button, gpointer user_data) {
    DrawData *data = (DrawData *)user_data;

    // Проверяем, какая кнопка активна
    if (gtk_check_button_get_active(GTK_CHECK_BUTTON(check_button))) {
        const char *label = gtk_check_button_get_label(GTK_CHECK_BUTTON(check_button));
        g_print("Выбран режим: %s\n", label);

        if (g_strcmp0(label, "Точка") == 0) { // TODO вынести названия
            data->draw_mode = POINT;
        } else if (g_strcmp0(label, "Стрелка") == 0) {
            data->draw_mode = LINE;
        } else {
            data->draw_mode = POINT;
        }
    }
}

// Обработчик активации приложения
static void activate(GtkApplication *app, gpointer user_data) {
    const char *image_path = g_object_get_data(G_OBJECT(app), "image-path");

    int8_t style_padding= 5;
    DrawData *draw_data = g_new0(DrawData, 1);

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Редактор скриншота");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);

    // Создаем главный бокс
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, style_padding);
    gtk_widget_set_hexpand(main_box, TRUE);
    gtk_widget_set_vexpand(main_box, TRUE);



    // Верхняя панель
    GtkWidget *toolbar_panel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, style_padding);
    gtk_widget_set_margin_start(toolbar_panel, style_padding);
    gtk_widget_set_margin_end(toolbar_panel, style_padding);
    gtk_widget_set_margin_top(toolbar_panel, style_padding);
    gtk_widget_set_margin_bottom(toolbar_panel, style_padding);

    // Добавляем панель  в главный бокс
    gtk_box_append(GTK_BOX(main_box), toolbar_panel);


    // Создаем группу радио-кнопок
    GtkWidget *radio1 = gtk_check_button_new_with_label("Точка");  // TODO вынести названия
    GtkWidget *radio2 = gtk_check_button_new_with_label("Стрелка");
    // Добавляем CSS класс
    gtk_widget_add_css_class(radio1, "radio-button-style");
    gtk_widget_add_css_class(radio2, "radio-button-style");
    // Объединяем их в одну группу (вторая кнопка присоединяется к первой)
    gtk_check_button_set_group(GTK_CHECK_BUTTON(radio2), GTK_CHECK_BUTTON(radio1));
    // Устанавливаем первую кнопку как выбранную по умолчанию
    gtk_check_button_set_active(GTK_CHECK_BUTTON(radio1), TRUE);
    // Добавляем обработчики сигналов
    g_signal_connect(radio1, "toggled", G_CALLBACK(on_radio_toggled), draw_data);
    g_signal_connect(radio2, "toggled", G_CALLBACK(on_radio_toggled), draw_data);
    // Добавляем в панель инструментов
    gtk_box_append(GTK_BOX(toolbar_panel), radio1);
    gtk_box_append(GTK_BOX(toolbar_panel), radio2);



    // ⚠️ Устанавливаем цвет фона через CSS
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "style.css");
    gtk_style_context_add_provider_for_display(
            gdk_display_get_default(),
            GTK_STYLE_PROVIDER(css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);


    //gtk_css_provider_load_from_string(css_provider, ".custom-bg { background-color: #b0b0b0; }");
    // Применяем CSS глобально для дисплея
//    gtk_style_context_add_provider_for_display(
//            gdk_display_get_default(),
//            GTK_STYLE_PROVIDER(css_provider),
//            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
//    );

    g_object_unref(css_provider);
    // Добавляем CSS класс к боксу
    gtk_widget_add_css_class(main_box, "custom-bg");



    // Загружаем изображение
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(image_path, &error);
    if (!pixbuf) {
        g_printerr("Ошибка загрузки изображения: %s\n", error->message);
        g_error_free(error);
        return;
    }

    // Создаем структуру данных

    draw_data->pixbuf = pixbuf;
    draw_data->image_path = g_strdup(image_path);  // Копируем путь
    draw_data->surface = NULL;  // Инициализируем как NULL

    // Создаем область рисования
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawing_area, TRUE);
    gtk_widget_set_vexpand(drawing_area, TRUE);

    // Устанавливаем функцию отрисовки
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area),
                                   draw_function,
                                   draw_data,
                                   free_draw_data);

    // ⚠️ ВАЖНО: Подключаем обработчик изменения размера
    g_signal_connect(drawing_area, "resize",
                     G_CALLBACK(on_drawing_area_resize),
                     draw_data);

    gtk_box_append(GTK_BOX(main_box), drawing_area);

    // Добавляем обработчик клика мыши
    GtkGesture *click_gesture = gtk_gesture_click_new();
    g_signal_connect(click_gesture, "pressed", G_CALLBACK(on_mouse_pressed), draw_data);
    g_signal_connect(click_gesture, "released", G_CALLBACK(on_mouse_released), draw_data);
    gtk_widget_add_controller(GTK_WIDGET(drawing_area), GTK_EVENT_CONTROLLER(click_gesture));

    // Добавляем обработчик клавиатуры
    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), NULL);
    gtk_widget_add_controller(GTK_WIDGET(window), key_controller);




    // Нижняя панель
    GtkWidget *bottom_panel= gtk_box_new(GTK_ORIENTATION_HORIZONTAL, style_padding);
    gtk_widget_set_margin_start(bottom_panel, style_padding);
    gtk_widget_set_margin_end(bottom_panel, style_padding);
    gtk_widget_set_margin_top(bottom_panel, style_padding);
    gtk_widget_set_margin_bottom(bottom_panel, style_padding);
    gtk_widget_set_halign(bottom_panel, GTK_ALIGN_END);
    // Кнопка сохранения
    GtkWidget *btnSave = gtk_button_new_with_label("Сохранить в файл и выйти");
    g_signal_connect(btnSave, "clicked", G_CALLBACK(save_callback), draw_data);
    gtk_box_append(GTK_BOX(bottom_panel), btnSave);
    // Добавляем панель  в главный бокс
    gtk_box_append(GTK_BOX(main_box), bottom_panel);



    gtk_window_set_child(GTK_WINDOW(window), main_box);
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