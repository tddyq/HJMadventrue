#include "util.h"
#include "timer.h"
#include "atlas.h"
#include "vector2.h"

#include <vector>
#include <functional>

class Animation
{
public:
    Animation()
    {
        timer.set_one_shot(false);
        //timer.resume();
        timer.set_on_timeout(
            [&]()
            {
                idx_frame++;

                is_anim_change = true;

                if (idx_frame >= frame_list.size())
                {
                    idx_frame = is_loop ? 0 : frame_list.size() - 1;
                    if (!is_loop && on_finished)
                        on_finished();
                }
            });
    }

    ~Animation() = default;

    void reset()
    {
        timer.restart();
        idx_frame = 0;

    }

    void set_position(const Vector2& position)
    {
        //����
        std::cout << "���¶���λ��:" <<position.x<<" "<<position.y << std::endl;

        this->position = position;
    }

    void set_loop(bool is_loop)
    {
        this->is_loop = is_loop;
    }

    void set_interval(float interval)
    {
        timer.set_wait_time(interval);
    }

    void set_on_finished(std::function<void()> on_finished)
    {
        this->on_finished = on_finished;
    }

    void add_frame(IMAGE* image, int num_h)
    {
        int width = image->getwidth();
        int height = image->getheight();
        int width_frame = width / num_h;

        for (int i = 0; i < num_h; i++)
        {
            Rect rect_src;
            rect_src.x = i * width_frame, rect_src.y = 0;
            rect_src.w = width_frame, rect_src.h = height;

            frame_list.emplace_back(image, rect_src);
        }
    }

    void add_frame(Atlas* atlas)
    {
        for (int i = 0; i < atlas->get_size(); i++)
        {
            IMAGE* image = atlas->get_image(i);
            int width = image->getwidth();
            int height = image->getheight();

            Rect rect_src;
            rect_src.x = 0, rect_src.y = 0;
            rect_src.w = width, rect_src.h = height;

            frame_list.emplace_back(image, rect_src);
        }
    }

    void on_update(float delta)
    {
        
        timer.on_update(delta);
    }

    void on_render(const Camera& camera)
    {
       

        const Frame& frame = frame_list[idx_frame];

        Rect rect_dst;
        rect_dst.x = (int)position.x - frame.rect_src.w / 2;
        rect_dst.y = (int)position.y - frame.rect_src.h / 2;
        rect_dst.w = frame.rect_src.w, rect_dst.h = frame.rect_src.h;

        putimage_ex(camera, frame.image, &rect_dst, &frame.rect_src);
    }

     Rect getRenderRect(const Camera& camera) {
        const Frame& frame = frame_list[idx_frame];

        // ����������ת��Ϊ��Ļ���� (�ؼ��޸�)
        int screen_x = static_cast<int>(position.x - camera.get_position().x) - frame.rect_src.w / 2;
        int screen_y = static_cast<int>(position.y - camera.get_position().y) - frame.rect_src.h / 2;

        return {
            screen_x,
            screen_y,
            frame.rect_src.w,
            frame.rect_src.h
        };
    }

    bool checkIsChange() {
        if (is_anim_change) {
            is_anim_change = false;
            return true;
        }
        else {
            return false;
        }
    }
private:
    struct Frame
    {
        Rect rect_src;
        IMAGE* image = nullptr;

        Frame() = default;
        Frame(IMAGE* image, const Rect& rect_src)
            : image(image), rect_src(rect_src) {}
        ~Frame() = default;
    };

private:
    Timer timer;
    Vector2 position;
    bool is_loop = true;
    size_t idx_frame = 0;
    std::vector<Frame> frame_list;
    std::function<void()> on_finished;
private:
    bool is_anim_change = true;
};