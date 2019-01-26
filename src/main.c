#include "YylerPlayer.h"
#include <signal.h>

YylerPlayer player;

void quit_by_ctrl_c(int sig)
{
    YylerPlayer_Quit(&player, 0, 0);
} 

int main(int argc, char *argv[])
{
    signal(SIGINT, quit_by_ctrl_c);
    YylerPlayer_Init(&player, argc, argv);
    YylerPlayer_PlayMusicList(&player, player.list);
    YylerPlayer_Quit(&player, 1, 0);
 
    return 0;
}
