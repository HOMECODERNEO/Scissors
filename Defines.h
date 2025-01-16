#ifndef DEFINES_H
#define DEFINES_H

#define APPLICATION_NAME    "Scissors"
#define APPLICATION_AUTHOR  "BY NEO"
#define APPLICATION_VERSION "v1.2.0"
#define APPLICATION_COUNTRY "Ukraine"

#define COLUMN_COUNT 5              // Максимальное количество изображений в строчке
#define HISTORY_IMAGE_MAX_SIZE 255  // Максимальное допустимое значения количества изображений в истории для хранения

// Еталонные значения разрешения для корректировки сетки истории с изображениями
#define ETALON_HEIGHT 1080
#define ETALON_WIDTH 1920

// Скорость анимации всплывающего уведомления
#define POPUP_ANIMATION_SPEED 200

// Дальность с которой закреплённые изображения прилипают друг к другу
#define FORM_STIKY_CLOSER_PIXELS 10

// Размер маркеров на фигурах в момент их создания и манипуляциях
#define FIGURE_MARKER_SIZE 14

// Коэффициент усиления изгиба линии при перемещении контрольных точек
#define LINE_BEZIER_INTENSITY 5.5

// Размеры по умолчанию при инициализации панели рисования
#define DEFFAULT_SIZE_PEN 2
#define DEFFAULT_SIZE_LINE 2
#define DEFFAULT_SIZE_ERASER 2
#define DEFFAULT_SIZE_FIGURE_BORDER 2

// Параметры закреплённых на экране изображений
const int FormMinimumHeight = 75;
const int FormMinimumWidth = 75;
const int FormBorderWidth = 15;

// Минимальные размеры фигур при рисовании
const int FigureMinimumHeight = 35;
const int FigureMinimumWidth = 35;
const int FigureBorderWidth = 17;

// Ключ шифрования
#define ALGORITHM_KEY "de0ca822b992833244f8de84d552e7ee592b0597c0"

// Файловые константы
#define MAIN_DATA_PATH "/Data/"

// Названия директорий
#define PEN_DASHPATTERNS_PATH "PenDashPatterns/"
#define BRUSH_PATTERNS_PATH "BrushPatterns/"
#define LANGUAGE_PATH "Lang/"
#define CACHE_PATH "Cache/"

#define SETTINGS_FILETYPE "Setting.conf"
#define PENDASHPATTERNS_FILETYPE "Patterns.txt"

#endif // DEFINES_H
