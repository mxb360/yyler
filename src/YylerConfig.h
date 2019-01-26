#ifndef __YYLER_CONFIG_H__
#define __YYLER_CONFIG_H__

#define YYLER_VERSION  "Version 1.0"

/* YylerPlayer */
#define PLAYER_WIDTH              71
#define PLAYER_HEIGHT             18
#define PLAYER_TITLE_SIZE         50
#define PLAYER_TITLE_X            7
#define PLAYER_TITLE_Y            4
#define PLAYER_TITLE_COLOR        COLOR_LIGHT_AQUA
#define PLAYER_BAR_X              4
#define PLAYER_BAR_Y              (PLAYER_HEIGHT - 1)
#define PLAYER_BAR_COLOR_1        COLOR_LIGHT_AQUA
#define PLAYER_BAR_COLOR_2        COLOR_LIGHT_BLUE
#define PLAYER_TIME_COLOR         COLOR_LIGHT_GREEN
#define PLAYER_ERR_X              0
#define PLAYER_ERR_Y              (PLAYER_HEIGHT + 2)
#define PLAYER_PROMPT_TIME_CNT    10
#define PLAYER_PROMPT_COLOR       COLOR_LIGHT_YELLOW
#define PLAYER_PROMPT_X           0
#define PLAYER_PROMPT_Y           (PLAYER_HEIGHT + 2)
#define PLAYER_DEFAULT_VOLUME     20
#define PLAYER_LYRIC_X            3
#define PLAYER_LYRIC_Y            7
#define PLAYER_LYRIC_COLOR        COLOR_LIGHT_GREEN
#define PLAYER_LYRIC_LINE         9
#define PLAYER_LYRIC_MAX_WITDH    40
#define PLAYER_LIST_INFO_X        7
#define PLAYER_LIST_INFO_Y        1
#define PLAYER_LIST_INFO_COLOR    COLOR_LIGHT_PURPLE
#define PLAYER_SHORTCUTS_X        3
#define PLAYER_SHORTCUTS_Y        6
#define PLAYER_LIST_X             3
#define PLAYER_LIST_Y             5
#define PLAYER_LIST_LINE          10        
#define PLAYER_LIST_CURRENT_COLOR COLOR_GREEN
#define PLAYER_LIST_CHOICE_COLOR  COLOR_LIGHT_GREEN

/* UI Mode */
#define PLAYER_SHOW_LYRIC         0
#define PLAYER_SHOW_SHORTCUTS     1
#define PLAYER_SHOW_LIST          2
#define PLAYER_SHOW_ABOUT         3
#define PLAYER_SHOW_SEARCH        4

#define VOLUME_CHANGE             5
#define TIME_CHANGE               10
#define LIST_MAX_CNT              1000
#define KEEP                      1
#define NO_KEEP                   0

/* øÿ÷∆√¸¡Ó */
#define CTR_NO_CTROL  0
#define CTRL_BREAK    1
#define CTRL_NEXT     2
#define CTRL_LAST     3
#define CTRL_ENTER    4

/* NetWork */
#define URL           "api.bzqll.com/music"

#define URL_NETEASE   "/netease"
#define URL_QQ        "/tencent"
#define URL_KUGOU     "/kugou"

#define URL_KEY       "?key=579621905"
#define URL_SEARCH    "/search"
#define URL_BR        "br=192000"

#define SEACH_URL(src, type, words)  URL "%s" URL_SEARCH URL_KEY "&type=%s&s=%s" , src, type, words

#define TEMP_FILE_PATH    "Temp/"

#endif
