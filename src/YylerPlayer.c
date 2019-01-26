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


/* ���������ļ�����Ӧ�ĸ���ļ��� */
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

/* ���¸�������״̬��Ϣ */
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

/* ���²���״̬�ͽ��� */
void YylerPlayer_Update(YylerPlayer *player, MusicList list)
{
    assert(player);
    assert(list);

    /* ������ */
    YylerPlayer_ResolveKey(player, list);
    /* ������ʾ */
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
    /* ���¸�������״̬��Ϣ */
    YylerPlayer_UpdateStatus(player, list);
    /* ���½�����ʾ */
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
   
/* ���Ʋ������Ļ������� */
void YylerPlayer_ShowFace(YylerPlayer *player)
{
    int i, j;
    assert(player);

    if (player->cmd_no_ui)
        return;

    /* ������ʾ/�������� */
    if (player->ui_drawed)
        Console_SetCurPos(player->prompt_x, player->prompt_y);
    else
        Console_SetCurPos(0, 0);
    for (i = 0; i < player->last_err_len; i++)
        putchar(' ');

    /* ���ƽ������ */
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
    /* Ĭ�ϸ赥��Ϣ */
    //Console_SetFrontColor(PLAYER_LIST_INFO_COLOR);
    //Console_SetCurPos(PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y);
    //printf("�赥����");
    //Console_SetCurPos(PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y + 1);
    //printf("�ļ�����");
    //Console_SetCurPos(PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y + 2);
    //printf("��ǰ�ǵ�0�ף���0��");
    /* ��������������Ϣ */
    //Console_SetCurPos(PLAYER_TITLE_X + PLAYER_TITLE_SIZE + 1, PLAYER_TITLE_Y);
    //Console_SetFrontColor(PLAYER_TITLE_COLOR);
    //putchar('|');
    //Console_SetCurPos(PLAYER_TITLE_X, PLAYER_TITLE_Y);
    //putchar('|');
    /* ������ */
    //Console_SetFrontColor(PLAYER_BAR_COLOR_2);
    //Console_SetCurPos(PLAYER_BAR_X, PLAYER_BAR_Y);
    //printf("-------------------------------------------------- 00:00/00:00 ");

    Console_ResetColor();
    Console_SetCurPos(3, PLAYER_HEIGHT);
    printf(" Q/Ctrl-C: �˳�                                  H: ��ʾ��������");
    player->ui_drawed = 1;
}

/* ��ʾ���� */
void YylerPlayer_ShowHelp(YylerPlayer *player)
{
    Console_SetFrontColor(COLOR_LIGHT_RED);
    printf("Yyler Music Player " YYLER_VERSION "      һ�������ж๦�����ֲ�����\n\n" );
    Console_SetFrontColor(COLOR_LIGHT_YELLOW);
    printf("�÷��� %s [ѡ�� ...] [�����ļ��� ...]\n", player->argv[0]);
    Console_ResetColor();
    printf("����и����ļ����������еĸ����ļ�����Ϊһ����ʱ�赥�����š�\n\n");
    printf("ѡ�\n");
    printf("  -p, --play [�ļ���[...]]  ���ŵ�ָ����һϵ�и����ļ�����ѡ����Բ�Ҫ\n");
    printf("  -l, --list <�赥��>       ���Ÿø赥�е����֣�\n");
    printf("                            ������������и���������ѡ������ñ�Ϊ������ʱ�赥����\n");
    printf("  -r, --repeat [����]       ѭ��������Ҫ���ŵ����ֶ�Σ�����������һ����������\n");
    printf("                            �������Ϊ�գ���ʾ����ѭ����\n");
    printf("  -s, --shuffle             ������Ÿ赥�еĸ�����\n");
    //printf("  -o, --only <���[...]>    ���������ñ�ŵĸ�������ű�����һϵ�е���������\n");
    printf("  -n, --no-ui               ����ʾ���棬��ʱ��������Ĭ�ϲ�����Ӧ����������Ctrl-C����\n");
    printf("  -h, --help                ��ʾ�˰�����Ϣ���˳���\n");
    printf("  -v, --version             ��ʾ�汾��Ϣ���˳���\n");
    printf("  -V, --volume <����>       �������ֲ���ʱ�ĳ�ʼ��������ΧΪ0-100��Ĭ�ϣ�%d��\n", PLAYER_DEFAULT_VOLUME);
    printf("  -q, --no-quit             ���пɲ��ŵ����ֲ�����ɺ��˳���\n");
    printf("  -S, --start-search        �����������󲻲������֣����������������档����-L��\n");
    printf("  -L, --start-list          �����������󲻲������֣���������赥�б���档\n");
    printf("  -d, --no-save             �������������֣����������ڲ�����ɺ�������ɾ����\n");
    printf("  --download-dir <·��>     ���û���·������·��������ڡ�Ĭ�ϵĻ���·���ǣ�" TEMP_FILE_PATH "��\n");
    //printf("  -f, --file <�����ļ���>   ʹ��<�����ļ���>�еĸ赥��Ϣ��������Ĭ�ϵ������ļ�����\n");
    //printf("  --start-time <��:��>      �����õ�ʱ�俪ʼ���ţ����Ե�һ�����ֵĵ�һ�β�����Ч�����ɳ�����Χ��\n");
    //printf("  --lyric-dir <·��[...]>   ������·����ӵ��������·����\n");
    printf("  --volume-change <�仯��>  ���������ı仯��������+/����-�������õı仯���ı䣬��Χֵ��1-49֮�䡣\n");
    printf("  --time-change <�仯��>    ��������ʱ��ı仯�������ÿ��/�������仯����������Χ��1-60֮�䡣\n");
    printf("  --use-key                 ��--no-ui������ָ��ʱ��������Ȼ��Ч��������ʾ�йصİ�������\n");
    printf("  --no-color                ��������ͽ������ʱ��������ɫ����ȫ�������ն�Ĭ����ɫ��\n");
    printf("  --do-not-clear            �������˳�ʱ���������ƵĽ��档\n");
    printf("\nʾ���� \n");
    Console_SetFrontColor(COLOR_LIGHT_GREEN);
    printf("  %s *.mp3 --repeat --volume 40 --list TempList \n", player->argv[0]);
    Console_ResetColor();
    printf("  ��ʾѭ�����ŵ�ǰĿ¼�µ�����mp3�ļ�������Ĭ����������Ϊ40���赥������ΪTempList\n\n");
}

/* ��ʾ ����... */
void YylerPlayer_ShowAbout(YylerPlayer *player, MusicList list)
{
    int i;
    const char *about_str[] = {
        "    ���ڱ������","",
        "    Yyler Music Player " YYLER_VERSION, "",
        "    һ���ڿ���̨/�ն������еĶ๦�����ֲ�������","",
        "    ���ߣ���С��",
        "    ���ڣ�2019-1","",
        "    H���� �鿴��������   --help: �鿴�����а���"
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

/* ��ʾ��ݼ����� */
void YylerPlayer_ShowShortcuts(YylerPlayer *player, MusicList list)
{
    const char *shortcuts_str[] = {
        "��ݼ���",   
        "  Q:  �˳�����                      Ctrl-C:  �˳�����\n",
        "  ��: ����+/�˵�����               ��:       ����-/�˵�����\n",
        "  ��: ����                          ��:      ����\n",
        "  <:  ��һ��                        >:       ��һ��\n",
        "  R:  ���²��ŵ�ǰ����              P:       ѭ��/ȡ��ѭ�����ŵ�ǰ����\n",
        "  L:  ��ת�������б����/�ص������� �س�:    ����ѡ�еĸ���\n",
        "  S:  ���������ϵĸ���              F:       �г�����/���ȵ�����赥\n"
        "                                  �ո�:    ��ͣ/����\n",
        "  H:  �鿴���еĿ�ݼ�/�ص�������   V:       ����...",
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

/* ��ʾ�赥�б� */
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
        sprintf(player->buf, "�ҵ�%s", list->name);
    else
        sprintf(player->buf, "�赥��%s", list->name);
    print(Console_GetDefaultFrontColor(), PLAYER_LIST_X, PLAYER_LIST_Y, 
        PLAYER_WIDTH - PLAYER_LIST_X - 10, 0, player->buf);
    if (player->ui_mode == PLAYER_SHOW_SEARCH)
        printf("%d��:", list->music_cnt);
    else
        printf("����%d��:", list->music_cnt);
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

/* ����UI���� */
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
    const char *src[] = {URL_NETEASE, "����������", URL_QQ, "QQ����", URL_KUGOU, "�ṷ����"};

    
    while (1) {
        int i;

        YylerPlayer_ClearUI(player);
        Console_ResetColor();
        for (i = 1; i <= 3; i++) {
            Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + i);
            printf("    <%d> %s", i, src[2 * i - 1]);
        }     
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + i++);
        printf("    �����롰s���ɻص���ʽ��棩");
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + ++i);
        printf("    ��ѡ���������棺  (Ĭ��1) ");
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
            printf("    ��Ч�����룡 <1, 2 or 3>    ");
            Thread_Sleep(1000);
            continue;
        }
        break;
    }

    while (1) {
        YylerPlayer_ClearUI(player);
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y);
        Console_ResetColor();
        printf("    <%s>����Ҫ�����Ĺؼ��ʣ�", src[ans1 * 2 - 1]);

        fgets(player->search_buf, sizeof player->search_buf, stdin);
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 1);
        if (strlen(player->search_buf) > 45) {
            printf("    �����ַ�̫����");
            Thread_Sleep(500);
        } else if (strlen(string_strip(player->search_buf)) == 0) {
            printf("    ����Ϊ�գ�");
            Thread_Sleep(500);
        } else
            break;
    }

    if (!strcmp("q", player->search_buf) || !strcmp("Q", player->search_buf))
        p = "�˳�", s = 1;
    if (!strcmp("s", player->search_buf) || !strcmp("S", player->search_buf))
        p = "���ص���ʽ���", s = 2;

    while (s) {
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 1);
        printf("                                                            ");
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 1);
        printf("    %s?<y> ����������%s��? <n> (Ĭ��y) ", p, player->search_buf);
        fgets(input_buf, sizeof input_buf, stdin);
        if (input_buf[0] == '\n' || ((input_buf[0] == 'y' || input_buf[0] == 'n') && input_buf[1] == '\n')) {
            if (input_buf[0] == 'n')
                ans2 = 1;
            else
                ans2 = 2;
            break;
        }
        Console_SetCurPos(PLAYER_LIST_X, PLAYER_LIST_Y + 2);
        printf("    ��Ч�����룡 <y or n>");
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

    printf("    ������%s���� ... ", player->search_buf);
    MusicList_Free(player->search_list);
    player->search_list = YylerPlayer_GetListBySearchNet(player, src[2 * (ans1 - 1)], player->search_buf);
    if (player->search_list == NULL) 
        YylerPlayer_PrintError(player, "��������ʧ��", "");
}

MusicList YylerPlayer_GetListBySearchNet(YylerPlayer *player, const char *src, const char *words)
{
    char url_buf[1024] = { 0 };
    static char buf[1024000];
    MusicList list;
    HttpClient client;

    cJSON *body, *data, *tmp;

    sprintf(buf, "��%s��", words);
    list = MusicList_New(buf);
    if (!list) {
        YylerPlayer_PrintError(player, "", "�����赥����ʧ��");
        return NULL;
    }
    Console_GBKToUTF8(url_buf, words);
    Socket_URLEncode(buf, url_buf);
    sprintf(url_buf, SEACH_URL(src, "song", buf));
    //YylerPlayer_ShowPrompt(player, NO_KEEP, "�������� %s ...", url_buf); //getchar();
    if ((client = Socket_HttpGet(url_buf, NULL)) == NULL) {
        YylerPlayer_PrintError(player, "����ʧ��", Socket_GetLastError());
        free(list);
        return NULL;
    }
    if (client->body == NULL) {
        YylerPlayer_PrintError(player, "����ʧ��2", Socket_GetLastError());
        free(list);
        Socket_FreeHttpClient(client);
        return NULL;
    }
    Console_UTF8ToGBK(buf, client->body);
    //YylerPlayer_ShowPrompt(player, NO_KEEP, "%s\n", client->header);
    //printf(buf);
    if ((body = cJSON_Parse(buf)) == NULL) {
        YylerPlayer_PrintError(player, "������Ϣ����ʧ��", "�޷�������ȡ����Ϣ");
        free(list);
        Socket_FreeHttpClient(client);
        return NULL;
    }

    if ((tmp = cJSON_GetObjectItem(body, "result")) && !strcmp(tmp->valuestring, "SUCCESS") &&
        (tmp = cJSON_GetObjectItem(body, "code")) && tmp->valueint == 200) {
        int i, size;
        
        YylerPlayer_ShowPrompt(player, NO_KEEP, "�����ɹ�");
        
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
    printf("\r���ȣ�%d%% ", n);
    //YylerPlayer_ShowPrompt(__yyler_player_bak, NO_KEEP, "������ ... %%%d  ", n);
}

int YylerPlayer_PlayerNetWorkMusic(YylerPlayer *player, MusicNode music)
{
    HttpClient client;
    FILE *fp;
    MusicNode node;

    assert(player);
    assert(music);

    YylerPlayer_ShowPrompt(player, KEEP, "�������ظ�������%s - %s�� ...", music->name, music->singer);
    Thread_Sleep(500);
    YylerPlayer_ShowPrompt(player, KEEP, " ");
    sprintf(player->buf, "%s&" URL_BR, music->name_url);
    __yyler_player_bak = player;
    client = Socket_HttpGet(player->buf, __download_callback_func);
    __yyler_player_bak = NULL;
    if (!client || !client->body) {
        YylerPlayer_PrintError(player, "��������ʧ��", "");
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

/* ������ʾ��ʾ�� */
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

    /* ����֮ǰ������ */    
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
    /* ��ʾ������ */
    printf(player->prompt_buf);
    player->last_err_len = strlen(player->prompt_buf);
    player->prompt_show = keep ? -1 : 0;
}

/* ��ӡ������Ϣ */
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
    printf("����: ");
    Console_ResetColor();
    printf("%s: ", str1);
    Console_SetFrontColor(COLOR_LIGHT_RED);
    printf("%s\n", str2);
    Console_ResetColor();
    player->last_err_len = strlen(player->argv[0]) + strlen(str1) + strlen(str2) + 10;
}


/* ���������ʾ�����ļ��� */
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
        strcpy(name + PLAYER_TITLE_SIZE, "������");
        strcat(name, list->node->name);
        strcat(name, "        ���֣�");
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

/* ��ʾ�������ŵĽ�������ʱ�� */
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
    sprintf(player->buf, "�赥������%s", list->name);
    print(PLAYER_LIST_INFO_COLOR, PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y,
        PLAYER_WIDTH - PLAYER_LIST_INFO_X - 5, 0, player->buf);
    printf("��");
    sprintf(player->buf, "�ļ�������%s", list->node->file_name);
    print(PLAYER_LIST_INFO_COLOR, PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y + 1,
        PLAYER_WIDTH - PLAYER_LIST_INFO_X - 5, 0, player->buf);
    printf("��");
    Console_SetCurPos(PLAYER_LIST_INFO_X, PLAYER_LIST_INFO_Y + 2);
    printf("��ǰ�ǵ�%d�ף���%d��", list->node->id, list->music_cnt);
}

/* ��ʾ���и�� */
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
        printf(">>> δ�ҵ��ø����ĸ�ʡ�");
    }
}

/* ���½���ĸ����ʾ */
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

/* ��ȡ�����ļ��� */
static void  get_list_by_cmdline(YylerPlayer *player)
{
    const char *args;
    char **file_names;
    int cnt = 0;

    assert(player);

    file_names = (char **)malloc(LIST_MAX_CNT * sizeof(char *));
    if (file_names == NULL) {
        printf("�ڴ����\n");
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
            sprintf(player->err_buf1, "��%s��", args);
            YylerPlayer_PrintError(player, player->err_buf1, "�޷�ʶ���ѡ��");
            printf(" ʹ�á�--help���鿴����\n");
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
                YylerPlayer_ShowPrompt(player, NO_KEEP, " ����ļ���%s�� ...", file_names[i]);
            } else {
                sprintf(player->err_buf1, "�ļ���%s�����ʧ��", file_names[i]);
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
            YylerPlayer_PrintError(player, args, "-r, --repeat  ѡ��Ĳ����Ƿ�");
            printf(" ʹ�á�--help���鿴����\n");
            exit(-1);
        }
    }
}

static void cmdline_get_volume(YylerPlayer *player, const char *args)
{
    char *end;
    if (args == "") {
        YylerPlayer_PrintError(player, "-V, --volume", "ѡ��ȱ�ٲ���");
        printf(" ʹ�á�--help���鿴����\n");
        exit(-1);
    }  else {
        int volume = strtol(args, &end, 0);
        if (end == args || volume > 100) {
            YylerPlayer_PrintError(player, args, "-V, --volume  ѡ��Ĳ����Ƿ�");
            printf(" ʹ�á�--help���鿴����\n");
            exit(-1);
        }
        player->cmdline_volume = volume;
    }
}

static void cmdline_get_volume_change(YylerPlayer *player, const char *args)
{
    char *end;
    if (args == "") {
        YylerPlayer_PrintError(player, "--volume-change", "ѡ��ȱ�ٲ���");
        printf(" ʹ�á�--help���鿴����\n");
        exit(-1);
    } else {
        int change = strtol(args, &end, 0);
        if (end == args || change < 1 || change > 49) {
            YylerPlayer_PrintError(player, args, "--volume-change  ѡ��Ĳ����Ƿ�");
            printf(" ʹ�á�--help���鿴����\n");
            exit(-1);
        }
        player->cmdline_volume_change = change;
    }
}

static void cmdline_get_time_change(YylerPlayer *player, const char *args)
{
    char *end;
    if (args == "") {
        YylerPlayer_PrintError(player, "--time-change", "ѡ��ȱ�ٲ���");
        printf(" ʹ�á�--help���鿴����\n");
        exit(-1);
    } else {
        int change = strtol(args, &end, 0);
        if (end == args || change < 1 || change > 60) {
            YylerPlayer_PrintError(player, args, "--time-change  ѡ��Ĳ����Ƿ�");
            printf(" ʹ�á�--help���鿴����\n");
            exit(-1);
        }
        player->cmdline_time_change = change;
    }
}

static void cmdline_get_list(YylerPlayer *player, const char *args)
{
    if (args == "") {
        YylerPlayer_PrintError(player, "-l, --list", "ѡ��ȱ�ٲ���");
        printf(" ʹ�á�--help���鿴����\n");
        exit(-1);
    } else {
        strncpy(player->cmd_list, args, sizeof player->cmd_list);
        player->cmd_list[sizeof player->cmd_list - 1] = 0;
    }
}


/* ���������� */
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

    /*���ƽ��� */
    //YylerPlayer_ShowFace(player);
    /* ��ȡ�����ļ��� */
    get_list_by_cmdline(player);
    /* ��������ļ����͸����б� */
    if (*player->cmd_list)
        strcpy(player->list->name, player->cmd_list);
    if (!player->have_stdin && *player->cmd_list) {
        /* �������ļ��������赥 */
        sprintf(player->err_buf2, "δ�ҵ��赥��%s��", player->cmd_list);
        YylerPlayer_PrintError(player, "�޷����Ÿ���", player->err_buf2);
        YylerPlayer_Quit(player, 0, -1);
    } else if (!player->list->music_cnt) {
        Thread_Sleep(1000);
        sprintf(player->err_buf1, "�赥��%s������ʧ��", player->list->name);
        YylerPlayer_PrintError(player, player->err_buf1, "�ø赥���޿ɲ��ŵ�����");
        YylerPlayer_Quit(player, 0, -1);
    } 
}

/* �������������� */
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
                YylerPlayer_ShowPrompt(player, KEEP, " --����ͣ-- ");
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
            YylerPlayer_ShowPrompt(player, NO_KEEP, " ���²���");
            Music_Play(list->node->music, 1, 0, 0);
            Lyric_GotoCurrentTime(list->node->lyric, 0);
            __print_lyric_lines(player, list);
            break;
        case 'P':
            if (player->repeat_one) {
                YylerPlayer_ShowPrompt(player, NO_KEEP, " ��ȡ���Ե�ǰ������ѭ������");
                player->repeat_one = 0;
            } else {
                YylerPlayer_ShowPrompt(player, NO_KEEP, " ��ѭ�����ŵ�ǰ����");
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
                YylerPlayer_ShowPrompt(player, NO_KEEP, " ������ %d%%    ", list->status.volume);
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
                YylerPlayer_ShowPrompt(player, 0, " ���ǵ�һ�ף�");
                break;
            }
            list->status.control_cmd = CTRL_LAST;
            YylerPlayer_ShowPrompt(player, 0, " ��һ��");
            break;
        case '.':
            if (list->node->id == list->music_cnt && !player->cmd_shuffle) {
                YylerPlayer_ShowPrompt(player, 0, " �������һ�ף�");
                break;
            }
            list->status.control_cmd = CTRL_NEXT;
            YylerPlayer_ShowPrompt(player, 0, " ��һ��");
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
                YylerPlayer_ShowPrompt(player, NO_KEEP, " ���%d��", player->cmdline_time_change);
            } else {
                if (list->status.time < (MTLen)player->cmdline_time_change * 1000)
                    list->status.time = 1;
                else
                    list->status.time -= player->cmdline_time_change * 1000;
                YylerPlayer_ShowPrompt(player, NO_KEEP, " ����%d��", player->cmdline_time_change);
                Lyric_GotoCurrentTime(list->node->lyric, list->status.time);
                __print_lyric_lines(player, list);
            }
            Music_Play(list->node->music, list->status.time, 0, 0);
            /* �����ǰ������ͣ״̬���ĳɲ�����״̬ */
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
        sprintf(player->err_buf1, "�޷����Ÿ����ļ���%s��", list->node->file_path);
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
    YylerPlayer_ShowPrompt(player, NO_KEEP, " ��ʼ���ţ� %s", list->node->name);
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

/* ��������ʼ�� */
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

/* �˳������� */
void YylerPlayer_Quit(YylerPlayer *player, int clear, int _quit)
{
    int i, j;

    assert(player);
    MusicList_Free(player->list);
    Console_ResetColor();
   
    /* �������� */
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
    /* �˳����� */
    exit(_quit);
}
