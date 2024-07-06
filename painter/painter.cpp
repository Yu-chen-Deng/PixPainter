#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <stdio.h> // DEBUG时Console用
#include <string.h>
#include <windows.h>
#include <math.h>

// 画布在显示器上显示大小
#define WIDTH 800
#define HEIGHT 600
// 为突显像素画板的效果，软件中1个像素被放大10倍（也即是80x60画布）
#define PIXEL_SIZE 10
// Rewind(回顾)功能的定义
#define LEFT 0
#define RIGHT 1

// 颜色滑动条的定义
typedef struct
{
    int x, y;
    int width, height;
    int min, max;
    int value;
} Slider;

// 工具栏第一栏
IMAGE button_palette_color, button_palette_add, button_palette;
// 工具栏第二栏
IMAGE button_script, button_load, button_save, button_clear, button_line, button_circle, button_back, button_forward, button_web;
// 导入/保存功能用
IMAGE img;
// Rewind功能缓存
IMAGE tmp;
// 形状绘制时实时预览用
IMAGE drawing;
// RGB调色条
IMAGE rSimg, gSimg, bSimg;

// RGB调色条定义
Slider rSlider, gSlider, bSlider;

// 选色框
int colorPicker[8] = {BLACK, RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE};

int currentColor = BLACK;

bool eraserMode = false;

// 绘制模式
enum DrawMode
{
    DRAW_PENCIL,
    DRAW_LINE,
    DRAW_CIRCLE
};
DrawMode drawMode = DRAW_PENCIL;

// Rewind功能相关定义
bool rewindMode = false;
int rewindNum = 10; // 总共缓存多少张
int rewindIndex = 0;
int rewindSecond = 5; // 每隔几秒缓存一张

// 初始化画布
void drawGrid();

// 工具栏 & 选色栏部分
void reColorPicker(int *colorPicker, int size, int rgb_16);
void drawColorPicker();
void drawToolBar();

// RGB调色条部分
void initSlider(Slider *slider, int x, int y, int width, int height, int min, int max, int value);
void drawSlider(Slider *slider);
bool updateSlider(Slider *slider);

// 画布绘图部分
void handleMouseClick(int x, int y);
void line();
void circle();
void script();

// 画布操作部分
void clearCanvas();
void loadCanvas();
void saveCanvas();
void rewindCanvas(int choice);

int main()
{
    initgraph(WIDTH + 50, HEIGHT + 100); // DEBUG时:initgraph(WIDTH + 50, HEIGHT + 100, EX_SHOWCONSOLE)

    initSlider(&rSlider, 25, HEIGHT + 20, 200, 10, 0, 255, 0);
    initSlider(&gSlider, 250, HEIGHT + 20, 200, 10, 0, 255, 0);
    initSlider(&bSlider, 475, HEIGHT + 20, 200, 10, 0, 255, 0);

    loadimage(&rSimg, "./slider/r.png");
    loadimage(&gSimg, "./slider/g.png");
    loadimage(&bSimg, "./slider/b.png");

    clearCanvas();
    drawColorPicker();
    drawToolBar();

    IMAGE Canva;
    char tmpName[50];
    time_t lastRewindTime = time(NULL);

    while (true)
    {
        // 使用双缓冲
        BeginBatchDraw(); // 为什么要用缓存？防止闪屏现象的出现。

        while (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();

            if (msg.y >= 0 && msg.y <= HEIGHT && msg.x >= 0 && msg.x <= WIDTH + 50)
            {
                rewindMode = true;

                switch (msg.uMsg)
                {
                case WM_MOUSEMOVE: // 鼠标移动
                    switch (drawMode)
                    {
                    case DRAW_PENCIL:
                        if (msg.mkLButton)
                        {
                            eraserMode = false;
                            handleMouseClick(msg.x, msg.y);
                        }
                        else if (msg.mkRButton)
                        {
                            eraserMode = true;
                            handleMouseClick(msg.x, msg.y);
                        }
                        break;
                    case DRAW_LINE:
                        break;
                    case DRAW_CIRCLE:
                        break;
                    }
                    break;
                case WM_LBUTTONDOWN: // 左键点击
                    eraserMode = false;
                    handleMouseClick(msg.x, msg.y);
                    break;
                case WM_RBUTTONDOWN: // 右键点击
                    eraserMode = true;
                    handleMouseClick(msg.x, msg.y);
                    break;
                }
            }
            else if (msg.y > HEIGHT)
            {
                if (msg.x > 0 && msg.x < 700 && msg.y > HEIGHT && msg.y < 650)
                {
                    // 更新滑动条的值
                    // 绘制滑动条
                    if (msg.x < 230)
                    {
                        updateSlider(&rSlider);
                        drawSlider(&rSlider);
                    }
                    else if (msg.x < 455)
                    {
                        updateSlider(&gSlider);
                        drawSlider(&gSlider);
                    }
                    else if (msg.x < 700)
                    {
                        updateSlider(&bSlider);
                        drawSlider(&bSlider);
                    }

                    // 绘制当前颜色
                    setfillcolor(RGB(rSlider.value, gSlider.value, bSlider.value));
                    bar(710, HEIGHT + 10, 740, HEIGHT + 40);
                }

                // 检测是否需要改变颜色
                else if (msg.x > 750 && msg.x < WIDTH && msg.y > HEIGHT && msg.y < 650 && msg.uMsg == WM_LBUTTONDOWN) // 这里不能用msg.mkLButton
                {
                    reColorPicker(colorPicker, 8, RGB(rSlider.value, gSlider.value, bSlider.value));

                    IMAGE click_button_palette_add;

                    loadimage(&click_button_palette_add, "./button/click_button_palette_add.png");
                    putimage(750, HEIGHT + 1, &click_button_palette_add);

                    // 更新完立刻对缓存进行刷新，避免新图像不显示
                    FlushBatchDraw();

                    while (GetMouseMsg().mkLButton)
                    {
                    }

                    loadimage(&button_clear, "./button/button_palette_add.png");
                    putimage(750, HEIGHT + 1, &button_palette_add);

                    drawColorPicker();
                }
                else if (msg.x > 300 && msg.x < 350 && msg.y > HEIGHT + 51 && msg.y < 700 && msg.uMsg == WM_LBUTTONDOWN)
                {
                    script();
                }
                else if (msg.x > 350 && msg.x < 400 && msg.y > HEIGHT + 51 && msg.y < 700 && msg.uMsg == WM_LBUTTONDOWN)
                {
                    loadCanvas();
                }
                else if (msg.x > 400 && msg.x < 450 && msg.y > HEIGHT + 51 && msg.y < 700 && msg.uMsg == WM_LBUTTONDOWN)
                {
                    saveCanvas();
                }
                else if (msg.x > 450 && msg.x < 500 && msg.y > HEIGHT + 51 && msg.y < 700 && msg.uMsg == WM_LBUTTONDOWN)
                {
                    clearCanvas();
                }
                else if (msg.x > 500 && msg.x < 550 && msg.y > HEIGHT + 51 && msg.y < 700 && msg.uMsg == WM_LBUTTONDOWN)
                {
                    line();
                }
                else if (msg.x > 550 && msg.x < 600 && msg.y > HEIGHT + 51 && msg.y < 700 && msg.uMsg == WM_LBUTTONDOWN)
                {
                    circle();
                }
                else if (msg.x > 600 && msg.x < 650 && msg.y > HEIGHT + 51 && msg.y < 700 && msg.uMsg == WM_LBUTTONDOWN)
                {
                    rewindCanvas(LEFT);
                }
                else if (msg.x > 650 && msg.x < 700 && msg.y > HEIGHT + 51 && msg.y < 700 && msg.uMsg == WM_LBUTTONDOWN)
                {
                    rewindCanvas(RIGHT);
                }
            }
        }

        // 刷新屏幕
        EndBatchDraw();

        // 缓存图片
        time_t currentTime = time(NULL);
        // 确保符合时间间隔要求，又因为while循环，避免一次性保存多张，引入了lastrewindtime。
        if (currentTime % rewindSecond == 0 && currentTime > lastRewindTime && rewindMode == true)
        {
            lastRewindTime = currentTime;

            rewindMode = false;

            // 如果缓存数量没达到上限
            if (rewindIndex < rewindNum)
            {
                getimage(&Canva, 0, 0, WIDTH, HEIGHT);

                sprintf(tmpName, "%s%d%s", "./tmp/tmp", rewindIndex, ".png");

                saveimage((LPCTSTR)tmpName, &Canva);

                rewindIndex += 1;
            }
            // 如果缓存数量达到上限
            else
            {
                char oldname[50], newname[50];

                rewindIndex = rewindNum - 1;

                // 删除最老的缓存
                remove("./tmp/tmp0.png");

                for (int i = 1; i < rewindNum; i++)
                {
                    sprintf(oldname, "%s%d%s", "./tmp/tmp", i, ".png");
                    sprintf(newname, "%s%d%s", "./tmp/tmp", i - 1, ".png");
                    rename(oldname, newname);
                }

                // 获取最新的画布，并保存为最新的缓存
                getimage(&Canva, 0, 0, WIDTH, HEIGHT);
                sprintf(tmpName, "%s%d%s", "./tmp/tmp", rewindIndex, ".png");
                saveimage((LPCTSTR)tmpName, &Canva);
            }
        }

        // 减少刷新率来增加流畅性
        Sleep(1);
    }

    closegraph();

    return 0;
}

void drawGrid()
{
    // 画网格
    for (int i = 0; i < WIDTH; i += PIXEL_SIZE)
    {
        line(i, 0, i, HEIGHT);
    }
    for (int j = 0; j < HEIGHT; j += PIXEL_SIZE)
    {
        line(0, j, WIDTH, j);
    }
}
// 更新选色栏数组
void reColorPicker(int *colorPicker, int size, int rgb_16)
{
    if (colorPicker[0] != rgb_16)
    {
        for (int i = (size - 1); i > 0; i--)
        {
            colorPicker[i] = colorPicker[i - 1];
        }
        colorPicker[0] = rgb_16;
    }
}
// 画选色栏
void drawColorPicker()
{

    for (int i = 0; i < 8; i++)
    {
        setfillcolor(colorPicker[i]);
        bar(WIDTH, i * 50, WIDTH + 50, (i + 1) * 50);
    }

    setfillcolor(LIGHTGRAY);
    solidrectangle(WIDTH, 400, WIDTH + 50, 600);
    setfillcolor(currentColor);
    solidrectangle(WIDTH + 5, 405, WIDTH + 45, 595);
    setfillcolor(LIGHTGRAY);
    solidrectangle(WIDTH + 10, 410, WIDTH + 40, 590);
}

void drawToolBar()
{
    loadimage(&button_script, "./button/button_script.png");
    loadimage(&button_load, "./button/button_load.png");
    loadimage(&button_save, "./button/button_save.png");
    loadimage(&button_clear, "./button/button_clear.png");
    loadimage(&button_line, "./button/button_line.png");
    loadimage(&button_circle, "./button/button_circle.png");
    loadimage(&button_back, "./button/button_back.png");
    loadimage(&button_forward, "./button/button_forward.png");
    loadimage(&button_web, "./button/button_web.png");

    setfillcolor(0x7F7F7F);
    solidrectangle(0, HEIGHT + 1, WIDTH + 50, HEIGHT + 100);

    // 文本框区域
    setfillcolor(DARKGRAY);
    solidrectangle(4, HEIGHT + 52, 296, HEIGHT + 98);
    setfillcolor(LIGHTGRAY);
    solidrectangle(7, HEIGHT + 55, 293, HEIGHT + 95);
    setfillcolor(WHITE);
    solidrectangle(10, HEIGHT + 58, 290, HEIGHT + 92);

    putimage(300, HEIGHT + 51, &button_script);
    putimage(350, HEIGHT + 51, &button_load);
    putimage(400, HEIGHT + 51, &button_save);
    putimage(450, HEIGHT + 51, &button_clear);
    putimage(500, HEIGHT + 51, &button_line);
    putimage(550, HEIGHT + 51, &button_circle);
    putimage(600, HEIGHT + 51, &button_back);
    putimage(650, HEIGHT + 51, &button_forward);
    putimage(800, HEIGHT + 51, &button_web);

    // 颜色预览区块和颜色添加区块
    loadimage(&button_palette_color, "./button/button_palette_color.png");
    loadimage(&button_palette_add, "./button/button_palette_add.png");
    loadimage(&button_palette, "./button/button_palette.png");

    putimage(700, HEIGHT + 1, &button_palette_color);
    putimage(750, HEIGHT + 1, &button_palette_add);
    putimage(800, HEIGHT + 1, &button_palette);

    // 绘制滑动条
    drawSlider(&rSlider);
    drawSlider(&gSlider);
    drawSlider(&bSlider);

    // 绘制当前颜色
    setfillcolor(RGB(rSlider.value, gSlider.value, bSlider.value));
    bar(709, HEIGHT + 9, 742, HEIGHT + 42);
}

// 初始化滑动条
void initSlider(Slider *slider, int x, int y, int width, int height, int min, int max, int value)
{
    slider->x = x;
    slider->y = y;
    slider->width = width;
    slider->height = height;
    slider->min = min;
    slider->max = max;
    slider->value = value;
}

// 绘制滑动条
void drawSlider(Slider *slider)
{
    // 滑动条背景
    // setfillcolor(LIGHTGRAY);
    // bar(slider->x, slider->y, slider->x + slider->width, slider->y + slider->height);
    if (slider->x == 25)
    {
        putimage(25, HEIGHT + 20, &rSimg);
    }
    else if (slider->x == 250)
    {
        putimage(250, HEIGHT + 20, &gSimg);
    }
    else
    {
        putimage(475, HEIGHT + 20, &bSimg);
    }

    // 滑动块
    int slider_pos = slider->x + (slider->value - slider->min) * slider->width / (slider->max - slider->min);
    setfillcolor(WHITE);
    solidcircle(slider_pos, slider->y + slider->height / 2, slider->height / 2);
}

// 检查鼠标事件，更新滑动条值
bool updateSlider(Slider *slider)
{
    bool changed = false;
    if (MouseHit()) // 使用 MouseHit 检查是否有鼠标事件
    {
        ExMessage msg = getmessage(EM_MOUSE); // 获取鼠标事件

        if (msg.message == WM_LBUTTONDOWN || (msg.message == WM_MOUSEMOVE && msg.lbutton)) // 检查左键按下或左键按住并移动事件
        {
            if (msg.x >= slider->x && msg.x <= slider->x + slider->width &&
                msg.y >= slider->y && msg.y <= slider->y + slider->height)
            {
                int newValue = slider->min + (msg.x - slider->x) * (slider->max - slider->min) / slider->width;
                if (newValue != slider->value)
                {
                    slider->value = newValue;
                    changed = true;
                }
            }
        }
    }
    return changed;
}

void handleMouseClick(int x, int y)
{
    if (x >= WIDTH || y >= HEIGHT)
    {
        int colorIndex = y / 50;
        if (colorIndex >= 0 && colorIndex < 8)
        {
            currentColor = colorPicker[colorIndex];
            drawColorPicker();
            eraserMode = false;
        }
    }
    else
    {
        int gridX = (x / PIXEL_SIZE) * PIXEL_SIZE;
        int gridY = (y / PIXEL_SIZE) * PIXEL_SIZE;

        switch (drawMode)
        {
        case DRAW_PENCIL:
            setfillcolor(eraserMode ? WHITE : currentColor);
            bar(gridX, gridY, gridX + PIXEL_SIZE, gridY + PIXEL_SIZE);
            break;

        case DRAW_LINE:
            drawMode = DRAW_PENCIL;
            int endX, endY, deltaX, deltaY, tick;
            float k;

            getimage(&img, 0, 0, WIDTH, HEIGHT);

            while (GetMouseMsg().mkLButton)
            {
                putimage(0, 0, &img);

                endX = GetMouseMsg().x;
                endY = GetMouseMsg().y;

                deltaX = endX - x;
                tick = deltaX / abs(deltaX);
                deltaY = endY - y;

                k = (float)deltaY / (float)deltaX;

                for (int tempX = x; abs(tempX - endX) > 1 * PIXEL_SIZE; tempX += (tick * PIXEL_SIZE))
                {
                    int gridX = (tempX / PIXEL_SIZE) * PIXEL_SIZE;
                    int gridY = ((y + k * (tempX - x)) / PIXEL_SIZE) * PIXEL_SIZE;

                    setfillcolor(currentColor);
                    bar(gridX, gridY, gridX + PIXEL_SIZE, gridY + PIXEL_SIZE);
                }

                getimage(&drawing, 0, 0, WIDTH, HEIGHT);
                FlushBatchDraw();

                if (GetMouseMsg().uMsg == WM_LBUTTONUP)
                {
                    putimage(0, 0, &drawing);
                    break;
                }

                Sleep(10);
            }

            drawMode = DRAW_LINE;
            break;

        case DRAW_CIRCLE:
            drawMode = DRAW_PENCIL;
            int radius;
            getimage(&img, 0, 0, WIDTH, HEIGHT);

            while (GetMouseMsg().mkLButton)
            {
                putimage(0, 0, &img);

                endX = GetMouseMsg().x;
                endY = GetMouseMsg().y;

                deltaX = endX - x;
                deltaY = endY - y;

                radius = sqrt(deltaX * deltaX + deltaY * deltaY) / PIXEL_SIZE;

                for (int i = -radius; i <= radius; i++)
                {
                    for (int j = -radius; j <= radius; j++)
                    {
                        if (i * i + j * j <= radius * radius)
                        {
                            int gridX = ((x / PIXEL_SIZE) + i) * PIXEL_SIZE;
                            int gridY = ((y / PIXEL_SIZE) + j) * PIXEL_SIZE;
                            setfillcolor(currentColor);
                            bar(gridX, gridY, gridX + PIXEL_SIZE, gridY + PIXEL_SIZE);
                        }
                    }
                }

                getimage(&drawing, 0, 0, WIDTH, HEIGHT);
                FlushBatchDraw();

                if (GetMouseMsg().uMsg == WM_LBUTTONUP)
                {
                    putimage(0, 0, &drawing);
                    break;
                }

                Sleep(10);
            }

            drawMode = DRAW_CIRCLE;
            break;
        }
    }
}

void line()
{
    if (drawMode != DRAW_LINE)
    {
        drawMode = DRAW_LINE;

        IMAGE click_button_line;
        loadimage(&click_button_line, "./button/click_button_line.png");

        drawToolBar();
        putimage(500, HEIGHT + 51, &click_button_line);
    }
    else
    {
        drawMode = DRAW_PENCIL;
        drawToolBar();
    }
}

void circle()
{
    if (drawMode != DRAW_CIRCLE)
    {
        drawMode = DRAW_CIRCLE;

        IMAGE click_button_circle;
        loadimage(&click_button_circle, "./button/click_button_circle.png");

        drawToolBar();
        putimage(550, HEIGHT + 51, &click_button_circle);
    }
    else
    {
        drawMode = DRAW_PENCIL;
        drawToolBar();
    }
}

void script()
{
    IMAGE click_button_script;

    loadimage(&click_button_script, "./button/click_button_script.png");
    putimage(300, HEIGHT + 51, &click_button_script);
    FlushBatchDraw();

    while (GetMouseMsg().mkLButton)
    {
    }

    loadimage(&button_clear, "./button/button_script.png");
    putimage(300, HEIGHT + 51, &button_script);

    // 获取可执行文件所在目录
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // 找到最后一个反斜杠的位置
    char *lastSlash = strrchr(exePath, '\\');
    if (lastSlash != NULL)
    {
        // 替换为批处理文件名
        strcpy(lastSlash + 1, "script.bat");
    }

    // 打印批处理文件路径用于调试
    printf("Batch file path: %s\n", exePath);

    // 以管理员权限运行批处理脚本
    ShellExecuteA(NULL, "runas", exePath, NULL, NULL, SW_SHOWNORMAL);
}

void clearCanvas()
{
    IMAGE click_button_clear;

    loadimage(&click_button_clear, "./button/click_button_clear.png");
    putimage(450, HEIGHT + 51, &click_button_clear);
    FlushBatchDraw();

    while (GetMouseMsg().mkLButton)
    {
    }

    loadimage(&button_clear, "./button/button_clear.png");
    putimage(450, HEIGHT + 51, &button_clear);

    setfillcolor(WHITE);
    solidrectangle(0, 0, WIDTH, HEIGHT); // 固体长方形
    drawGrid();
}

void loadCanvas()
{
    IMAGE click_button_load;

    loadimage(&click_button_load, "./button/click_button_load.png");
    putimage(350, HEIGHT + 51, &click_button_load);
    FlushBatchDraw();

    while (GetMouseMsg().mkLButton)
    {
    }

    loadimage(&button_load, "./button/button_load.png");
    putimage(350, HEIGHT + 51, &button_load);

    loadimage(&img, "canvas.png");
    putimage(0, 0, &img);
}

void saveCanvas()
{
    IMAGE click_button_save;

    loadimage(&click_button_save, "./button/click_button_save.png");
    putimage(400, HEIGHT + 51, &click_button_save);
    FlushBatchDraw();

    while (GetMouseMsg().mkLButton)
    {
    }

    loadimage(&button_load, "./button/button_save.png");
    putimage(400, HEIGHT + 51, &button_save);

    getimage(&img, 0, 0, WIDTH, HEIGHT);
    saveimage("canvas.png", &img);
}

void rewindCanvas(int choice)
{
    char tmpName[50];

    if (choice == LEFT)
    {
        IMAGE click_button_back;

        loadimage(&click_button_back, "./button/click_button_back.png");
        putimage(600, HEIGHT + 51, &click_button_back);
        FlushBatchDraw();

        while (GetMouseMsg().mkLButton)
        {
        }

        if (rewindIndex > 0)
        {
            rewindIndex -= 1;
        }

        sprintf(tmpName, "%s%d%s", "./tmp/tmp", rewindIndex, ".png");

        loadimage(&tmp, tmpName);
        putimage(0, 0, &tmp);

        loadimage(&button_back, "./button/button_back.png");
        putimage(600, HEIGHT + 51, &button_back);
    }
    else if (choice == RIGHT)
    {
        IMAGE click_button_forward;

        loadimage(&click_button_forward, "./button/click_button_forward.png");
        putimage(650, HEIGHT + 51, &click_button_forward);
        FlushBatchDraw();

        while (GetMouseMsg().mkLButton)
        {
        }

        if (rewindIndex < rewindNum - 1)
        {
            rewindIndex += 1;
        }

        sprintf(tmpName, "%s%d%s", "./tmp/tmp", rewindIndex, ".png");

        loadimage(&tmp, tmpName);
        putimage(0, 0, &tmp);

        loadimage(&button_forward, "./button/button_forward.png");
        putimage(650, HEIGHT + 51, &button_forward);
    }
}
