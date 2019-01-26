#include "YylerPlayer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <assert.h>

static YylerPlayer *__yyler_player_bak = NULL;

#define IS_SPACE(ch) ((ch) == ' ' || (ch) == '\n' || (ch) == '\r' || (ch) == '\t')

static void print(Color color, int x, int y, int n, int fill, const char *str)
{
    int i;
    int len = strlen(str);

    Console_SetFrontColor(color);
    Console_SetCurPos(x, y);
    for (i = 0; i < n && i < len; i++)
        putchar(str[i]);
    if (i == n)
        printf("\b\b\b...");
    if (fill) {
        for (; i < n; i++)
            putchar(' ');
        putchar(' ');
    }
}


/* 生成音乐文件名对应的歌词文件名 */
static char *get_lyric_file_name(char *file_name)
{
    int len = strlen(file_name);
    while (--len > 0 && file_name[len] != '.');
    if (len == 0)
        return "";
    strcpy(file_name + len, ".lrc");
    return file_name;
}


static char *string_strip(char *str)
{
    int i = 0, j;
    while (IS_SPACE(str[i])) i++;
    j = strlen(str) - 1;
    while (IS_SPACE(str[j])) j--;
    str[j + 1] = 0;
    strcpy(str, str + i);
    return str;
}

/* 更新歌曲播放状态信息 */
void YylerPlayer_UpdateStatus(YylerPlayer *player, MusicList list)
{
    assert(player);
    assert(list);

    if (list->status.control_cmd == CTRL_ENTER)
        return;

    if (list->status.status != PAUSED) {
        list->status.time = Music_GetCurrentTimeLength(list->node->music);
        list->status.minute = list->status.time / 1000 / 60;
        list->status.second = list->status.time / 1000 % 60;
    }
}

/* 更新播放状态和界面 */
void YylerPlayer_Update(YylerPlayer *player, MusicList list)
{
    assert(player);
    assert(list);

    /* 处理按键 */
    YylerPlayer_ResolveKey(player, list);
    /* 更新提示 */
    if (player->prompt_show >= 0 && player->prompt_show++ == PLAYER_PROMPT_TIME_CNT)
    {
        int i, len = strlen(player->prompt_buf);
        if (player->cmd_no_ui)
            return;
        if (player->ui_drawed)
            Console_SetCurPos(player->err_x, player->err_y);
        else
            Console_SetCurPos(0, 0);
        for (i = 0; i < len; i++)
            putchar(' ');
        player->prompt_show = -1;
    }
    /* 更新歌曲播放状态信息 */
    YylerPlayer_UpdateStatus(player, list);
    /* 更新界面显示 */
    YylerPlayer_ShowFileName(player, list);
    YylerPlayer_ShowProgressBar(player, list);
    switch (player->ui_mode)
    {
    case PLAYER_SHOW_LYRIC:
        YylerPlayer_ShowLyric(player, list);
        break;
    case PLAYER_SHOW_SHORTCUTS:
        YylerPlayer_ShowShortcuts(player, list);
        break;
    case PLAYER_SHOW_LIST:
        YylerPlayer_ShowList(player, list);
        break;
    case PLAYER_SHOW_ABOUT:
        YylerPlayer_ShowAbout(player, list);
        break;
    case PLAYER_SHOW_SEARCH:
        YylerPlayer_ShowList(player, player->search_list);
    default:
        break;
    }
}
   
/* 绘制播放器的基本界面 */
void YylerPlayer_ShowFace(YylerPlayer *player)
{
    int i, j;
    assert(player);

    if (player->cmd_no_ui)
        return;

    /* 擦除提示/错误内容 */
    if (player->ui_drawed)
        Console_SetCurPos(player->prompt_x, player->prompt_y);
    else
        Console_SetCurPos(0, 0);
    for (i = 0; i < player->last_err_len; i++)
        putchar(' ');

    /* 绘制界面外框 */
    Console_SetFrontColor(COLOR_LIGHT_PURPLE);
    Console_SetCurPos(0, 0);
    printf(" +");
    for (i = 0; i < PLAYER_WIDTH; i++)
        putchar('-');
    printf("+\n");
    for (i = 0; i < PLAYER_HEIGHT; i++) {
        printf(" |");
        for (j = 0; j < PLAYER_WIDTH; j++)
            putchar(' ');
        printf("|\n");
    }
    printf(" +");
    for (i = 0; i < PLAYER_WIDTH; i++)
        putchar('-');
    printf("+\n");
    /* 默认歌单信息 */
    //Console_SetFrontColor(PLAYER_LIST_INFO_COLOR);
    //Console_SetCurPos(PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y);
    //printf("歌单名：");
    //Console_SetCurPos(PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y + 1);
    //printf("文件名：");
    //Console_SetCurPos(PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y + 2);
    //printf("当前是第0首，共0首");
    /* 歌名，歌手名信息 */
    //Console_SetCurPos(PLAYER_TITLE_X + PLAYER_TITLE_SIZE + 1, PLAYER_TITLE_Y);
    //Console_SetFrontColor(PLAYER_TITLE_COLOR);
    //putchar('|');
    //Console_SetCurPos(PLAYER_TITLE_X, PLAYER_TITLE_Y);
    //putchar('|');
    /* 进度条 */
    //Console_SetFrontColor(PLAYER_BAR_COLOR_2);
    //Console_SetCurPos(PLAYER_BAR_X, PLAYER_BAR_Y);
    //printf("-------------------------------------------------- 00:00/00:00 ");

    Console_ResetColor();
    Console_SetCurPos(3, PLAYER_HEIGHT);
    printf(" Q/Ctrl-C: 退出                                  H: 显示按键帮助");
    player->ui_drawed = 1;
}

/* 显示帮助 */
void YylerPlayer_ShowHelp(YylerPlayer *player)
{
    Console_SetFrontColor(COLOR_LIGHT_RED);
    printf("Yyler Music Player " YYLER_VERSION "      一个命令行多功能音乐播放器\n\n" );
    Console_SetFrontColor(COLOR_LIGHT_YELLOW);
    printf("用法： %s [选项 ...] [歌曲文件名 ...]\n", player->argv[0]);
    Console_ResetColor();
    printf("如果有歌曲文件名，将所有的歌曲文件创建为一个临时歌单并播放。\n\n");
    printf("选项：\n");
    printf("  -p, --play [文件名[...]]  播放的指定的一系列歌曲文件，此选项可以不要\n");
    printf("  -l, --list <歌单名>       播放该歌单中的音乐；\n");
    printf("                            如果命令行中有歌曲名，该选项的作用变为设置临时歌单名。\n");
    printf("  -r, --repeat [次数]       循环播放需要播放的音乐多次，次数必须是一个正整数；\n");
    printf("                            如果次数为空，表示无限循环。\n");
    printf("  -s, --shuffle             随机播放歌单中的歌曲。\n");
    //printf("  -o, --only <编号[...]>    仅播放设置编号的歌曲，编号必须是一系列的正整数。\n");
    printf("  -n, --no-ui               不显示界面，此时，播放器默认不再响应按键（除了Ctrl-C）。\n");
    printf("  -h, --help                显示此帮助信息并退出。\n");
    printf("  -v, --version             显示版本信息并退出。\n");
    printf("  -V, --volume <音量>       设置音乐播放时的初始音量；范围为0-100。默认：%d。\n", PLAYER_DEFAULT_VOLUME);
    printf("  -q, --no-quit             所有可播放的音乐播放完成后不退出。\n");
    printf("  -S, --start-search        启动播放器后不播放音乐，立即进入搜索界面。忽略-L。\n");
    printf("  -L, --start-list          启动播放器后不播放音乐，立即进入歌单列表界面。\n");
    printf("  -d, --no-save             不保存在线音乐，在线音乐在播放完成后立即被删除。\n");
    printf("  --download-dir <路径>     设置缓存路径，该路径必须存在。默认的缓存路径是：" TEMP_FILE_PATH "。\n");
    //printf("  -f, --file <数据文件名>   使用<数据文件名>中的歌单信息，而不是默认的数据文件名。\n");
    //printf("  --start-time <分:秒>      从设置的时间开始播放，仅对第一首音乐的第一次播放有效，不可超出范围。\n");
    //printf("  --lyric-dir <路径[...]>   将所有路径添加到歌词搜索路径。\n");
    printf("  --volume-change <变化量>  设置音量的变化量，音量+/音量-按照设置的变化量改变，范围值在1-49之间。\n");
    printf("  --time-change <变化量>    设置音乐时间的变化量，设置快进/快退所变化的秒数，范围在1-60之间。\n");
    printf("  --use-key                 当--no-ui参数被指定时，按键任然有效，但与显示有关的按键除外\n");
    printf("  --no-color                内容输出和界面绘制时不设置颜色，而全部采用终端默认颜色。\n");
    printf("  --do-not-clear            播放器退出时不擦除绘制的界面。\n");
    printf("\n示例： \n");
    Console_SetFrontColor(COLOR_LIGHT_GREEN);
    printf("  %s *.mp3 --repeat --volume 40 --list TempList \n", player->argv[0]);
    Console_ResetColor();
    printf("  表示循环播放当前目录下的所有mp3文件，并将默认音量设置为40，歌单名设置为TempList\n\n");
}

/* 显示 关于... */
void YylerPlayer_ShowAbout(YylerPlayer *player, MusicList list)
{
    int i;
    const char *about_str[] = {
        "    关于本软件：","",
        "    Yyler Music Player " YYLER_VERSION, "",
        "    一款在控制台/终端下运行的多功能音乐播放器。","",
        "    作者：马小波",
        "    日期：2019-1","",
        "    H键： 查看按键帮助   --help: 查看命令行帮助"
    };

    if (player->cmd_no_ui)
        return;
    if (list->status.status != START_PLAY && !player->ui_mode_changed)
        return;

    YylerPlayer_ClearUI(player);
    Console_ResetColor();
    for (i = 0; i < sizeof about_str / sizeof about_str[0]; i++) {
        Console_SetCurPos(PLAYER_SHORTCUTS_X, PLAYER_SHORTCUTS_Y + i);
        printf(about_str[i]);
    }
}

/* 显示快捷键帮助 */
void YylerPlayer_ShowShortcuts(YylerPlayer *player, MusicList list)
{
    const char *shortcuts_str[] = {
        "快捷键：",   
        "  Q:  退出程序                      Ctrl-C:  退出程序\n",
        "  ↑: 音量+/菜单上移               ↓:       音量-/菜单下移\n",
        "  ←: 快退                          →:      快退\n",
        "  <:  下一曲                        >:       下一曲\n",
        "  R:  重新播放当前音乐              P:       循环/取消循环播放当前音乐\n",
        "  L:  跳转至播放列表界面/回到主界面 回车:    播放选中的歌曲\n",
        "  S:  搜索网络上的歌曲              F:       列出最新/最热的网络歌单\n"
        "                                  空格:    暂停/继续\n",
        "  H:  查看所有的快捷键/回到主界面   V:       关于...",
    };

    if (player->cmd_no_ui)
        return;
    if (list->status.status == START_PLAY || player->ui_mode_changed) {
        int i;

        YylerPlayer_ClearUI(player);
        Console_ResetColor();
        for (i = 0; i < sizeof shortcuts_str / sizeof shortcuts_str[0]; i++) {
            Console_SetCurPos(PLAYER_SHORTCUTS_X, PLAYER_SHORTCUTS_Y + i);
            printf(shortcuts_str[i]);
        }
    }
}

#define LIST_SHOW_STRING  "%-3d %s - %s", node->id, node->name, node->singer

/* 显示歌单列表 */
void YylerPlayer_ShowList(YylerPlayer * player, MusicList list)
{
    int current_music = PLAYER_LIST_LINE /  2;
    MusicNode node;
    int i;

    if (player->cmd_no_ui)
        return;
    if (player->ui_mode_changed) {
        YylerPlayer_ClearUI(player);
        player->start_node_show = list->node ? list->node : list->first;
        player->current_node_show = player->start_node_show;
        player->list_node_changed = 1;
    }

    if (!player->list_node_changed)
        return;
    if (player->ui_mode == PLAYER_SHOW_SEARCH)
        sprintf(player->buf, "找到%s", list->name);
    else
        sprintf(player->buf, "歌单《%s", list->name);
    print(Console_GetDefaultFrontColor(), PLAYER_LIST_X, PLAYER_LIST_Y, 
        PLAYER_WIDTH - PLAYER_LIST_X - 10, 0, player->buf);
    if (player->ui_mode == PLAYER_SHOW_SEARCH)
        printf("%d个:", list->music_cnt);
    else
        printf("》共%d首:", list->music_cnt);
    node = player->start_node_show;
 
    for (i = 1; i <= PLAYER_LIST_LINE && node; i++) {
        Color color;

        if (node == player->current_node_show) {
            color = PLAYER_LIST_CHOICE_COLOR;
            sprintf(player->buf, ">>> " LIST_SHOW_STRING);
        } else  if (node == list->node) {
            color = PLAYER_LIST_CURRENT_COLOR;
            sprintf(player->buf, " |> " LIST_SHOW_STRING);
        } else{
            color = Console_GetDefaultFrontColor();
            sprintf(player->buf, "    " LIST_SHOW_STRING);
        }
        
        print(color, PLAYER_LIST_X, PLAYER_LIST_Y + i, 
            PLAYER_WIDTH - PLAYER_LIST_X - 12, 1, player->buf);
        printf("  %02d:%02d  ", node->minute, node->second);

        if (node->next == NULL || i == PLAYER_LIST_LINE)
            player->end_node_show = node;
        node = node->next; 
    }
    player->list_node_changed = 0;
}

/* 清理UI界面 */
void YylerPlayer_ClearUI(YylerPlayer *player) {
    int i, j;

    if (player->cmd_no_ui)
        return;
    for (i = PLAYER_LIST_INFO_Y + 4; i < PLAYER_HEIGHT - 1; i++) {
        Console_SetCurPos(3, i);
        for (j = 0; j < PLAYER_WIDTH - 2; j++)
            putchar(' ');
    } 
}

void YylerPlayer_ShowSearch(YylerPlayer *player)
{
    char input_buf[5];
    const char *p = NULL;
    int s = 0, ans1 = 0, ans2 = 0;
    const char *src[] = {URL_NETEASE, "网易云音乐", URL_QQ, "QQ音乐", URL_KUGOU, "酷狗音乐"};

    
    while (1) {
        int i;

        YylerPlayer_ClearUI(player);
        Console_ResetColor();
        for (i = 1; i <= 3; i++) {
            Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + i);
            printf("    <%d> %s", i, src[2 * i - 1]);
        }     
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + i++);
        printf("    （输入“s”可回到歌词界面）");
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + ++i);
        printf("    请选择搜索引擎：  (默认1) ");
        fgets(input_buf, sizeof input_buf, stdin);
        string_strip(input_buf);
        if (!strcmp(input_buf, "1") || input_buf[0] == '\0')
            ans1 = 1;
        else if (!strcmp(input_buf, "2"))
            ans1 = 2;
        else if (!strcmp(input_buf, "3"))
            ans1 = 3;
        else if (!strcmp(input_buf, "s") || !strcmp(input_buf, "S")) {
            player->ui_mode = PLAYER_SHOW_LYRIC;
            player->ui_mode_changed = 1;
            return;
        } else {
            Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + ++i);
            printf("    无效的输入！ <1, 2 or 3>    ");
            Thread_Sleep(1000);
            continue;
        }
        break;
    }

    while (1) {
        YylerPlayer_ClearUI(player);
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y);
        Console_ResetColor();
        printf("    <%s>输入要搜索的关键词：", src[ans1 * 2 - 1]);

        fgets(player->search_buf, sizeof player->search_buf, stdin);
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 1);
        if (strlen(player->search_buf) > 45) {
            printf("    输入字符太长！");
            Thread_Sleep(500);
        } else if (strlen(string_strip(player->search_buf)) == 0) {
            printf("    输入为空！");
            Thread_Sleep(500);
        } else
            break;
    }

    if (!strcmp("q", player->search_buf) || !strcmp("Q", player->search_buf))
        p = "退出", s = 1;
    if (!strcmp("s", player->search_buf) || !strcmp("S", player->search_buf))
        p = "返回到歌词界面", s = 2;

    while (s) {
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 1);
        printf("                                                            ");
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 1);
        printf("    %s?<y> 还是搜索“%s”? <n> (默认y) ", p, player->search_buf);
        fgets(input_buf, sizeof input_buf, stdin);
        if (input_buf[0] == '\n' || ((input_buf[0] == 'y' || input_buf[0] == 'n') && input_buf[1] == '\n')) {
            if (input_buf[0] == 'n')
                ans2 = 1;
            else
                ans2 = 2;
            break;
        }
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 2);
        printf("    无效的输入！ <y or n>");
        Thread_Sleep(1000);
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 2);
        printf("                                 ");
    }
    if (s == 1 && ans2 == 2) {
        YylerPlayer_Quit(player, 1, 0);
    }  else if (s == 2 && ans2 == 2) {
        player->ui_mode = PLAYER_SHOW_LYRIC;
        player->ui_mode_changed = 1;
        return;
    }

    printf("    搜索“%s”中 ... ", player->search_buf);
    MusicList_Free(player->search_list);
    player->search_list = YylerPlayer_GetListBySearchNet(player, src[2 * (ans1 - 1)], player->search_buf);
    if (player->search_list == NULL) 
        YylerPlayer_PrintError(player, "搜索歌曲失败", "");
}

MusicList YylerPlayer_GetListBySearchNet(YylerPlayer *player, const char *src, const char *words)
{
    char url_buf[1024] = { 0 };
    static char buf[1024000];
    MusicList list;
    HttpClient client;

    cJSON *body, *data, *tmp;

    sprintf(buf, "“%s”", words);
    list = MusicList_New(buf);
    if (!list) {
        YylerPlayer_PrintError(player, "", "搜索歌单创建失败");
        return NULL;
    }
    Console_GBKToUTF8(url_buf, words);
    Socket_URLEncode(buf, url_buf);
    sprintf(url_buf, SEACH_URL(src, "song", buf));
    //YylerPlayer_ShowPrompt(player, NO_KEEP, "正在连接 %s ...", url_buf); //getchar();
    if ((client = Socket_HttpGet(url_buf, NULL)) == NULL) {
        YylerPlayer_PrintError(player, "搜索失败", Socket_GetLastError());
        free(list);
        return NULL;
    }
    if (client->body == NULL) {
        YylerPlayer_PrintError(player, "搜索失败2", Socket_GetLastError());
        free(list);
        Socket_FreeHttpClient(client);
        return NULL;
    }
    Console_UTF8ToGBK(buf, client->body);
    //YylerPlayer_ShowPrompt(player, NO_KEEP, "%s\n", client->header);
    //printf(buf);
    if ((body = cJSON_Parse(buf)) == NULL) {
        YylerPlayer_PrintError(player, "网络信息解析失败", "无法解析获取的信息");
        free(list);
        Socket_FreeHttpClient(client);
        return NULL;
    }

    if ((tmp = cJSON_GetObjectItem(body, "result")) && !strcmp(tmp->valuestring, "SUCCESS") &&
        (tmp = cJSON_GetObjectItem(body, "code")) && tmp->valueint == 200) {
        int i, size;
        
        YylerPlayer_ShowPrompt(player, NO_KEEP, "搜索成功");
        
        data = cJSON_GetObjectItem(body, "data");
        size = cJSON_GetArraySize(data);
        //printf("size = %d\n", size); 
        for (i = 0; i < size; i++) {
            MusicNode node = MusicNode_CreateNodeByJsonData(cJSON_GetArrayItem(data, i));
            MusicList_AddMusicEnd(list, node);

           // printf("\n  %02d: %s - %s", i + 1, cJSON_GetObjectItem(tmp, "name")->valuestring,
           //     cJSON_GetObjectItem(tmp, "singer")->valuestring);
        }
        list->node = NULL;
    }
      
    cJSON_Delete(body);
    //Socket_FreeHttpClient(client);

    return list;
}

static void __download_callback_func(int n)
{
    printf("\r进度：%d%% ", n);
    //YylerPlayer_ShowPrompt(__yyler_player_bak, NO_KEEP, "下载中 ... %%%d  ", n);
}

int YylerPlayer_PlayerNetWorkMusic(YylerPlayer *player, MusicNode music)
{
    HttpClient client;
    FILE *fp;
    MusicNode node;

    assert(player);
    assert(music);

    YylerPlayer_ShowPrompt(player, KEEP, "正在下载歌曲：“%s - %s” ...", music->name, music->singer);
    Thread_Sleep(500);
    YylerPlayer_ShowPrompt(player, KEEP, " ");
    sprintf(player->buf, "%s&" URL_BR, music->name_url);
    __yyler_player_bak = player;
    client = Socket_HttpGet(player->buf, __download_callback_func);
    __yyler_player_bak = NULL;
    if (!client || !client->body) {
        YylerPlayer_PrintError(player, "歌曲下载失败", "");
        return -1;
    }

    sprintf(player->buf, "%s - %s.mp3", music->name, music->singer);
    strcpy(music->file_name, player->buf);
    sprintf(player->buf, TEMP_FILE_PATH "%s - %s.mp3", music->name, music->singer);
    strcpy(music->file_path, player->buf);

    fp = fopen(player->buf, "wb");
    if (fp) {
        fwrite(client->body, 1, client->body_len, fp);
        fclose(fp);
    }

    //Socket_FreeHttpClient(client);
    client = Socket_HttpGet(music->lyric_url, NULL);
    if (!client || !client->body)
        return -1;
    sprintf(player->buf, TEMP_FILE_PATH "%s - %s.lrc", music->name, music->singer);
    fp = fopen(player->buf, "w");
    if (fp) {
        fwrite(client->body, 1, client->body_len, fp);
        fclose(fp);
    }
    //Socket_FreeHttpClient(client);
    node = (MusicNode)malloc(sizeof(struct _MusicNode));
    if (node) {
        memcpy(node, music, sizeof(struct _MusicNode));
        MusicList_AddMusicFirst(player->list, node);
        player->current_node_show = node;
    }
    return 0;
}   

/* 界面显示提示词 */
void YylerPlayer_ShowPrompt(YylerPlayer *player, int keep, const char *format, ...)
{
    int i;
    va_list ap;
    if (player->cmd_no_ui)
        return;

    assert(player);
    va_start(ap, format);
    vsprintf(player->prompt_buf, format, ap);
    va_end(ap);

    /* 擦除之前的内容 */    
    if (player->ui_drawed)
        Console_SetCurPos(player->prompt_x, player->prompt_y);
    else
        Console_SetCurPos(0, 0);
    Console_SetFrontColor(PLAYER_PROMPT_COLOR);
    
    for (i = 0; i < player->last_err_len; i++)
        putchar(' ');
    if (player->ui_drawed)
        Console_SetCurPos(player->prompt_x, player->prompt_y);
    else
        Console_SetCurPos(0, 0);
    /* 显示新内容 */
    printf(player->prompt_buf);
    player->last_err_len = strlen(player->prompt_buf);
    player->prompt_show = keep ? -1 : 0;
}

/* 打印错误信息 */
void YylerPlayer_PrintError(YylerPlayer *player, const char *str1, const char *str2)
{
    int i;
    assert(player);

    if (player->ui_drawed)
        Console_SetCurPos(player->err_x, player->err_y);
    else
        Console_SetCurPos(0, 0);
    if (!player->cmd_no_ui)
        for (i = 0; i < player->last_err_len; i++)
            putchar(' ');
    if (player->ui_drawed)
        Console_SetCurPos(player->err_x, player->err_y);
    else
        Console_SetCurPos(0, 0);
    Console_ResetColor();
    printf(" %s: ", player->argv[0]);
    Console_SetFrontColor(COLOR_LIGHT_RED);
    printf("错误: ");
    Console_ResetColor();
    printf("%s: ", str1);
    Console_SetFrontColor(COLOR_LIGHT_RED);
    printf("%s\n", str2);
    Console_ResetColor();
    player->last_err_len = strlen(player->argv[0]) + strlen(str1) + strlen(str2) + 10;
}


/* 界面滚动显示歌曲文件名 */
void YylerPlayer_ShowFileName(YylerPlayer *player, MusicList list)
{
    static int start, end, s_flag = 1, flag = 0, len;
    static char *name = NULL;
    int i;

    assert(player);
    assert(list);

    if (list->status.status == PAUSED || player->cmd_no_ui)
        return;
    if (list->status.status == START_PLAY) {
        int pos = 0;
        if (name)
            free(name);
        name = malloc(PLAYER_TITLE_SIZE + strlen(list->node->name) + strlen(list->node->singer) + 25);
        memset(name, ' ', PLAYER_TITLE_SIZE);
        strcpy(name + PLAYER_TITLE_SIZE, "歌名：");
        strcat(name, list->node->name);
        strcat(name, "        歌手：");
        strcat(name, list->node->singer);
        strcat(name, " ");
        len = strlen(name);
        start = 0;
        end = PLAYER_TITLE_SIZE - 1;
        s_flag = 1;


    }

    Console_SetCurPos(PLAYER_TITLE_X + PLAYER_TITLE_SIZE + 1, PLAYER_TITLE_Y);
    Console_SetFrontColor(PLAYER_TITLE_COLOR);
    putchar('|');
    Console_SetCurPos(PLAYER_TITLE_X, PLAYER_TITLE_Y);
    putchar('|');
    if (name[start] < 0)
        s_flag = !s_flag;
    if (s_flag && name[start] < 0)
        putchar(' ');
    else
        putchar(name[start]);
    for (i = start + 1; i != end && i != len; i = (i + 1) % (len))
        putchar(name[i]);
    start = (start + 1) % (len);
    end = (end + 1) % (len);
    if (name[(end + len - 1) % (len)] < 0 && flag == 0)
        end++, flag = 1;
    else if (flag == 1)
        end = (end + len - 1) % (len), flag = 0;
}

/* 显示歌曲播放的进度条和时间 */
void YylerPlayer_ShowProgressBar(YylerPlayer *player, MusicList list)
{
    int x, i;

    assert(player);
    assert(list);

    if (player->cmd_no_ui)
        return;
    Console_SetFrontColor(PLAYER_TIME_COLOR);
    Console_SetCurPos(PLAYER_BAR_X + 50, PLAYER_BAR_Y);
    printf("  %02d:%02d/%02d:%02d", list->status.minute, list->status.second, 
                                    list->node->minute,  list->node->second);
    Console_SetCurPos(PLAYER_BAR_X, PLAYER_BAR_Y);
    x = (int)(50.0*list->status.time / list->node->time);
    Console_SetFrontColor(PLAYER_BAR_COLOR_1);
    for (i = 0; i <= x; i++)
        putchar('+');
    Console_SetFrontColor(PLAYER_BAR_COLOR_2);
    for (; i < 50; i++)
        putchar('-');
}

void YylerPlayer_ShowListInfo(YylerPlayer * player, MusicList list)
{
    assert(player);
    assert(list);

    if (player->cmd_no_ui)
        return;
    sprintf(player->buf, "歌单名：《%s", list->name);
    print(PLAYER_LIST_INFO_COLOR, PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y,
        PLAYER_WIDTH - PLAYER_LIST_INFO_X - 5, 0, player->buf);
    printf("》");
    sprintf(player->buf, "文件名：“%s", list->node->file_name);
    print(PLAYER_LIST_INFO_COLOR, PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y + 1,
        PLAYER_WIDTH - PLAYER_LIST_INFO_X - 5, 0, player->buf);
    printf("”");
    Console_SetCurPos(PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y + 2);
    printf("当前是第%d首，共%d首", list->node->id, list->music_cnt);
}

/* 显示多行歌词 */
static void __print_lyric_lines(YylerPlayer *player, MusicList list)
{
    assert(player);
    assert(list);

    if (player->ui_mode != PLAYER_SHOW_LYRIC || player->cmd_no_ui || 
        list->status.control_cmd == CTRL_ENTER)
        return;
    if (list->node->lyric) {
        Color color;
        int i;

        for (i = 0; i < PLAYER_LYRIC_LINE; i++) {
            Console_SetCurPos(PLAYER_LYRIC_X, PLAYER_LYRIC_Y + i);
            if (i == PLAYER_LYRIC_LINE / 2) {
                color = PLAYER_LYRIC_COLOR;
                sprintf(player->buf, ">>> ");
            } else {
                color = Console_GetDefaultFrontColor();
                sprintf(player->buf, "    ");
            }
                
            Console_UTF8ToGBK(player->buf + 4, Lyric_GetLyric(list->node->lyric, i - PLAYER_LYRIC_LINE / 2));
            print(color, PLAYER_LYRIC_X, PLAYER_LYRIC_Y + i, PLAYER_WIDTH - PLAYER_LYRIC_X - 2, 1, player->buf);
        }
    } else if (list->status.status == START_PLAY || 
        (player->ui_mode_changed && player->ui_mode == PLAYER_SHOW_LYRIC)) {
        Console_SetFrontColor(COLOR_LIGHT_RED);
        Console_SetCurPos(PLAYER_LYRIC_X, PLAYER_LYRIC_Y + PLAYER_LYRIC_LINE / 2);
        printf(">>> 未找到该歌曲的歌词。");
    }
}

/* 更新界面的歌词显示 */
void YylerPlayer_ShowLyric(YylerPlayer *player, MusicList list)
{
    assert(player);
    assert(list);

    if (player->cmd_no_ui)
        return;
    if (player->ui_mode_changed) {
        YylerPlayer_ClearUI(player);
        __print_lyric_lines(player, list);
    }

    if (Lyric_ShouldBeNext(list->node->lyric, list->status.time) > 0 )
        __print_lyric_lines(player, list);
}

/* 获取歌曲文件名 */
static void  get_list_by_cmdline(YylerPlayer *player)
{
    const char *args;
    char **file_names;
    int cnt = 0;

    assert(player);

    file_names = (char **)malloc(LIST_MAX_CNT * sizeof(char *));
    if (file_names == NULL) {
        printf("内存错误\n");
        exit(-1);
    }
    while (strcmp(args = CmdLine_GetNextString(), "")) {
        if (IS_ARG(args)) {
            int this_cnt = System_GetFiles(file_names + cnt, LIST_MAX_CNT - cnt, args);
            if (this_cnt == 0) 
                file_names[cnt++] = (char *)args;
            else
                cnt += this_cnt;

        } else {
            sprintf(player->err_buf1, "“%s”", args);
            YylerPlayer_PrintError(player, player->err_buf1, "无法识别的选项");
            printf(" 使用“--help”查看帮助\n");
            free(file_names);
            exit(-1);
        }
    }
    player->list = MusicList_New("stdin");
    if (player->list) {
        int i;
        MusicNode node;
        for (i = 0; i < cnt; i++) {
            if ((node = MusicNode_CreateNodeByName(file_names[i]))) {
                MusicList_AddMusicEnd(player->list, node);
                YylerPlayer_ShowPrompt(player, NO_KEEP, " 添加文件“%s” ...", file_names[i]);
            } else {
                sprintf(player->err_buf1, "文件“%s”添加失败", file_names[i]);
                YylerPlayer_PrintError(player, player->err_buf1, Music_GetLastError());
                Thread_Sleep(300);
            }
        }
    }
    
    if (cnt) 
        player->have_stdin = 1;
    free(file_names);
    CmdLine_Free();
}

static void cmdline_repeat(YylerPlayer *player, const char *args)
{
    char *end;
    if (args == "")
        player->cmdline_repeat = -1;
    else {
        player->cmdline_repeat = strtol(args, &end, 0);
        if (end == args) {
            YylerPlayer_PrintError(player, args, "-r, --repeat  选项的参数非法");
            printf(" 使用“--help”查看帮助\n");
            exit(-1);
        }
    }
}

static void cmdline_get_volume(YylerPlayer *player, const char *args)
{
    char *end;
    if (args == "") {
        YylerPlayer_PrintError(player, "-V, --volume", "选项缺少参数");
        printf(" 使用“--help”查看帮助\n");
        exit(-1);
    }  else {
        int volume = strtol(args, &end, 0);
        if (end == args || volume > 100) {
            YylerPlayer_PrintError(player, args, "-V, --volume  选项的参数非法");
            printf(" 使用“--help”查看帮助\n");
            exit(-1);
        }
        player->cmdline_volume = volume;
    }
}

static void cmdline_get_volume_change(YylerPlayer *player, const char *args)
{
    char *end;
    if (args == "") {
        YylerPlayer_PrintError(player, "--volume-change", "选项缺少参数");
        printf(" 使用“--help”查看帮助\n");
        exit(-1);
    } else {
        int change = strtol(args, &end, 0);
        if (end == args || change < 1 || change > 49) {
            YylerPlayer_PrintError(player, args, "--volume-change  选项的参数非法");
            printf(" 使用“--help”查看帮助\n");
            exit(-1);
        }
        player->cmdline_volume_change = change;
    }
}

static void cmdline_get_time_change(YylerPlayer *player, const char *args)
{
    char *end;
    if (args == "") {
        YylerPlayer_PrintError(player, "--time-change", "选项缺少参数");
        printf(" 使用“--help”查看帮助\n");
        exit(-1);
    } else {
        int change = strtol(args, &end, 0);
        if (end == args || change < 1 || change > 60) {
            YylerPlayer_PrintError(player, args, "--time-change  选项的参数非法");
            printf(" 使用“--help”查看帮助\n");
            exit(-1);
        }
        player->cmdline_time_change = change;
    }
}

static void cmdline_get_list(YylerPlayer *player, const char *args)
{
    if (args == "") {
        YylerPlayer_PrintError(player, "-l, --list", "选项缺少参数");
        printf(" 使用“--help”查看帮助\n");
        exit(-1);
    } else {
        strncpy(player->cmd_list, args, sizeof player->cmd_list);
        player->cmd_list[sizeof player->cmd_list - 1] = 0;
    }
}


/* 处理命令行 */
void YylerPlayer_ResolveCmdLine(YylerPlayer *player)
{
    const char *args;
    assert(player);

    CmdLine_Init(player->argc, player->argv);
    /* --no-color */
    if (CmdLine_HaveLongOpt("no-color", 1))
        Console_UseColor(0);
    /* -h, --help */
    if (CmdLine_HaveShortOpt('h', 1) || CmdLine_HaveLongOpt("help", 1)) {
        YylerPlayer_ShowHelp(player);
        YylerPlayer_Quit(player, 0, 0);
    }
    /* -v, --version */
    if (CmdLine_HaveShortOpt('v', 1) || CmdLine_HaveLongOpt("version", 1)) {
        Console_SetFrontColor(COLOR_LIGHT_YELLOW);
        printf("Yyler Music Player " YYLER_VERSION "\n");
        YylerPlayer_Quit(player, 0, 0);
    }
    /* -p, --play */
    if (CmdLine_HaveShortOpt('p', 1) || CmdLine_HaveLongOpt("play", 1)) {
        CmdLine_HaveShortOpt('p', 1);
        CmdLine_HaveLongOpt("play", 1);
        player->have_stdin = 1;
    }
    /* -l, --list */
    if ((args = CmdLine_GetNextArgOfShortOpt('l'))) {
        cmdline_get_list(player, args);
        CmdLine_HaveLongOpt("list", 1);
    } else if ((args = CmdLine_GetNextArgOfLongOpt("list"))) {
        cmdline_get_list(player, args);
    }
    /* -r, --repeat  */
    if ((args = CmdLine_GetNextArgOfShortOpt('r'))) {
        cmdline_repeat(player, args);
        CmdLine_HaveLongOpt("repeat", 1);
    }  else if ((args = CmdLine_GetNextArgOfLongOpt("repeat"))) {
        cmdline_repeat(player, args);
    } else
        player->cmdline_repeat = 1;
    /* -s, --shuffle */
    if (CmdLine_HaveShortOpt('s', 1) || CmdLine_HaveLongOpt("shuffle", 1)) {
        CmdLine_HaveShortOpt('s', 1);
        CmdLine_HaveLongOpt("shuffle", 1);
        player->cmd_shuffle = 1;
    }
    /* -V, --volume */
    if ((args = CmdLine_GetNextArgOfShortOpt('V'))) {
        cmdline_get_volume(player, args);
        CmdLine_HaveLongOpt("volume", 1);
    } else if ((args = CmdLine_GetNextArgOfLongOpt("volume"))) {
        cmdline_get_volume(player, args);
    }
    /* --volume-change */
    if ((args = CmdLine_GetNextArgOfLongOpt("volume-change"))) {
        cmdline_get_volume_change(player, args);
    }
    /*--time-change */
    if ((args = CmdLine_GetNextArgOfLongOpt("time-change"))) {
        cmdline_get_time_change(player, args);
    }
    /* -n, --no-ui */
    if (CmdLine_HaveShortOpt('n', 1) || CmdLine_HaveLongOpt("no-ui", 1)) {
        CmdLine_HaveShortOpt('n', 1);
        CmdLine_HaveLongOpt("no-ui", 1);
        player->cmd_no_ui = 1;
    }
    /* --use-key */
    if (CmdLine_HaveLongOpt("use-key", 1)) {
        player->cmd_use_key = 1;
    }
    /* --do-not-clear */
    if ( CmdLine_HaveLongOpt("do-not-clear", 1)) {
        player->cmd_do_not_clear = 1;
    }

    /*绘制界面 */
    //YylerPlayer_ShowFace(player);
    /* 获取歌曲文件名 */
    get_list_by_cmdline(player);
    /* 处理歌曲文件名和歌曲列表 */
    if (*player->cmd_list)
        strcpy(player->list->name, player->cmd_list);
    if (!player->have_stdin && *player->cmd_list) {
        /* 从配置文件中搜索歌单 */
        sprintf(player->err_buf2, "未找到歌单“%s”", player->cmd_list);
        YylerPlayer_PrintError(player, "无法播放歌曲", player->err_buf2);
        YylerPlayer_Quit(player, 0, -1);
    } else if (!player->list->music_cnt) {
        Thread_Sleep(1000);
        sprintf(player->err_buf1, "歌单《%s》创建失败", player->list->name);
        YylerPlayer_PrintError(player, player->err_buf1, "该歌单中无可播放的音乐");
        YylerPlayer_Quit(player, 0, -1);
    } 
}

/* 播放器按键处理 */
void YylerPlayer_ResolveKey(YylerPlayer *player, MusicList list)
{
    assert(list);
    assert(player);

    if (Console_HaveKey())
    {
        int key = Console_GetKey(NO_MATCH_CASE);
        int ui_mode;
        if (player->cmd_no_ui && !player->cmd_use_key)
            return;

        switch (key) {
        case ' ':
            if (list->status.status == PAUSED) {
                Music_Resume(list->node->music);
                list->status.status = PLAYING;
                YylerPlayer_ShowPrompt(player, NO_KEEP, "             ");
            } else if (list->status.status == PLAYING) {
                Music_Pause(list->node->music);
                list->status.status = PAUSED;
                YylerPlayer_ShowPrompt(player, KEEP, " --已暂停-- ");
            }
            break;
        case 'Q':
            YylerPlayer_Quit(player, 1, 0);
            break;
        case 'H':
            if (player->cmd_no_ui)
                break;
            if (player->ui_mode != PLAYER_SHOW_SHORTCUTS) 
                player->ui_mode = PLAYER_SHOW_SHORTCUTS;
            else
                player->ui_mode = PLAYER_SHOW_LYRIC;
            player->ui_mode_changed = 1;
            break;
        case 'L':
            if (player->cmd_no_ui)
                break;
            if (player->ui_mode != PLAYER_SHOW_LIST) 
                player->ui_mode = PLAYER_SHOW_LIST;
            else
                player->ui_mode = PLAYER_SHOW_LYRIC;
            player->ui_mode_changed = 1;
            break;
        case 'R':
            YylerPlayer_ShowPrompt(player, NO_KEEP, " 重新播放");
            Music_Play(list->node->music, 1, 0, 0);
            Lyric_GotoCurrentTime(list->node->lyric, 0);
            __print_lyric_lines(player, list);
            break;
        case 'P':
            if (player->repeat_one) {
                YylerPlayer_ShowPrompt(player, NO_KEEP, " 已取消对当前歌曲的循环播放");
                player->repeat_one = 0;
            } else {
                YylerPlayer_ShowPrompt(player, NO_KEEP, " 将循环播放当前歌曲");
                player->repeat_one = 1;
            }
            break;
        case 'V':
            if (player->cmd_no_ui)
                break;
            if (player->ui_mode != PLAYER_SHOW_ABOUT)
                player->ui_mode = PLAYER_SHOW_ABOUT;
            else
                player->ui_mode = PLAYER_SHOW_LYRIC;
            player->ui_mode_changed = 1;
            break;
        case 'S':
            ui_mode = player->ui_mode;
            player->ui_mode = PLAYER_SHOW_SEARCH;

            if (ui_mode == PLAYER_SHOW_SEARCH || !player->search_list)
                YylerPlayer_ShowSearch(player);

            player->ui_mode_changed = 1;
            break;
            case KEY_UP:
        case KEY_DOWN:
            if (player->ui_mode != PLAYER_SHOW_LIST && player->ui_mode != PLAYER_SHOW_SEARCH) {
                if (key == KEY_UP) {
                    list->status.volume += player->cmdline_volume_change;
                    if (list->status.volume > 100)
                        list->status.volume = 100;
                }  else {
                    list->status.volume -= player->cmdline_volume_change;
                    if (list->status.volume < 0)
                        list->status.volume = 0;
                }
                Music_SetVolume(list->node->music, list->status.volume * 10);
                YylerPlayer_ShowPrompt(player, NO_KEEP, " 音量： %d%%    ", list->status.volume);
            } else if (player->current_node_show) {
                if (key == KEY_DOWN) {
                    if (player->current_node_show == player->end_node_show &&
                        player->end_node_show->next)
                        player->end_node_show = player->end_node_show->next,
                        player->start_node_show = player->start_node_show->next;
                    if (player->current_node_show->next)
                        player->current_node_show = player->current_node_show->next;
                }  else {
                    if (player->current_node_show == player->start_node_show &&
                        player->start_node_show->last)
                        player->end_node_show = player->end_node_show->last,
                        player->start_node_show = player->start_node_show->last;
                    if (player->current_node_show->last)
                        player->current_node_show = player->current_node_show->last;
                }
                player->list_node_changed = 1;
            }
            break;
        case ',':
            if (list->node->id == 1 && !player->cmd_shuffle) {
                YylerPlayer_ShowPrompt(player, 0, " 这是第一首！");
                break;
            }
            list->status.control_cmd = CTRL_LAST;
            YylerPlayer_ShowPrompt(player, 0, " 上一曲");
            break;
        case '.':
            if (list->node->id == list->music_cnt && !player->cmd_shuffle) {
                YylerPlayer_ShowPrompt(player, 0, " 这是最后一首！");
                break;
            }
            list->status.control_cmd = CTRL_NEXT;
            YylerPlayer_ShowPrompt(player, 0, " 下一曲");
            break;
        case KEY_ENTER:
            if (player->current_node_show == NULL)
                break;
            if (player->ui_mode == PLAYER_SHOW_SEARCH) 
                if (YylerPlayer_PlayerNetWorkMusic(player, player->current_node_show))
                    break;
            if (player->ui_mode == PLAYER_SHOW_LIST || player->ui_mode == PLAYER_SHOW_SEARCH) {
                list->status.control_cmd = CTRL_ENTER;
                player->ui_mode = PLAYER_SHOW_LYRIC;
                player->ui_mode_changed = 1;
            }
            
            break;
        case KEY_LEFT:
        case KEY_RIGHT:
            if (key == KEY_RIGHT) {
                list->status.time += player->cmdline_time_change * 1000;
                if (list->status.time > list->node->time - 500)
                    list->status.time = list->node->time - 500;
                YylerPlayer_ShowPrompt(player, NO_KEEP, " 快进%d秒", player->cmdline_time_change);
            } else {
                if (list->status.time < (MTLen)player->cmdline_time_change * 1000)
                    list->status.time = 1;
                else
                    list->status.time -= player->cmdline_time_change * 1000;
                YylerPlayer_ShowPrompt(player, NO_KEEP, " 快退%d秒", player->cmdline_time_change);
                Lyric_GotoCurrentTime(list->node->lyric, list->status.time);
                __print_lyric_lines(player, list);
            }
            Music_Play(list->node->music, list->status.time, 0, 0);
            /* 如果当前处于暂停状态，改成播放中状态 */
            if (list->status.status == PAUSED)
                list->status.status = PLAYING;
            break;
        default:
            break;
        }
    }
}

static int is_music_should_be_stoped(YylerPlayer *player, MusicList list)
{
    assert(list);
    assert(player);

    if (Music_GetStatus(list->node->music) == MUSIC_STOPPED)
        return 1;
    if (list->status.control_cmd == CTRL_LAST  ||
        list->status.control_cmd == CTRL_NEXT  ||
        list->status.control_cmd == CTRL_BREAK ||
        list->status.control_cmd == CTRL_ENTER)
        return 1;
    return 0;
}

int YylerPlayer_PlayMusic(YylerPlayer *player, MusicList list)
{
    assert(list);
    assert(player);
    assert(list->node);

    if (!(list->node->music = Music_Create(list->node->file_path))) {
        sprintf(player->err_buf1, "无法播放歌曲文件“%s”", list->node->file_path);
        YylerPlayer_PrintError(player, player->err_buf1, Music_GetLastError());
        Thread_Sleep(2000);
        return -1;
    }

    strcpy(player->buf, list->node->file_path);
    list->node->lyric = Lyric_GetLyricByFile(get_lyric_file_name(player->buf));
    list->node->time = Music_GetTimeLength(list->node->music);
    list->node->minute = list->node->time / 1000 / 60;
    list->node->second = list->node->time / 1000 % 60;
    list->status.status = START_PLAY;

    Music_SetVolume(list->node->music, list->status.volume * 10);
    YylerPlayer_ShowFace(player);
    YylerPlayer_ShowPrompt(player, NO_KEEP, " 开始播放： %s", list->node->name);
    YylerPlayer_ShowListInfo(player, list);
    Music_Play(list->node->music, 0, 0, 0);
    __print_lyric_lines(player, list);

    do {
        YylerPlayer_Update(player, list);
        Thread_Sleep(100);
        if (list->status.status == START_PLAY)
            list->status.status = PLAYING;
        player->ui_mode_changed = 0;
    } while (!is_music_should_be_stoped(player, list));
    Music_Delete(list->node->music);
    Lyric_Free(list->node->lyric);
    list->node->music = NULL;
    list->node->lyric = NULL;
    player->list_node_changed = 1;
    return 0;
}

static MusicNode get_random_node(MusicList list)
{
    int i, n = rand() % list->music_cnt;
    MusicNode node = list->first;
    for (i = 0; i < n; i++) 
        node = node->next;
    return node;
}

void YylerPlayer_ChangeListByControl(YylerPlayer *player, MusicList list)
{
    assert(list);
    assert(player);


    switch (list->status.control_cmd) {
    case CTR_NO_CTROL:  
        if (player->repeat_one)
            break;
        if (player->cmd_shuffle) 
            list->node = get_random_node(list);
        else {
            if (list->node->next == NULL) {
                if (player->cmdline_repeat && --player->cmdline_repeat) {
                    list->node = list->first;
                } else
                    list->status.control_cmd = CTRL_BREAK;
            } else
                list->node = list->node->next;
        }
        break;
    case CTRL_NEXT:
        list->node = player->cmd_shuffle ? get_random_node(list): list->node->next;
         break;
    case CTRL_LAST:
        list->node = player->cmd_shuffle ? get_random_node(list) : list->node->last;
        break;
    case CTRL_ENTER:
        list->node = player->current_node_show;
        break;
    default:
        break;
    }
}

void YylerPlayer_PlayMusicList(YylerPlayer *player, MusicList list)
{
    assert(list);
    assert(player);

    list->node = player->cmd_shuffle ? get_random_node(list) : list->first;
    if (player->cmdline_volume < 0)
        list->status.volume = PLAYER_DEFAULT_VOLUME;
    else
        list->status.volume = player->cmdline_volume;

    assert(list->node);
    while (list->status.control_cmd != CTRL_BREAK) {
        list->status.control_cmd = CTR_NO_CTROL;
        if (player->cmdline_repeat)
            YylerPlayer_PlayMusic(player, list);
        YylerPlayer_ChangeListByControl(player, list);
    }
}

/* 播放器初始化 */
void YylerPlayer_Init(YylerPlayer *player, int argc, char *argv[])
{ 
    int x, y;

    assert(player);

    srand((unsigned int)time(NULL));
    Console_GetCurPos(&x, &y);
    Console_SetOrigin(x, y);

    memset(player, 0, sizeof(struct _YylerPlayer));
    player->argc = argc;
    player->argv = argv;
    player->err_x = PLAYER_ERR_X;
    player->err_y = PLAYER_ERR_Y;
    player->prompt_x = PLAYER_PROMPT_X;
    player->prompt_y = PLAYER_PROMPT_Y;
    player->cmdline_volume = -1;
    player->cmdline_volume_change = VOLUME_CHANGE;
    player->cmdline_time_change = TIME_CHANGE;
    YylerPlayer_ResolveCmdLine(player);
}

/* 退出播放器 */
void YylerPlayer_Quit(YylerPlayer *player, int clear, int _quit)
{
    int i, j;

    assert(player);
    MusicList_Free(player->list);
    Console_ResetColor();
   
    /* 擦除界面 */
    if (clear && player->ui_drawed && !player->cmd_do_not_clear)
    {
        Console_SetCurPos(0, 0);
        for (i = 0; i < PLAYER_HEIGHT + 3; i++) {
            for (j = 0; j < PLAYER_WIDTH + 3; j++)
                putchar(' ');
            if (i < PLAYER_HEIGHT + 2)
                putchar('\n');
        }
        Console_SetCurPos(0, 0);
    } else if (player->ui_drawed)
        Console_SetCurPos(0, PLAYER_HEIGHT + 2);
    /* 退出程序 */
    exit(_quit);
}
