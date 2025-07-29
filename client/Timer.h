#pragma once
#include <functional>

class Timer
{
public:
    Timer() = default;
    ~Timer() = default;

    void restart()
    {
        pass_time = 0;
        shotted = false;
    }

    void set_wait_time(float val)
    {
        wait_time = val;
    }

    void set_one_shot(bool flag)
    {
        one_shot = flag;
    }

    void set_on_timeout(std::function<void()> on_timeout)
    {
        this->on_timeout = on_timeout;
    }

    void pause()
    {
        paused = true;
    }

    void resume()
    {
        paused = false;
    }

    void on_update(float delta)
    {
        //调试
        std::cout << "定时器开始更新" << std::endl;

        if (paused) return;

        pass_time += delta;

        //调试
        std::cout << "定时器passtime: "<<pass_time<<" "<<"Timer delta: " << delta << std::endl;

        if (pass_time >= wait_time)
        {
            bool can_shot = (!one_shot || (one_shot && !shotted));
            shotted = true;
            if (can_shot && on_timeout)
                on_timeout();
            pass_time -= wait_time;
        }
    }

private:
    float pass_time = 0;
    float wait_time = 0;
    bool paused = false;
    bool shotted = false;
    bool one_shot = false;
    std::function<void()> on_timeout;
};





