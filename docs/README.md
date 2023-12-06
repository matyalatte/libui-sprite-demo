# libui-sprite-demo
Demo app to animate 2D sprites using [libui-ng](https://github.com/libui-ng/libui-ng).

https://github.com/matyalatte/libui-sprite-demo/assets/69258547/41914efa-f9a0-4766-a105-301dfda18dc7

> :warning: This demo uses [a forked branch](https://github.com/matyalatte/libui-ng/tree/image_buffer) for `uiImageBuffer`.
> The official libui-ng doesn't support it yet.

## How It Work

You can create image buffers with `uiNewImageBuffer` and `uiImageBufferUpdate`.  

```c
uiImageBuffer* image = uiNewImageBuffer(draw_context, width, height, has_alpha);
uiImageBufferUpdate(image, data);  // the size of data is width * height * 4

// You should free the buffer with uiFreeImageBuffer later
```

Then, you can draw it with `uiImageBufferDraw`.  
You can also apply affine transformation with `uiDrawTransform`.  

```c
uiRect src_rect;  // sprite area in the image buffer
double cx, cy;  // center point of the sprite
double x, y;  // coordinates of the center point in uiArea
double sx, sy;  // scale
double rad;  // rotation angle

uiDrawSave(draw_context);

uiDrawMatrix m;
uiDrawMatrixSetIdentity(&m);
uiDrawMatrixRotate(&m, x, y, rad);
uiDrawTransform(draw_context, &m);

uiRect dstrect = {
    (int)(x - cx * sx),
    (int)(y - cy * sy),
    (int)(src_rect.Width * sx),
    (int)(src_rect.Height * sy)
};
uiImageBufferDraw(draw_context, image, &src_rect, &dst_rect, 1);  // 1 means linear interpolation

uiDrawRestore(draw_context);  // reset matrix for other sprites
```


## Supported Platforms

-   Windows 7 or later  
-   macOS 10.9 or later  
-   Linux with GTK+ 3.10 or later  

## Building

There are platform-specific documents for building the executable with [Meson](https://github.com/mesonbuild/meson).  

-   [Building Workflow for Windows](./Build-on-Windows.md)  
-   [Building Workflow for macOS](./Build-on-Mac.md)  
-   [Building Workflow for Linux](./Build-on-Linux.md)  

## License

-   Code is licensed under the [MIT license](../LICENSE).
-   The project contains [libspng](https://github.com/randy408/libspng), it is licensed under the [BSD 2-clause "Simplified" License](https://github.com/randy408/libspng/blob/master/LICENSE).
-   PNG files in `./sprites` are in the public domain.  
    The original files are available here.  
    [Miami Synth | Ansimuz Games](https://ansimuz.com/site/portfolio/miami-synth/)
