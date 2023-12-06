#include <stdio.h>
#include <string.h>
#include <cmath>
#include "ui.h"
#include "png_reader.hpp"
#include "sprite.hpp"
#include "demo_sprites.hpp"
#include "env_utils.hpp"

uiAreaHandler g_handler;
DemoSprites g_sprites;

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

// This will be called by uiAreaQueueRedrawAll
static void HandlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
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
    g_sprites.DrawSprites(p->Context);
}

static void HandlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
    // Move car to mouse
    g_sprites.SetCarTargetX(e->X);
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
    g_sprites.MoveSprites();
    uiAreaQueueRedrawAll(uiArea(data));
    return 1;
}

void CreateWindow()
{
    // Main window
    uiWindow* mainwin = uiNewWindow("libui sprites demo", 800, 256, 1);
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

    uiTimer(10, OnAnimating, area);

    // Load sprites
    int ret = g_sprites.LoadSprites(area);
    if (ret) {
        uiMsgBoxError(mainwin, "Failed to load sprites.", g_sprites.GetErrorMsg());
    }

    // Make them visible
    uiControlShow(uiControl(mainwin));
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
    SetCwd(exe_path);

    // Craete main window
    CreateWindow();

    // Start main loop
    uiMain();

    return 0;
}
