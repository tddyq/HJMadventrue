#pragma once
#include <wtypes.h>
#include <vector>
#include<iostream>
#include <graphics.h>
class Atlas
{
public:
    Atlas() = default;
    ~Atlas() = default;

    void load_from_file(LPCTSTR path_template, int num) {
        img_list.clear();
        img_list.resize(num);

        TCHAR path_file[256];
        for (int i = 0; i < num; i++) {
            _stprintf_s(path_file, path_template, i + 1); // 注意：此处应为 _stprintf_s
            loadimage(&img_list[i], path_file);
        }
        if (img_list.empty()) {
            std::cout << path_file << " loading fail" << std::endl;
        }
        else {
            std::cout << path_file << " loading success" << std::endl;
        }
    }

    void clear() {
        img_list.clear();
    }

    int get_size() {
        if (img_list.empty()) {
            std::cout << "Atlas is empty!" << std::endl;
        }
        return (int)img_list.size();
    }

    IMAGE* get_image(int idx) {
        if (idx < 0 || idx >= img_list.size())  // 修正 "@" 为 "0"
            return nullptr;
        return &img_list[idx];
    }

    void add_image(const IMAGE& img) {
        img_list.push_back(img);
    }
private:
    std::vector<IMAGE> img_list;
};

