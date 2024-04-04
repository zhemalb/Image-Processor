#include <map>
#include <cstdlib>
#include <functional>
#include "image.h"

using Func = std::function<void(Image&, const std::vector<float>&)>;

struct FilterInfo {
    std::string name;
    std::vector<float> parameters;
};

void HandleCropFilter(Image& image, const std::vector<float>& parameters) {
    if (parameters.size() == 2) {
        int new_width = static_cast<int>(parameters[0]);
        int new_height = static_cast<int>(parameters[1]);
        image.Crop(new_width, new_height);
    } else {
        std::cerr << "Error: Incorrect number of parameters for filter -crop" << std::endl;
    }
}

void HandleGrayscaleFilter(Image& image, const std::vector<float>& parameters) {
    image.Grayscale();
}

void HandleNegativeFilter(Image& image, const std::vector<float>& parameters) {
    image.Negative();
}

void HandleBlurFilter(Image& image, const std::vector<float>& parameters) {
    if (parameters.size() == 1) {
        float sigma = parameters[0];
        image.GaussianBlur(sigma);
    } else {
        std::cerr << "Error: Incorrect number of parameters for filter -blur" << std::endl;
    }
}

void HandleSharpeningFilter(Image& image, const std::vector<float>& parameters) {
    image.Sharpening();
}

void HandleThermoFilter(Image& image, const std::vector<float>& parameters) {
    image.Thermo();
}

void HandleEdgeDetectionFilter(Image& image, const std::vector<float>& parameters) {
    if (parameters.size() == 1) {
        float threshold = parameters[0];
        image.EdgeDetection(threshold);
    } else {
        std::cerr << "Error: Incorrect number of parameters for filter -edge" << std::endl;
    }
}

void ApplyFilters(Image& image, const std::vector<FilterInfo>& filters) {
    // Словарь для хранения функций-обработчиков для каждого типа фильтра
    std::map<std::string, Func> filter_handlers = {{"-crop", HandleCropFilter},         {"-gs", HandleGrayscaleFilter},
                                                   {"-neg", HandleNegativeFilter},      {"-blur", HandleBlurFilter},
                                                   {"-sharp", HandleSharpeningFilter},  {"-thermo", HandleThermoFilter},
                                                   {"-edge", HandleEdgeDetectionFilter}};
    // Применяем каждый фильтр к изображению
    for (const auto& filter : filters) {
        if (filter_handlers.find(filter.name) != filter_handlers.end()) {
            // Вызываем соответствующий обработчик фильтра с параметрами
            filter_handlers[filter.name](image, filter.parameters);
        } else {
            std::cerr << "Error: Unknown filter " << filter.name << std::endl;
        }
    }
}

// Функция для обработки аргументов командной строки
std::vector<FilterInfo> ParseCommandLine(int argc, char* argv[]) {
    std::vector<FilterInfo> filters;

    // Пропускаем первый аргумент (имя программы)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Проверяем, является ли аргумент именем фильтра
        if (arg.substr(0, 1) == "-") {
            FilterInfo filter_info;
            filter_info.name = arg;

            // Получаем параметры фильтра (если они есть)
            for (int j = i + 1; j < argc; ++j) {
                std::string param = argv[j];
                // Если следующий аргумент начинается с "-", значит текущий аргумент является последним параметром
                if (param.substr(0, 1) == "-") {
                    break;
                }
                filter_info.parameters.push_back(static_cast<float>(std::atof(param.c_str())));
                ++i;  // Переходим к следующему аргументу
            }

            filters.push_back(filter_info);
        }
    }

    return filters;
}

int main(int argc, char* argv[]) {
    // Проверяем, что передано достаточно аргументов
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <input_file> <output_file> [-filter1 param1 param2 ...] [-filter2 param1 param2 ...] ..."
                  << std::endl;
        return 1;
    }

    // Получаем имя входного файла и выходного файла
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];

    // Создаем объект изображения из входного файла
    Image image(0, 0);
    image.Read(input_filename);

    // Получаем список фильтров из аргументов командной строки
    std::vector<FilterInfo> filters = ParseCommandLine(argc, argv);

    // Применяем фильтры к изображению
    ApplyFilters(image, filters);

    // Сохраняем изображение в выходной файл
    image.Export(output_filename);

    std::cout << "Image processed successfully!" << std::endl;

    return 0;
}
