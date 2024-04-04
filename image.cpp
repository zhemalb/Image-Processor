#include "image.h"

Color::Color() : r(0), g(0), b(0) {
}

Color::Color(float r, float g, float b) : r(r), g(g), b(b) {
}

Color::~Color() {
}

Image::Image(int width, int height)
    : m_width_(width), m_height_(height), m_colors_(std::vector<Color>(width * height)) {
}

Image::~Image() {
}

Color Image::GetColor(int x, int y) const {
    return m_colors_[y * m_width_ + x];
}

void Image::Read(const char* path) {

    std::ifstream f;
    f.open(path, std::ios::in | std::ios::binary);

    if (!f.is_open()) {
        std::cout << "This file cannot be opened" << std::endl;
        return;
    }

    const int file_header_size = 14;
    const int information_header_size = 40;

    unsigned char file_header[file_header_size];
    f.read(reinterpret_cast<char*>(file_header), file_header_size);

    if (file_header[0] != 'B' || file_header[1] != 'M') {
        std::cout << "This path does not lead to a bitmap image" << std::endl;
        f.close();
        return;
    }

    unsigned char information_header[information_header_size];
    f.read(reinterpret_cast<char*>(information_header), information_header_size);
    const unsigned char index_1 = 5;
    const unsigned char value_1 = 8;
    const unsigned char index_2 = 6;
    const unsigned char value_2 = 16;
    const unsigned char index_3 = 7;
    const unsigned char value_3 = 24;
    m_width_ = information_header[4] + (information_header[index_1] << value_1) +
               (information_header[index_2] << value_2) + (information_header[index_3] << value_3);
    const unsigned char index_4 = 8;
    const unsigned char index_5 = 9;
    const unsigned char index_6 = 10;
    const unsigned char index_7 = 11;
    m_height_ = information_header[index_4] + (information_header[index_5] << value_1) +
                (information_header[index_6] << value_2) + (information_header[index_7] << value_3);

    m_colors_.resize(m_width_ * m_height_);

    const int padding_amount = ((4 - (m_width_ * 3) % 4) % 4);

    for (int y = 0; y < m_height_; ++y) {
        for (int x = 0; x < m_width_; ++x) {
            unsigned char color[3];
            f.read(reinterpret_cast<char*>(color), 3);
            const float max_color = 255.0f;
            m_colors_[y * m_width_ + x].r = static_cast<float>(color[2]) / max_color;
            m_colors_[y * m_width_ + x].g = static_cast<float>(color[1]) / max_color;
            m_colors_[y * m_width_ + x].b = static_cast<float>(color[0]) / max_color;
        }

        f.ignore(padding_amount);
    }

    f.close();

    std::cout << "File read\n";
}

void Image::Crop(int new_width, int new_height) {
    if (new_width > m_width_ || new_height > m_height_) {
        // Если запрошенные ширина или высота превышают размеры исходного изображения,
        // выдаем доступную часть изображения
        new_width = std::min(new_width, m_width_);
        new_height = std::min(new_height, m_height_);
    }

    std::vector<Color> cropped_colors(new_width * new_height);

    // Определяем координаты левого нижнего угла для обрезки
    int start_x = 0;
    int start_y = m_height_ - new_height;

    // Копируем нужную часть изображения в новый буфер
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            cropped_colors[y * new_width + x] = m_colors_[(start_y + y) * m_width_ + (start_x + x)];
        }
    }

    // Обновляем размеры и цвета изображения
    m_width_ = new_width;
    m_height_ = new_height;
    m_colors_ = cropped_colors;

    std::cout << "File was cropped\n";
}

void Image::Grayscale() {
    for (int i = 0; i < m_width_ * m_height_; ++i) {
        const float red = 0.299f;
        const float green = 0.587f;
        const float blue = 0.114f;
        float gray = red * m_colors_[i].r + green * m_colors_[i].g + blue * m_colors_[i].b;
        m_colors_[i].r = gray;
        m_colors_[i].g = gray;
        m_colors_[i].b = gray;
    }
    std::cout << "Grayscale filter was applied\n";
}

void Image::Negative() {
    for (int i = 0; i < m_width_ * m_height_; ++i) {
        m_colors_[i].r = 1.0f - m_colors_[i].r;
        m_colors_[i].g = 1.0f - m_colors_[i].g;
        m_colors_[i].b = 1.0f - m_colors_[i].b;
    }
    std::cout << "Negative filter was applied\n";
}
#define float_two 2.0f
void Image::GaussianBlur(float sigma) {
    std::vector<Color> temporary_colors(m_width_ * m_height_);
    auto gaussian = [sigma](float x, float y) {
        return (1.0f / (float_two * M_PI * sigma * sigma)) *  // M_PI - число Пи
               std::exp(-(x * x + y * y) / (float_two * sigma * sigma));
    };

    // Применяем фильтр к каждому пикселю
    for (int y = 0; y < m_height_; ++y) {
        for (int x = 0; x < m_width_; ++x) {
            float r = 0.0f;
            float g = 0.0f;
            float b = 0.0f;
            float total_weight = 0.0f;

            // Проходим по окрестности пикселя и вычисляем взвешенную сумму значений цветов
            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    int neighbor_x = x + i;
                    int neighbor_y = y + j;

                    if (neighbor_x >= 0 && neighbor_x < m_width_ && neighbor_y >= 0 && neighbor_y < m_height_) {
                        float weight = static_cast<float>(gaussian(static_cast<float>(i), static_cast<float>(j)));
                        total_weight += weight;

                        r += m_colors_[neighbor_y * m_width_ + neighbor_x].r * weight;
                        g += m_colors_[neighbor_y * m_width_ + neighbor_x].g * weight;
                        b += m_colors_[neighbor_y * m_width_ + neighbor_x].b * weight;
                    }
                }
            }

            temporary_colors[y * m_width_ + x].r = r / total_weight;
            temporary_colors[y * m_width_ + x].g = g / total_weight;
            temporary_colors[y * m_width_ + x].b = b / total_weight;
        }
    }

    // Копируем результат обратно в основной массив цветов
    m_colors_ = temporary_colors;
    std::cout << "Gaussian Blur filter was applied\n";
}

void Image::Thermo() {
    std::vector<Color> processed_colors(m_width_ * m_height_);

    for (int y = 1; y < m_height_ - 1; ++y) {
        for (int x = 1; x < m_width_ - 1; ++x) {
            const unsigned char c = 5;
            float new_r = -1 * m_colors_[(y - 1) * m_width_ + (x - 1)].r + -1 * m_colors_[(y - 1) * m_width_ + x].r +
                          -1 * m_colors_[(y - 1) * m_width_ + (x + 1)].r + -1 * m_colors_[y * m_width_ + (x - 1)].r +
                          c * m_colors_[y * m_width_ + x].r + -1 * m_colors_[y * m_width_ + (x + 1)].r +
                          -1 * m_colors_[(y + 1) * m_width_ + (x - 1)].r + -1 * m_colors_[(y + 1) * m_width_ + x].r +
                          -1 * m_colors_[(y + 1) * m_width_ + (x + 1)].r;

            float new_g = -1 * m_colors_[(y - 1) * m_width_ + (x - 1)].g + -1 * m_colors_[(y - 1) * m_width_ + x].g +
                          -1 * m_colors_[(y - 1) * m_width_ + (x + 1)].g + -1 * m_colors_[y * m_width_ + (x - 1)].g +
                          c * m_colors_[y * m_width_ + x].g + -1 * m_colors_[y * m_width_ + (x + 1)].g +
                          -1 * m_colors_[(y + 1) * m_width_ + (x - 1)].g + -1 * m_colors_[(y + 1) * m_width_ + x].g +
                          -1 * m_colors_[(y + 1) * m_width_ + (x + 1)].g;

            float new_b = -1 * m_colors_[(y - 1) * m_width_ + (x - 1)].b + -1 * m_colors_[(y - 1) * m_width_ + x].b +
                          -1 * m_colors_[(y - 1) * m_width_ + (x + 1)].b + -1 * m_colors_[y * m_width_ + (x - 1)].b +
                          c * m_colors_[y * m_width_ + x].b + -1 * m_colors_[y * m_width_ + (x + 1)].b +
                          -1 * m_colors_[(y + 1) * m_width_ + (x - 1)].b + -1 * m_colors_[(y + 1) * m_width_ + x].b +
                          -1 * m_colors_[(y + 1) * m_width_ + (x + 1)].b;

            processed_colors[y * m_width_ + x] = Color(new_r, new_g, new_b);
        }
    }

    m_colors_ = processed_colors;

    std::cout << "Thermo filter was applied\n";
}

void Image::Sharpening() {
    std::vector<Color> processed_colors(m_width_ * m_height_);

    for (int y = 1; y < m_height_ - 1; ++y) {
        for (int x = 1; x < m_width_ - 1; ++x) {
            const unsigned char c = 5;
            float new_r = std::min(
                1.0f,
                std::max(0.0f,
                         0 * m_colors_[(y - 1) * m_width_ + (x - 1)].r + -1 * m_colors_[y * m_width_ + (x - 1)].r +
                             0 * m_colors_[(y + 1) * m_width_ + (x - 1)].r + -1 * m_colors_[(y - 1) * m_width_ + x].r +
                             c * m_colors_[y * m_width_ + x].r + -1 * m_colors_[(y + 1) * m_width_ + x].r +
                             0 * m_colors_[(y - 1) * m_width_ + (x + 1)].r + -1 * m_colors_[y * m_width_ + (x + 1)].r +
                             0 * m_colors_[(y + 1) * m_width_ + (x + 1)].r));

            float new_g = std::min(
                1.0f,
                std::max(0.0f,
                         0 * m_colors_[(y - 1) * m_width_ + (x - 1)].g + -1 * m_colors_[y * m_width_ + (x - 1)].g +
                             0 * m_colors_[(y + 1) * m_width_ + (x - 1)].g + -1 * m_colors_[(y - 1) * m_width_ + x].g +
                             c * m_colors_[y * m_width_ + x].g + -1 * m_colors_[(y + 1) * m_width_ + x].g +
                             0 * m_colors_[(y - 1) * m_width_ + (x + 1)].g + -1 * m_colors_[y * m_width_ + (x + 1)].g +
                             0 * m_colors_[(y + 1) * m_width_ + (x + 1)].g));

            float new_b = std::min(
                1.0f,
                std::max(0.0f,
                         0 * m_colors_[(y - 1) * m_width_ + (x - 1)].b + -1 * m_colors_[y * m_width_ + (x - 1)].b +
                             0 * m_colors_[(y + 1) * m_width_ + (x - 1)].b + -1 * m_colors_[(y - 1) * m_width_ + x].b +
                             c * m_colors_[y * m_width_ + x].b + -1 * m_colors_[(y + 1) * m_width_ + x].b +
                             0 * m_colors_[(y - 1) * m_width_ + (x + 1)].b + -1 * m_colors_[y * m_width_ + (x + 1)].b +
                             0 * m_colors_[(y + 1) * m_width_ + (x + 1)].b));

            processed_colors[y * m_width_ + x] = Color(new_r, new_g, new_b);
        }
    }

    m_colors_ = processed_colors;

    std::cout << "Sharpening filter was applied\n";
}

void Image::EdgeDetection(float threshold) {
    // Применяем фильтр grayscale
    Grayscale();

    // Применяем фильтр Edge Detection к каждому пикселю, начиная с (1, 1) и заканчивая (m_width_ - 2, m_height_ - 2)
    for (int y = 1; y < m_height_ - 1; ++y) {
        for (int x = 1; x < m_width_ - 1; ++x) {
            // Вычисляем новое значение цвета пикселя, используя указанную матрицу
            float new_value = -1 * m_colors_[(y - 1) * m_width_ + (x - 1)].r - m_colors_[y * m_width_ + (x - 1)].r -
                              m_colors_[(y + 1) * m_width_ + (x - 1)].r + -1 * m_colors_[(y - 1) * m_width_ + x].r +
                              4 * m_colors_[y * m_width_ + x].r - m_colors_[(y + 1) * m_width_ + x].r +
                              -1 * m_colors_[(y - 1) * m_width_ + (x + 1)].r - m_colors_[y * m_width_ + (x + 1)].r -
                              m_colors_[(y + 1) * m_width_ + (x + 1)].r;

            // Определяем цвет пикселя в зависимости от порога
            Color new_color = (new_value > threshold) ? Color(1.0f, 1.0f, 1.0f) : Color(0.0f, 0.0f, 0.0f);
            m_colors_[y * m_width_ + x] = new_color;
        }
    }

    std::cout << "Edge Detection filter was applied\n";
}

void Image::Export(const char* path) const {
    std::ofstream f;
    f.open(path, std::ios::out | std::ios::binary);

    if (!f.is_open()) {
        std::cout << "File cannot be opened\n";
        return;
    }

    unsigned char bmp_pad[3] = {0, 0, 0};
    const int padding_amount = ((4 - (m_width_ * 3) % 4) % 4);

    const int file_header_size = 14;
    const int information_header_size = 40;
    const int file_size =
        file_header_size + information_header_size + m_width_ * m_height_ * 3 + padding_amount * m_width_;
    unsigned char file_header[file_header_size];

    // Тип файл
    file_header[0] = 'B';
    file_header[1] = 'M';
    // Размер файла
    const unsigned char val_1 = 8;
    const unsigned char val_2 = 16;
    const unsigned char val_3 = 24;
    file_header[2] = file_size;
    file_header[3] = file_size >> val_1;
    file_header[4] = file_size >> val_2;
    const unsigned char idx_0 = 5;
    file_header[idx_0] = file_size >> val_3;
    // Резерв 1 - не используется
    const unsigned char ind_1 = 6;
    const unsigned char ind_2 = 7;
    file_header[ind_1] = 0;
    file_header[ind_2] = 0;
    // Резерв 2 - не используется
    const unsigned char ind_3 = 8;
    const unsigned char ind_4 = 9;
    file_header[ind_3] = 0;
    file_header[ind_4] = 0;
    // Пиксель инф offset
    const unsigned char ind_5 = 10;
    const unsigned char ind_6 = 11;
    const unsigned char ind_7 = 12;
    const unsigned char ind_8 = 13;
    file_header[ind_5] = file_header_size + information_header_size;
    file_header[ind_6] = 0;
    file_header[ind_7] = 0;
    file_header[ind_8] = 0;

    unsigned char information_header[information_header_size];

    // Размер хэдера
    information_header[0] = information_header_size;
    information_header[1] = 0;
    information_header[2] = 0;
    information_header[3] = 0;
    // Ширина изображения
    const unsigned char indx_1 = 4;
    const unsigned char indx_2 = 5;
    const unsigned char indx_3 = 6;
    const unsigned char indx_4 = 7;
    information_header[indx_1] = m_width_;
    information_header[indx_2] = m_width_ >> val_1;
    information_header[indx_3] = m_width_ >> val_2;
    information_header[indx_4] = m_width_ >> val_3;
    // Высота изображения
    const unsigned char indx_5 = 8;
    const unsigned char indx_6 = 9;
    const unsigned char indx_7 = 10;
    const unsigned char indx_8 = 11;
    information_header[indx_5] = m_height_;
    information_header[indx_6] = m_height_ >> val_1;
    information_header[indx_7] = m_height_ >> val_2;
    information_header[indx_8] = m_height_ >> val_3;
    // Неисп
    const unsigned char indx_9 = 12;
    const unsigned char indx_10 = 13;
    information_header[indx_9] = 1;
    information_header[indx_10] = 0;
    // Бит на пиксель RGB
    const unsigned char indx_11 = 14;
    const unsigned char indx_12 = 15;
    information_header[indx_11] = val_3;
    information_header[indx_12] = 0;
    // Сжатие - без сжатия
    const unsigned char indx_13 = 16;
    const unsigned char indx_14 = 17;
    const unsigned char indx_15 = 18;
    const unsigned char indx_16 = 19;
    information_header[indx_13] = 0;
    information_header[indx_14] = 0;
    information_header[indx_15] = 0;
    information_header[indx_16] = 0;
    // Размер изображения - без сжатия
    const unsigned char indx_17 = 20;
    const unsigned char indx_18 = 21;
    const unsigned char indx_19 = 22;
    const unsigned char indx_20 = 23;
    information_header[indx_17] = 0;
    information_header[indx_18] = 0;
    information_header[indx_19] = 0;
    information_header[indx_20] = 0;
    // X пикселей на метр
    const unsigned char indx_21 = 24;
    const unsigned char indx_22 = 25;
    const unsigned char indx_23 = 26;
    const unsigned char indx_24 = 27;
    information_header[indx_21] = 0;
    information_header[indx_22] = 0;
    information_header[indx_23] = 0;
    information_header[indx_24] = 0;
    // Y пикселей на метр
    const unsigned char indx_25 = 28;
    const unsigned char indx_26 = 29;
    const unsigned char indx_27 = 30;
    const unsigned char indx_28 = 31;
    information_header[indx_25] = 0;
    information_header[indx_26] = 0;
    information_header[indx_27] = 0;
    information_header[indx_28] = 0;
    // Общее количество цветов
    const unsigned char indx_29 = 32;
    const unsigned char indx_30 = 33;
    const unsigned char indx_31 = 34;
    const unsigned char indx_32 = 35;
    information_header[indx_29] = 0;
    information_header[indx_30] = 0;
    information_header[indx_31] = 0;
    information_header[indx_32] = 0;
    // Важные цвета
    const unsigned char indx_33 = 36;
    const unsigned char indx_34 = 37;
    const unsigned char indx_35 = 38;
    const unsigned char indx_36 = 39;
    information_header[indx_33] = 0;
    information_header[indx_34] = 0;
    information_header[indx_35] = 0;
    information_header[indx_36] = 0;

    f.write(reinterpret_cast<char*>(file_header), file_header_size);
    f.write(reinterpret_cast<char*>(information_header), information_header_size);

    for (int y = 0; y < m_height_; y++) {
        for (int x = 0; x < m_width_; x++) {
            const float colorm = 255.0f;
            unsigned char r = static_cast<unsigned char>(GetColor(x, y).r * colorm);
            unsigned char g = static_cast<unsigned char>(GetColor(x, y).g * colorm);
            unsigned char b = static_cast<unsigned char>(GetColor(x, y).b * colorm);

            unsigned char color[] = {b, g, r};

            f.write(reinterpret_cast<char*>(color), 3);
        }

        f.write(reinterpret_cast<char*>(bmp_pad), padding_amount);
    }

    f.close();
    std::cout << "The file has been created\n";
}
