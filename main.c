#include <libc.h>
#include <stdbool.h>
#define fatal "error: fatal\n"
#define cd_args "error: cd: bad arguments\n"
#define cd_path "error: cd: cannot change directory to "
#define exec "error: cannot execute "

int error(char *s) {
    while (*s)
        write(2, s++, 1);
    return 1;
}

void cd(char **cmd_list) {
    if (!cmd_list[1] || cmd_list[2]) {
        error(cd_args);
        return ;
    }
    if (chdir(cmd_list[1]) < 0) {
        error(cd_path);
        error(cmd_list[1]);
        error("\n");
    }
}

int main(int ac, char **av, char **envp) {
    if (ac == 1)
        return 0;
    char *cmd_list[100000] = {NULL};
    int i = 0;
    int nbr;
    int tmp;
    int fd[2];
    tmp = dup(0);

    do {
        i++;
        nbr = 0;
        for (; i < ac && strcmp(av[i], ";"); i++) {
            int j = 0;
            while ( i + j < ac && strcmp(av[i + j], ";") && strcmp(av[i + j], "|") ) {
                cmd_list[j] = av[i + j];
                j++;
            }
            i += j;
            cmd_list[j] = NULL;
            if (cmd_list[0] && !strcmp(cmd_list[0], "cd"))
            {
                cd(cmd_list);
                continue;
            }
            if (av[i] && !strcmp(av[i], "|"))
                pipe(fd);
            else if (av[i] && !strcmp(av[i], ";"))
            {
                i--;
            }
            nbr++;
            int id = fork();
            if (id < 0)
                return error(fatal);
            if (id == 0) {

                if (av[i] && !strcmp(av[i], "|")) {
                    close(1);
                    dup2(fd[1], 1);
                    close(fd[0]), close(fd[1]);
                }
                execve(cmd_list[0], cmd_list, envp);
                error(exec);
                error(cmd_list[0]);
                error("\n");
                return 1;
            } else {
                if (av[i] && !strcmp(av[i], "|")) {
                    close(0);
                    dup2(fd[0], 0);
                    close(fd[0]), close(fd[1]);
                }
            }
        }
        for (int k = 0; k < nbr; k++)
            waitpid(-1, NULL, 0);
        dup2(tmp, 0);
    }
    while (av[i] && !strcmp(av[i], ";"));
    close(tmp);
}