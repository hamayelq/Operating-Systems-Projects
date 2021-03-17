#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define MAX_CHAR 255

#define JUGGLER_TOT 8
#define FLAMENCO_TOT 15
#define SOLOISTS_TOT 2

#define JUGGLER 0
#define FLAMENCO 1
#define SOLOIST 2

#define STAGE_POSITIONS 4

struct performer
{
    char *typeName;  // "FDAN"/"JGLR"/"SOLO"
    int id;          // simple iterator id
    int performType; // type of performer but number
} performer;

struct stagePosition
{
    char *positionName;              // Pos 0/1/2/3
    int isFree;                      // is the curr pos free
    struct performer *currPerformer; // store the current performer at the stage position
} stagePosition;

struct stage
{
    int onStageTotal;                       // how many performers are on stage?
    struct stagePosition stagePositions[4]; // array of stage positions (4 stage positions on a stage)
} stage;

/* get the seed from text file */
int getSeed()
{
    char seedBuffer[MAX_CHAR];         // char array to store seed from seed.text
    int randSeed;                      // the seed to be converted to int
    FILE *fp = fopen("seed.txt", "r"); // file pointer, set to read only

    // check if can't read file
    if (fp != NULL)
    {
        if (fgets(seedBuffer, MAX_CHAR, fp) != NULL)
        {
            fclose(fp);
            randSeed = atoi(seedBuffer);
            return randSeed;
        }
        else
        {
            printf("Seed file is empty! Exiting...\n"); //hjhh
            exit(1);
        }
    }
    else
    {
        printf("Cannot read from seed file! Exiting...\n");
        exit(1);
    }
}

/* deprecated function to check if performers were correctly created */
// void printPerformers(int performerType)
// {
//     switch (performerType)
//     {
//     case 0:
//         for (int i = 0; i < JUGGLER_TOT; i++)
//         {
//             printf("%s #%d %d\n", jugglers[i].typeName, jugglers[i].id, jugglers[i].performType);
//         }
//         break;
//     case 1:
//         for (int i = 0; i < FLAMENCO_TOT; i++)
//         {
//             printf("%s #%d %d\n", flamencos[i].typeName, flamencos[i].id, flamencos[i].performType);
//         }
//         break;
//     case 2:
//         for (int i = 0; i < SOLOISTS_TOT; i++)
//         {
//             printf("%s #%d %d\n", soloists[i].typeName, soloists[i].id, soloists[i].performType);
//         }
//         break;
//     }
// }