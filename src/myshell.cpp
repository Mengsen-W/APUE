/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 09:54:11
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 10:26:25
 * @Description: my shell
 */

#include <glob.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#define DELIMS " \t\n"

struct cmd_st {
  glob_t globres;
};

static void prompt(void) { std::cout << "mysh-0.1 $"; }
static void parse(char* line, struct cmd_st* res) {
  char* tok;
  int i = 0;
  while (1) {
    tok = strsep(&line, DELIMS);
    if (tok == nullptr) break;
    if (tok[0] == '\0') continue;
    glob(tok, GLOB_NOCHECK | GLOB_APPEND * i, NULL, &res->globres);
    i = 1;
  }
}

int main() {
  char* linebuf = nullptr;
  size_t linebuf_size = 0;
  struct cmd_st cmd;
  pid_t pid;
  while (1) {
    prompt();
    if (getline(&linebuf, &linebuf_size, stdin) < 0) break;
    parse(linebuf, &cmd);
    if (0) {
    } else /*外部命令*/
    {
      pid = fork();
      if (pid < 0) {
        perror("fork()");
        exit(1);
      }
      if (pid == 0) {
        execvp(cmd.globres.gl_pathv[0], cmd.globres.gl_pathv);
        perror("execvp()");
        exit(1);
      } else {
        wait(NULL);
      }
    }
  }
  return 0;
}