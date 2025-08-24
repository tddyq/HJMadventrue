#pragma once

#include "camera.h"

#include <graphics.h>

#pragma comment(lib, "WINMM.lib")
#pragma comment(lib, "MSIMG32.lib")

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

class Rect
{
public:
    Rect() {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
    }
    Rect(int x,int y,int w,int h):x(x),y(y),w(w),h(h){}
    ~Rect() = default;

    bool operator==(const Rect& other) {
        return (this->x == other.x) && (this->y == other.y)
            && (this->w == other.w) && (this->h == other.h);
    }
    bool operator!=(const Rect& other) {
        return (this->x != other.x) || (this->y != other.y)
            || (this->w != other.w) || (this->h != other.h);
    }
    //矩阵合体
    Rect combineRects(const Rect& b) {
        // 如果a是空矩形，返回b
        if (this->w <= 0 || this->h <= 0) return b;

        // 如果b是空矩形，返回a
        if (b.w <= 0 || b.h <= 0) return *this;

        // 计算合并后的边界
        int left = std::min(this->x, b.x);
        int top = std::min(this->y, b.y);
        int right = std::max(this->x + this->w, b.x + b.w);
        int bottom = std::max(this->y + this->h, b.y + b.h);

        return {
            left,
            top,
            right - left,
            bottom - top
        };
    }
    //辅助函数：检查矩形是否相交
    bool intersects(const Rect& other) const {
        return !(
            (x + w < other.x) ||  // a在b左侧
            (other.x + other.w < x) ||  // a在b右侧
            (y + h < other.y) ||  // a在b上方
            (other.y + other.h < y)   // a在b下方
            );
    }
    bool intersects(int x1, int y1, int x2, int y2) const {
        return !(
            (this->x + this->w < x1) ||  // 当前矩形在目标左侧
            (x2 < this->x) ||          // 当前矩形在目标右侧
            (this->y + this->h < y1) || // 当前矩形在目标上方
            (y2 < this->y)             // 当前矩形在目标下方
            );
    }
    bool isValid() const {
        return (w > 0 && h > 0);
    }
public:
    int x, y;
    int w, h;

};

inline void putimage_ex(const Camera& camera, IMAGE* img, const Rect* rect_dst, const Rect* rect_src = nullptr)
{
    //调试
    std::cout << "正在渲染" << std::endl;

    static BLENDFUNCTION blend_func = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    const Vector2& pos_camera = camera.get_position();
    AlphaBlend(GetImageHDC(GetWorkingImage()),
        (int)(rect_dst->x - pos_camera.x), (int)(rect_dst->y - pos_camera.y),
        rect_dst->w, rect_dst->h, GetImageHDC(img), rect_src ? rect_src->x : 0, rect_src ? rect_src->y : 0,
        rect_src ? rect_src->w : img->getwidth(), rect_src ? rect_src->h : img->getheight(), blend_func);
}

inline void load_audio(LPCTSTR path, LPCTSTR id)
{
    static TCHAR str_cmd[512];
    _stprintf_s(str_cmd, _T("open %s alias %s"), path, id);
    mciSendString(str_cmd, NULL, 0, NULL);
}

inline void play_audio(LPCTSTR id, bool is_loop = false)
{
    static TCHAR str_cmd[512];
    _stprintf_s(str_cmd, _T("play %s %s from 0"), id, is_loop ? _T("repeat") : _T(""));
    mciSendString(str_cmd, NULL, 0, NULL);
}

inline void stop_audio(LPCTSTR id)
{
    static TCHAR str_cmd[512];
    _stprintf_s(str_cmd, _T("stop %s"), id);
    mciSendString(str_cmd, NULL, 0, NULL);
}

