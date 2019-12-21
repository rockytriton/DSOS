#include "rash.h"
#include "../../drivers/video/video.h"
#include "../../drivers/muart/muart.h"
#include <lib/dsstring.h>
#include "log.h"

typedef enum {
    UserInput,
    Exec,
    DrawPrompt
} RashState;

#define MAX_LINE 50

typedef struct {
    int xPos;
    int yPos;
    int lineNumber;
    int charNumber;
    char lineBuffer[255];
    char cwd[255];
    RashState state;
} RashInfo;

static RashInfo rashInfo = {
    .xPos = 0,
    .yPos = 0,
    .lineNumber = 0,
    .charNumber = 0,
    .state = UserInput
};

void rash_draw_string(char *sz);
void rash_exec(char *sz);

void rash_draw_prompt() {
    rashInfo.state = DrawPrompt;
    rash_draw_string("rash :> ");
    rashInfo.state = UserInput;
}

void rash_scroll() {
    rashInfo.lineNumber++;

    if (rashInfo.lineNumber > MAX_LINE) {
        rashInfo.lineNumber--;
        rashInfo.yPos -= (font_get_height() + 2);
        video_scroll(2);
    }
}

void rash_draw_char(char c) {
    if (c == 0) {
        return;
    }

    if (c == '\b') {
        rashInfo.xPos -= (font_get_width() + 2);
        video_draw_char(' ', rashInfo.xPos, rashInfo.yPos);
        rashInfo.charNumber--;
    }
    else if (c == '\r' || c == '\n') {
        rashInfo.xPos = 0;
        rashInfo.yPos += font_get_height() + 2;
        rashInfo.lineBuffer[rashInfo.charNumber] = 0;
        rash_scroll();

        if (rashInfo.state == UserInput && rashInfo.charNumber > 0) {
            char sz[255];
            str_copy(sz, rashInfo.lineBuffer);
            rashInfo.charNumber = 0;
            rash_exec(sz);
        }
    } else {
        video_draw_char(c, rashInfo.xPos, rashInfo.yPos);
        rashInfo.xPos += font_get_width() + 2;

        if (rashInfo.state == UserInput) {
            rashInfo.lineBuffer[rashInfo.charNumber] = c;
            rashInfo.charNumber++;
        }
    }
}

void rash_draw_string(char *sz) {
    for (int i=0; sz[i] != 0; i++) {
        rash_draw_char(sz[i]);
    }
}

void rash_main() {
    video_set_resolution(1920, 1280, 16);
    video_clear_screen(Black);
    video_set_text_color(White);
    rash_draw_prompt();

    str_copy(rashInfo.cwd, "/home/user");

    while(1) {
        char c = muart_read_char();
        log_println("RECV CHAR: %c - %2X", c, c);
        rash_draw_char(c);
    }
}


void rash_exec(char *sz) {
    rashInfo.state = Exec;

    if (str_equal(sz, "ls")) {
        rash_draw_string("Listing directory contents:\n");
        rash_draw_string(".\n");
        rash_draw_string("..\n");
    } else if (str_equal(sz, "pwd")) {
        rash_draw_string(rashInfo.cwd);
    } else if (str_equal(sz, "")) {
        
    } else {
        rash_draw_string("Unknown Command");
    }

    rashInfo.state = UserInput;
    rash_draw_string("\n");
    rash_draw_prompt();
}
