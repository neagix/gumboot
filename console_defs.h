
#ifndef _CONSOLE_DEFS_H
#define _CONSOLE_DEFS_H


#define CONSOLE_CHAR_WIDTH 8
#define CONSOLE_CHAR_HEIGHT 16
#define CONSOLE_ROW_HEIGHT CONSOLE_CHAR_HEIGHT

#define RESOLUTION_W 640
#define RESOLUTION_H 480

#define CONSOLE_Y_OFFSET (CONSOLE_CHAR_HEIGHT-2)
#define CONSOLE_X_OFFSET  0

#define CONSOLE_WIDTH RESOLUTION_W
#define CONSOLE_LINES ((RESOLUTION_H-CONSOLE_Y_OFFSET)/CONSOLE_ROW_HEIGHT - 1)
#define CONSOLE_COLUMNS (CONSOLE_WIDTH/CONSOLE_CHAR_WIDTH)

#endif // _CONSOLE_DEFS_H
