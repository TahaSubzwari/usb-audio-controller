/**
  ******************************************************************************
  * @file           : volume_display.c
  * @brief          : LCD volume dial rendering.
  *                   Provides LCD initialization and arc-style volume dial
  *                   drawing with partial redraw to avoid full framebuffer clears.
  ******************************************************************************
  */
#include "image.h"
#include "LCD_2inch.h"
#include "volume_display.h"

void LCD_Init()
{
	DEV_Module_Init();
	LCD_2IN_SetBackLight(1000);
	LCD_2IN_Init();
	Paint_NewImage(LCD_2IN_WIDTH,LCD_2IN_HEIGHT, ROTATE_180, BLACK);
	Paint_SetClearFuntion(LCD_2IN_Clear);
	Paint_SetDisplayFuntion(LCD_2IN_DrawPaint);
  
	Paint_Clear(BLACK);
	DEV_Delay_ms(100);
  
	Paint_DrawString_EN (70, 10, "Volume", &Font20, WHITE, RED);
	printf("LCD Initialized!\r\n");
}

void DrawVolumeDial(int volume, int prev_vol)
{
    UWORD xc = 120; // center for volume
    UWORD yc = 120;
    UWORD radius = 80;

    float start = -3.5;
    float end   = 0.35;
    int scaled = volume * 10;

    //Erase previous state
    if(prev_vol >= 0){
		// Erase old number — draw a filled black rectangle over it
		Paint_DrawRectangle(xc - 30, yc + 40, xc + 30, yc + 60, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    }

    //Background arc (grey)
    Paint_DrawArc(xc, yc, radius, start, end, GRAY, DOT_PIXEL_2X2);

    // Progress arc (filled)
    float progress = scaled / 1000.0f;
    float current = start + (end - start) * progress;

    Paint_DrawArc(xc, yc, radius, start, current, RED, DOT_PIXEL_2X2);

    //Volume
    char buf[10];
    sprintf(buf, "%d", volume);

    Paint_DrawString_EN(xc - 18, yc + 40, buf, &Font24, WHITE, WHITE);

    prev_vol = volume;

    return;
}

