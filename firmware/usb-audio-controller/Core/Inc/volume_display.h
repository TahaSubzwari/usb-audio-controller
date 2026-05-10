/**
  ******************************************************************************
  * @file           : volume_display.h
  * @brief          : Interface for LCD volume display functions.
  ******************************************************************************
  */
#ifndef VOLUME_DISPLAY_H
#define VOLUME_DISPLAY_H

void LCD_Init(void);
void DrawVolumeDial(int volume, int muted);

#endif
