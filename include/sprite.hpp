#pragma once
#include "ui.h"
#include "png_reader.hpp"

// wrapper for uiImageBuffer
class ImageBuffer {
 private:
    uiImageBuffer *m_image_buffer;
    int m_width;
    int m_height;
    int m_has_alpha;

 public:
    ImageBuffer() : m_image_buffer(NULL) {}

    ~ImageBuffer() {
        if (m_image_buffer)
            uiFreeImageBuffer(m_image_buffer);
    }

    int CreateFromPng(uiDrawContext *c, const char* file_name)
    {
        PngReader reader;
        int ret = reader.ReadFromFile(file_name);
        if (ret) return 1;
        reader.GetSize(&m_width, &m_height);
        m_has_alpha = reader.HasAlpha();
        m_image_buffer = uiNewImageBuffer(c, m_width, m_height, m_has_alpha);
        uiImageBufferUpdate(m_image_buffer, reader.GetData());
        return 0;
    }

    void GetSize(int *width, int *height)
    {
        *width = m_width;
        *height = m_height;
    }

    uiRect GetRect() {
        return { 0, 0, m_width, m_height };
    }

    int HasAlpha() { return m_has_alpha; }

    uiImageBuffer *GetLibuiBuffer() { return m_image_buffer; }
};

class Sprite {
 protected:
    uiImageBuffer *m_image_buffer;
    uiRect m_src_rect;  // sprite area in the image buffer
    double m_cx, m_cy;  // conter point of the sprite
    double m_x, m_y;  // coordinates of the center point in uiArea
    double m_sx, m_sy;  // scale
    double m_rad;  // rotation angle

 public:
    Sprite() : m_image_buffer(NULL),
               m_src_rect({ 0, 0, 0, 0 }),
               m_cx(0.0), m_cy(0.0),
               m_x(0.0), m_y(0.0),
               m_sx(1.0), m_sy(1.0),
               m_rad(0.0) {}

    void SetBuffer(ImageBuffer &buf) { m_image_buffer = buf.GetLibuiBuffer(); }
    void SetSrcRect(uiRect rect) { m_src_rect = rect; }
    void SetPosition(double x, double y) { m_x = x; m_y = y; }
    void SetCenter(double cx, double cy) { m_cx = cx; m_cy = cy; }
    void SetScale(double sx, double sy) { m_sx = sx; m_sy = sy; }
    void SetAngle(double rad) { m_rad = rad; }

    uiImageBuffer *GetLibuiBuffer() { return m_image_buffer; }
    uiRect GetSrcRect() { return m_src_rect; }
    void GetPosition(double *x, double *y) { *x = m_x; *y = m_y; }
    void GetCenter(double *cx, double *cy) { *cx = m_cx; *cy = m_cy; }
    void GetScale(double *sx, double *sy) { *sx = m_sx; *sy = m_sy; }
    double GetAngle() { return m_rad; }

    void Draw(uiDrawContext *c)
    {
        uiDrawSave(c);

        uiDrawMatrix rm;
        uiDrawMatrixSetIdentity(&rm);
        uiDrawMatrixRotate(&rm, m_x, m_y, m_rad);
        uiDrawTransform(c, &rm);

        uiRect dstrect = {
            (int)(m_x - m_cx * m_sx),
            (int)(m_y - m_cy * m_sy),
            (int)(m_src_rect.Width * m_sx),
            (int)(m_src_rect.Height * m_sy)
        };
        uiImageBufferDraw(c, m_image_buffer, &m_src_rect, &dstrect);

        uiDrawRestore(c);  // reset matrix for other sprites
    }
};
