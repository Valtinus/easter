#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#define MAX_VERS_LENGTH 100
#define FILE_NAME "versek.txt"

void addPoem();
void listPoems();
void deletePoem();
void modifyPoem();
void sendPoem(int pipefd[2]);
void receivePoems(int pipefd[2], char *poems[2]);
void showChosenPoems(char *poems[2]);
void deleteChosenPoem(const char *poemToDelete);
void locsolas();
void handler(int signum);

int main()
{
    srand(time(NULL));
    int choice;

    do
    {
        printf("\n------------------------------\n");
        printf("1. Új vers hozzáadása\n");
        printf("2. Versek kilistázása\n");
        printf("3. Vers törlése\n");
        printf("4. Vers módosítása\n");
        printf("5. Locsolás\n");
        printf("0. Kilépés\n");
        printf("------------------------------\n");
        printf("Válassz egy lehetőséget: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            addPoem();
            break;
        case 2:
            listPoems();
            break;
        case 3:
            deletePoem();
            break;
        case 4:
            modifyPoem();
            break;
        case 5:
            locsolas();
            break;
        case 0:
            printf("Kilépés\n");
            break;
        default:
            printf("Érvénytelen!\n");
            break;
        }
    } while (choice != 0);

    return 0;
}

void addPoem()
{
    char poem[MAX_VERS_LENGTH];
    printf("Írd be az új verset: ");
    getchar();
    fgets(poem, MAX_VERS_LENGTH, stdin);

    FILE *file = fopen(FILE_NAME, "a");
    if (file == NULL)
    {
        file = fopen(FILE_NAME, "w");
        if (file == NULL)
        {
            printf("Hiba a fájl létrehozásakor.\n");
            return;
        }
    }

    fprintf(file, "%s", poem);
    fclose(file);

    printf("Vers hozzáadva a listához.\n");
}

void listPoems()
{
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL)
    {
        printf("Nincs ilyen fájl.\n");
        return;
    }

    char poem[MAX_VERS_LENGTH];
    int count = 0;
    printf("\nVersek: \n");
    while (fgets(poem, MAX_VERS_LENGTH, file) != NULL)
    {
        printf("%d - %s", count + 1, poem);
        count++;
    }

    fclose(file);

    if (count == 0)
    {
        printf("Még nincsenek versek.\n");
    }
}

void deletePoem()
{
    listPoems();
    int n;
    printf("Írd be melyik verset szeretnéd törölni: ");
    scanf("%d", &n);

    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL)
    {
        printf("Nincs ilyen fájl.\n");
        return;
    }

    char poems[MAX_VERS_LENGTH][MAX_VERS_LENGTH];
    int count = 0;
    while (fgets(poems[count], MAX_VERS_LENGTH, file) != NULL)
    {
        count++;
    }
    fclose(file);

    file = fopen(FILE_NAME, "w");
    if (file == NULL)
    {
        printf("Hiba a fájl megnyitásakor.\n");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        if (i + 1 != n)
        {
            fprintf(file, "%s", poems[i]);
        }
    }
    fclose(file);
    printf("Vers törölve.\n");
}

void modifyPoem()
{
    listPoems();
    int n;
    printf("Írd be melyik verset szeretnéd módosítani: ");
    scanf("%d", &n);

    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL)
    {
        printf("Nincs ilyen fájl.\n");
        return;
    }

    char poems[MAX_VERS_LENGTH][MAX_VERS_LENGTH];
    int count = 0;
    while (fgets(poems[count], MAX_VERS_LENGTH, file) != NULL)
    {
        count++;
    }
    fclose(file);

    printf("Írd be a módosított verset: ");
    char newPoem[MAX_VERS_LENGTH];
    getchar();
    fgets(newPoem, MAX_VERS_LENGTH, stdin);

    file = fopen(FILE_NAME, "w");
    if (file == NULL)
    {
        printf("Hiba a fájl megnyitásakor.\n");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        if (i + 1 == n)
        {
            fprintf(file, "%s", newPoem);
        }
        else
        {
            fprintf(file, "%s", poems[i]);
        }
    }
    fclose(file);
    printf("Vers módosítva.\n");
}

void handler(int signum)
{
    // printf("Handler dolgozik.\n");
}

char *chooseRandomPoems()
{
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL)
    {
        perror("Hiba a fájl megnyitásakor.\n");
        exit(EXIT_FAILURE);
    }

    char *result = (char *)malloc(2 * MAX_VERS_LENGTH * sizeof(char));
    char poem1[MAX_VERS_LENGTH], poem2[MAX_VERS_LENGTH];
    int index1, index2;
    long int file_length = 0;
    int poem_count = 0;

    fseek(file, 0, SEEK_END);
    file_length = ftell(file);
    rewind(file);

    while (fgets(poem1, MAX_VERS_LENGTH, file) != NULL)
    {
        poem_count++;
    }

    index1 = rand() % poem_count;
    do
    {
        index2 = rand() % poem_count;
    } while (index2 == index1);

    rewind(file);
    for (int i = 0; i <= index2; i++)
    {
        fgets(poem2, MAX_VERS_LENGTH, file);
        if (i == index1)
        {
            fgets(poem1, MAX_VERS_LENGTH, file);
        }
    }
    fclose(file);

    strcpy(result, poem1);
    strcat(result, ";");
    strcat(result, poem2);

    return result;
}

void sendPoem(int pipefd[2])
{
    close(pipefd[0]);
    char *chosenPoems = chooseRandomPoems();

    write(pipefd[1], chosenPoems, strlen(chosenPoems) + 1);
    close(pipefd[1]);

    free(chosenPoems);
}

void receivePoems(int pipefd[2], char *poems[2])
{
    char buffer[(MAX_VERS_LENGTH * 2) + 1];
    ssize_t bytes_read;

    bytes_read = read(pipefd[0], buffer, sizeof(buffer));
    if (bytes_read == -1)
    {
        perror("Hiba az üzenet fogadásakor.\n");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);

    char *token = strtok(buffer, ";");
    int i = 0;
    while (token != NULL)
    {
        poems[i] = strdup(token);
        token = strtok(NULL, ";");
        i++;
    }
}

struct msg_buffer
{
    long msg_type;
    char msg_text[MAX_VERS_LENGTH];
};

void showChosenPoems(char *poems[2])
{
    printf("Ezeket a verseket kaptam:\n");
    for (int i = 0; i < 2; i++)
    {
        printf("%d. vers:\n%s\n", i + 1, poems[i]);
    }
}

void deleteChosenPoem(const char *poemToDelete)
{
    const char *tempFileName = "temp.txt";

    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL)
    {
        perror("Hiba a fájl megnyitásakor.\n");
        exit(EXIT_FAILURE);
    }

    FILE *tempFile = fopen(tempFileName, "w");
    if (tempFile == NULL)
    {
        perror("Hiba az ideiglenes fájl megnyitásakor.\n");
        exit(EXIT_FAILURE);
    }

    char poem[MAX_VERS_LENGTH];
    char *line;
    while ((line = fgets(poem, MAX_VERS_LENGTH, file)) != NULL)
    {
        if (strcmp(line, poemToDelete) == 0)
        {
            continue;
        }
        fprintf(tempFile, "%s", line);
    }

    fclose(file);
    fclose(tempFile);

    if (remove(FILE_NAME) != 0)
    {
        perror("Hiba az eredeti fájl törlésekor.\n");
        exit(EXIT_FAILURE);
    }
    if (rename(tempFileName, FILE_NAME) != 0)
    {
        perror("Hiba az ideiglenes fájl átnevezésekor.\n");
        exit(EXIT_FAILURE);
    }
}

void locsolas()
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("Hiba a cső létrehozásakor.\n");
        exit(EXIT_FAILURE);
    }

    struct msg_buffer msg;
    key_t key = ftok("beadando2.c", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        perror("Hiba az üzenetsor létrehozásakor.\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, handler);
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_SETMASK, &sigset, NULL);

    pid_t pids[4];
    for (int i = 0; i < 4; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            perror("Forkolás sikertelen.\n");
            exit(EXIT_FAILURE);
        }
        else if (pids[i] == 0)
        {
            if (i == rand() % 4)
            {
                kill(getppid(), SIGUSR1);
                close(pipefd[1]);

                char *poems[2];
                receivePoems(pipefd, poems);
                showChosenPoems(poems);

                int chosenPoemIndex = rand() % 2;

                msg.msg_type = 1;
                strcpy(msg.msg_text, poems[chosenPoemIndex]);
                msgsnd(msgid, &msg, sizeof(msg.msg_text), 0);

                printf("%sSzabad-e locsolni!\n", msg.msg_text);

                exit(0);
            }
            else
            {
                exit(0);
            }
        }
    }

    sendPoem(pipefd);
    close(pipefd[1]);

    sigemptyset(&sigset);
    sigsuspend(&sigset);

    msgrcv(msgid, &msg, sizeof(msg.msg_text), 1, 0);

    deleteChosenPoem(msg.msg_text);

    for (int i = 0; i < 4; i++)
    {
        waitpid(pids[i], NULL, 0);
    }

    msgctl(msgid, IPC_RMID, NULL);
}