// Жемуков Альберт Артурович БПМИ239 2023-2024 год обучения. Проект по предмету "Программирование на C++"
#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#ifndef M_PI  // число Пи (magic number)
#define M_PI 3.14159265358979323846
#endif

struct Color {
    float r, g, b;

    Color();
    Color(float r, float g, float b);
    ~Color();
};

class Image {
public:
    Image(int width, int height);
    ~Image();
    //
    Color GetColor(int x, int y) const;
    // Чтение и экспорт
    void Read(const char* path);
    void Export(const char* path) const;
    // Фильтры и изменение размера изображения
    void Crop(int new_width, int new_height);
    void Grayscale();
    void Negative();
    void GaussianBlur(float sigma);
    void Sharpening();
    void Thermo();  // доп фильтр 1
    void EdgeDetection(float threshold);

private:
    int m_width_;
    int m_height_;
    std::vector<Color> m_colors_;
};
