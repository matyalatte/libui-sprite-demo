#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <time.h>
#include "ui.h"
#include "sprite.hpp"
#include "env_utils.hpp"  // GetExecutablePath(), SetCwd(), GetDirectory()

// TODO clean up the dirty code

class SpriteHandler {
 private:
    std::vector<ImageBuffer> m_image_buffers;
    std::vector<Sprite> m_sprites;
    std::string m_error_msg;
    PngReader m_png;
    int m_step;
    clock_t m_start;
    int m_start_step;
    uiSpinbox *m_spinbox_buffer;
    uiSpinbox *m_spinbox_sprite;
    uiCheckbox *m_checkbox_fast;
    uiLabel *m_label_fps;

 public:
    SpriteHandler() : m_image_buffers(), m_sprites(),
                      m_png(), m_error_msg(), m_step(0),
                      m_start(clock()), m_start_step(0) {}

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
        // load image
        int ret = m_png.ReadFromFile("sprites/palm-tree.png");
        if (ret) {
            m_error_msg = std::string("File not found. (sprites/palm-tree.png)");
            m_image_buffers.clear();
            return 1;
        }

        int width, height, has_alpha;
        m_png.GetSize(&width, &height);
        has_alpha = m_png.HasAlpha();

        m_image_buffers.resize(1);
        ImageBuffer& buf = m_image_buffers[0];
        buf.Create(c, width, height, has_alpha);
        buf.Update(m_png.GetData());

        m_sprites.resize(14400);
        for (int i = 0; i < 14400; i++) {
            Sprite &sprite = m_sprites[i];
            sprite.SetBuffer(buf);
            uiRect rect = buf.GetRect();
            sprite.SetSrcRect(rect);
            sprite.SetPosition(5 * (i / 120), 5 * (i % 120));
            sprite.SetCenter(width / 2, height / 2);
            sprite.SetScale(2.0, 2.0);
        }

        return 0;
    }

    void Update()
    {
        if (HasError()) return;
        int i = 0;
        int num = uiSpinboxValue(m_spinbox_buffer);
        ImageBuffer& buf = m_image_buffers[0];
        for (int i = 0; i < num; i++) {
            buf.Update(m_png.GetData());
        }
    }

    void Step()
    {
        int i = 0;
        int num = uiSpinboxValue(m_spinbox_sprite);
        for (auto &sprite : m_sprites) {
            if (i > num) break;
            sprite.SetAngle((double)(m_step % 200) * uiPi / 100);
            i++;
        }
        m_step++;
    }

    void DrawSprites(uiDrawContext *c)
    {
        if (HasError()) return;
        int i = 0;
        int num = uiSpinboxValue(m_spinbox_sprite);
        if (uiCheckboxChecked(m_checkbox_fast)) {
            for (auto &sprite : m_sprites) {
                if (i > num) break;
                sprite.DrawFast(c);
                i++;
            }
        } else {
            for (auto &sprite : m_sprites) {
                if (i > num) break;
                sprite.Draw(c);
                i++;
            }
        }
    }

    void CreateControls(uiBox *vbox)
    {
        m_label_fps = uiNewLabel("FPS: 0");
        uiBoxAppend(vbox, uiControl(m_label_fps), 0);

        m_spinbox_buffer = uiNewSpinbox(0, 14400);
        uiBoxAppend(vbox, uiControl(m_spinbox_buffer), 0);

        m_spinbox_sprite = uiNewSpinbox(1, 14400);
        uiBoxAppend(vbox, uiControl(m_spinbox_sprite), 0);

        m_checkbox_fast = uiNewCheckbox("Use uiImageBufferDrawFast()");
        uiBoxAppend(vbox, uiControl(m_checkbox_fast), 0);
    }

    void CheckFPS()
    {
        clock_t current = clock();
        if ((current - m_start) >= 2000) {
            m_start = current;
            m_step %= 200;
            m_start_step = m_step;
        }
        if ((current - m_start) >= 1000) {
            m_start += 1000;
            int fps = m_step - m_start_step;
            std::string fps_str = "FPS: " + std::to_string(fps);
            uiLabelSetText(m_label_fps, fps_str.c_str());
            m_step %= 200;
            m_start_step = m_step;
        }
    }
};

uiAreaHandler g_handler;
SpriteHandler g_sprite_handler;

// helper to quickly set a brush color
static void SetSolidBrush(uiDrawBrush *brush, uint32_t color, double alpha)
{
    uint8_t component;

    brush->Type = uiDrawBrushTypeSolid;
    component = (uint8_t) ((color >> 16) & 0xFF);
    brush->R = ((double) component) / 255;
    component = (uint8_t) ((color >> 8) & 0xFF);
    brush->G = ((double) component) / 255;
    component = (uint8_t) (color & 0xFF);
    brush->B = ((double) component) / 255;
    brush->A = alpha;
}

// This will be called by uiAreaQueueRedrawAll and uiControlShow
static void HandlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
    if (g_sprite_handler.HasError()) return;  // When failed to load sprites

    if (!g_sprite_handler.HasImage()) {
        // Load sprites when this handler is called by uiControlShow()
        g_sprite_handler.LoadSprites(p->Context);
        if (g_sprite_handler.HasError()) return;
    }

    g_sprite_handler.Step();
    g_sprite_handler.Update();

    // fill the area
    uiDrawPath *path;
    uiDrawBrush brush;
    SetSolidBrush(&brush, 0xEEEEEE, 1.0);
    path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawPathAddRectangle(path, 0, 0, p->AreaWidth, p->AreaHeight);
    uiDrawPathEnd(path);
    uiDrawFill(p->Context, path, &brush);
    uiDrawFreePath(path);

    // draw sprites
    g_sprite_handler.DrawSprites(p->Context);
}

static void HandlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
    // do nothing
}

static void HandlerMouseCrossed(uiAreaHandler *ah, uiArea *a, int left)
{
    // do nothing
}

static void HandlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
    // do nothing
}

static int HandlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
    // reject all keys
    return 0;
}

static int OnClosing(uiWindow *w, void *data)
{
    uiQuit();
    return 1;
}

static int OnShouldQuit(void *data)
{
    uiWindow *mainwin = uiWindow(data);
    uiControlDestroy(uiControl(mainwin));
    return 1;
}

static int OnAnimating(void *data)
{
    g_sprite_handler.CheckFPS();
    uiAreaQueueRedrawAll(uiArea(data));
    return 1;
}

void CreateWindow()
{
    // Main window
    uiWindow* mainwin = uiNewWindow("libui sprites demo", 600, 600, 1);
    uiWindowOnClosing(mainwin, OnClosing, NULL);
    uiOnShouldQuit(OnShouldQuit, mainwin);
    uiWindowSetMargined(mainwin, 0);

    // Main container
    uiBox *vbox = uiNewVerticalBox();
    uiBoxSetPadded(vbox, 1);
    uiWindowSetChild(mainwin, uiControl(vbox));

    // Drawing area
    g_handler.Draw = HandlerDraw;
    g_handler.MouseEvent = HandlerMouseEvent;
    g_handler.MouseCrossed = HandlerMouseCrossed;
    g_handler.DragBroken = HandlerDragBroken;
    g_handler.KeyEvent = HandlerKeyEvent;

    uiArea *area = uiNewArea(&g_handler);
    uiBoxAppend(vbox, uiControl(area), 1);

    // Call OnAnimating every 1ms to move sprites
    uiTimer(1, OnAnimating, area);

    // Spinbox and checkbox.
    g_sprite_handler.CreateControls(vbox);

    // Make them visible and call HandlerDraw() to load sprites
    uiControlShow(uiControl(mainwin));

    if (g_sprite_handler.HasError()) {
        uiMsgBoxError(mainwin, "Failed to load sprites.", g_sprite_handler.GetErrorMsg());
    }
}

int main(void)
{
    // Initialize libui
    uiInitOptions options;
    const char *err;

    memset(&options, 0, sizeof (uiInitOptions));
    err = uiInit(&options);
    if (err != NULL) {
        fprintf(stderr, "error initializing libui: %s", err);
        uiFreeInitError(err);
        return 1;
    }

    // Change cwd to exe path to read image files
    std::string exe_path = GetExecutablePath();
    SetCwd(GetDirectory(exe_path));

    // Craete main window
    CreateWindow();

    // Start main loop
    uiMain();

    return 0;
}
