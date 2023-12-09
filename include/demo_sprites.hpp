#pragma once
#include <vector>
#include <array>
#include <string>
#include "ui.h"
#include "sprite.hpp"

enum IMAGE_INDEX : int {
    IMAGE_CAR = 0,
    IMAGE_BACK,
    IMAGE_BUILDINGS,
    IMAGE_HIGHWAY,
    IMAGE_PALMS,
    IMAGE_PALMTREE,
    IMAGE_COUNT
};

const char* IMAGE_FILES[IMAGE_COUNT] = {
    "sprites/car-running.png",
    "sprites/back.png",
    "sprites/buildings.png",
    "sprites/highway.png",
    "sprites/palms.png",
    "sprites/palm-tree.png"
};

// Sprites that just move horizontally
class ScrollSprite : public Sprite {
 private:
    double m_speed;
    double m_current;
    double m_start;
    double m_length;

 public:
    ScrollSprite() : Sprite() {}

    void SetAnimation(double speed, double start, double length)
    {
        m_speed = speed;
        m_start = start;
        m_current = 0;
        m_length = length;
        m_x = m_start;
    }

    void Move() {
        m_current = std::fmod(m_current + m_speed, m_length);
        m_x = m_start + m_current;
    }
};

class Car : public Sprite {
 private:
    int m_count;
    double m_target_x;
    double m_rad_speed;

 public:
    Car() : Sprite() {}

    void Initialize(ImageBuffer &buf)
    {
        SetBuffer(buf);

        // There are four sprites in the image buffer.
        // So, we dont need the whole image.
        uiRect rect = buf.GetRect();
        rect.Height /= 4;
        SetSrcRect(rect);

        SetCenter(92, 0);
        SetPosition(200, 166);
        m_count = 0;
        m_target_x = 200;
        m_rad_speed = 0;
    }

    void Animate()
    {
        m_count = (m_count + 1) % 20;

        // There are four sprites for car animation.
        // They are alinged vertically in the image buffer.
        // So, we can animate it by changing y-coordinate.
        m_src_rect.Y = m_src_rect.Height * (m_count / 5);

        // Move a bit vertically in uiArea.
        m_y = 165 + (m_count / 5 % 2);

        // Move horizontally to the target point.
        if (std::abs(m_target_x - m_x) > 2.0) {
            if (m_target_x > m_x)
                m_x += 2.0;
            else
                m_x -= 2.0;
        }

        m_rad = std::fmod(m_rad + m_rad_speed, 2 * uiPi);
    }

    void SetTargetX(double mouse_x)
    {
        if (mouse_x < 120)
            m_target_x = 120;
        else if (mouse_x > 690)
            m_target_x = 690;
        else
            m_target_x = mouse_x;
    }

    void Rotate()
    {
        m_rad_speed = 0.1;
    }

    void ResetRotation()
    {
        m_rad = 0;
        m_rad_speed = 0;
    }
};

class DemoSpriteHandler {
 private:
    std::vector<ImageBuffer> m_image_buffers;
    Car m_car;
    std::vector<ScrollSprite> m_scroll_sprites;

    std::string m_error_msg;

 public:
    DemoSpriteHandler() : m_image_buffers(), m_car(), m_scroll_sprites(), m_error_msg() {}

    const char* GetImageFileName(int image_id) {
        return IMAGE_FILES[image_id];
    }

    int HasImage() {
        return m_image_buffers.size() > 0;
    }

    int HasError() {
        return m_error_msg.length() != 0;
    }

    const char* GetErrorMsg() {
        return m_error_msg.c_str();
    }

    int LoadSprites(uiDrawContext *c)
    {
        // load images
        m_image_buffers.resize(IMAGE_COUNT);
        int ret = 0;
        for (int i = 0; i < IMAGE_COUNT; i++) {
            ret = m_image_buffers[i].CreateFromPng(c, IMAGE_FILES[i]);
            if (ret) {
                m_error_msg = std::string("File not found. (") + GetImageFileName(i) + ")";
                break;
            }
        }

        if (ret) {
            m_image_buffers.clear();
            return 1;
        }

        // create the car sprite
        {
            ImageBuffer &buf = m_image_buffers[IMAGE_CAR];
            m_car.Initialize(buf);
        }

        struct Queue {
            int image_id;
            double x, y;
            double speed;
            double move_length;
        };

        // image id, x, y, speed, move length
        std::vector<Queue> queues = {
            { IMAGE_BACK, 0, 0, -0.5, 224 },
            { IMAGE_BACK, 224, 0, -0.5, 224 },
            { IMAGE_BACK, 448, 0, -0.5, 224 },
            { IMAGE_BACK, 672, 0, -0.5, 224 },
            { IMAGE_BACK, 896, 0, -0.5, 224 },
            { IMAGE_BUILDINGS, 0, 12, -1, 256 },
            { IMAGE_BUILDINGS, 256, 12, -1, 256 },
            { IMAGE_BUILDINGS, 512, 12, -1, 256 },
            { IMAGE_BUILDINGS, 768, 12, -1, 256 },
            { IMAGE_BUILDINGS, 1024, 12, -1, 256 },
            { IMAGE_PALMS, 0, 16, -5, 224 },
            { IMAGE_PALMS, 224, 16, -5, 224 },
            { IMAGE_PALMS, 448, 16, -5, 224 },
            { IMAGE_PALMS, 672, 16, -5, 224 },
            { IMAGE_PALMS, 896, 16, -5, 224 },
            { IMAGE_HIGHWAY, 0, 20, -10, 896 },
            { IMAGE_HIGHWAY, 896, 20, -10, 896 },
            { IMAGE_PALMTREE, 1120, 52, -10, 2240 },
        };

        // Create back ground sprites
        m_scroll_sprites.resize(queues.size());
        for (int i = 0; i < queues.size(); i++) {
            Queue q = queues[i];
            ImageBuffer &buf = m_image_buffers[q.image_id];
            ScrollSprite &sprite = m_scroll_sprites[i];
            sprite.SetBuffer(buf);
            uiRect rect = buf.GetRect();
            sprite.SetSrcRect(rect);
            sprite.SetPosition(q.x, q.y);
            sprite.SetAnimation(q.speed, q.x, q.move_length);
        }

        return 0;
    }

    void DrawSprites(uiDrawContext *c)
    {
        if (HasError()) return;
        int i;
        for (i = 0; i < m_scroll_sprites.size() - 1; i++) {
            m_scroll_sprites[i].Draw(c);
        }
        m_car.Draw(c);
        m_scroll_sprites[i].Draw(c);
    }

    void MoveSprites()
    {
        if (HasError()) return;
        for (ScrollSprite &s : m_scroll_sprites) {
            s.Move();
        }
        m_car.Animate();
    }

    void SetCarTargetX(double mouse_x)
    {
        m_car.SetTargetX(mouse_x);
    }

    void RotateCar()
    {
        m_car.Rotate();
    }

    void ResetCarRotation()
    {
        m_car.ResetRotation();
    }
};
