#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include "spectacular.h"

sem_t stageSem, printSem, leaveStageSem, enterSem, napSem;
sem_t stageLock, fdancerLock, soloistLock, jugglerLock, emptySem, orderSem;

struct performer jugglers[JUGGLER_TOT] = {NULL};
struct performer flamencos[FLAMENCO_TOT] = {NULL};
struct performer soloists[SOLOISTS_TOT] = {NULL};

struct stage performanceStage;

int currType = 0;

int totalfDancers, totalJugglers;

void createStage()
{
    performanceStage.onStageTotal = 0;
    for (int i = 0; i < 4; i++)
    {
        char *positionName = i == 0 ? "Position Zero" : i == 1 ? "Position One"
                                                    : i == 2   ? "Position Two"
                                                               : "Position Three";
        performanceStage.stagePositions[i].currPerformer = NULL;
        performanceStage.stagePositions[i].isFree = 1;
        performanceStage.stagePositions[i].positionName = positionName;
    }
}

void printStage()
{
    printf("[ ");
    for (int i = 0; i < 4; i++)
    {
        if (performanceStage.stagePositions[i].currPerformer != NULL)
        {
            printf(performanceStage.stagePositions[i].currPerformer->id < 10 ? "%s #0%d " : "%s #%d ",
                   performanceStage.stagePositions[i].currPerformer->typeName,
                   performanceStage.stagePositions[i].currPerformer->id);
        }
        else
        {
            printf("________ ");
        }
    }
    printf("] ");
}

void createPerformers()
{
    for (int i = 0; i < JUGGLER_TOT; i++)
    {
        jugglers[i].id = i + 1;
        jugglers[i].typeName = "JGLR";
        jugglers[i].performType = JUGGLER;
    }

    for (int i = 0; i < FLAMENCO_TOT; i++)
    {
        flamencos[i].id = i + 1;
        flamencos[i].typeName = "FDAN";
        flamencos[i].performType = FLAMENCO;
    }

    for (int i = 0; i < SOLOISTS_TOT; i++)
    {
        soloists[i].id = i + 1;
        soloists[i].typeName = "SOLO";
        soloists[i].performType = SOLOIST;
    }
}

/* deprecated function to check if performers were correctly created */
void printPerformers(int performerType)
{
    switch (performerType)
    {
    case 0:
        for (int i = 0; i < JUGGLER_TOT; i++)
        {
            printf("%s #%d %d\n", jugglers[i].typeName, jugglers[i].id, jugglers[i].performType);
        }
        break;
    case 1:
        for (int i = 0; i < FLAMENCO_TOT; i++)
        {
            printf("%s #%d %d\n", flamencos[i].typeName, flamencos[i].id, flamencos[i].performType);
        }
        break;
    case 2:
        for (int i = 0; i < SOLOISTS_TOT; i++)
        {
            printf("%s #%d %d\n", soloists[i].typeName, soloists[i].id, soloists[i].performType);
        }
        break;
    }
}

void perform(struct performer *currPerformer)
{
    // struct performer *currPerformerPtr = currPerformer;
    char *positionName;
    int stageIndex;
    int performerId = currPerformer->id;
    char *performerTypeName = currPerformer->typeName;
    int randPerformDuration = rand() % (1000000 - 100000 + 1) + 100000; // performance duration btw 1ms to 10ms

    sem_wait(&stageSem);
    sem_wait(&stageLock);
    for (int i = 0; i < 4; i++)
    {
        if (performanceStage.stagePositions[i].isFree)
        {
            performanceStage.stagePositions[i].isFree = 0;
            performanceStage.stagePositions[i].currPerformer = currPerformer;
            performanceStage.onStageTotal++;
            positionName = performanceStage.stagePositions[i].positionName;
            stageIndex = i;
            sem_post(&stageLock);
            break;
        }
    }
    sem_post(&stageLock);

    /* after the performer joins the stage, it prints its position on the stage and performs for a random duration */
    printf("JOIN: ");
    printStage();
    printf(performerId < 10 ? "%s #0%d joined at %s, performing for %d milliseconds\n\n" : "%s #%d joined at %s, performing for %d milliseconds\n\n",
           performerTypeName, performerId, positionName, randPerformDuration);
    usleep(randPerformDuration); // performer performing for a random duration

    performanceStage.stagePositions[stageIndex].isFree = 1;
    performanceStage.stagePositions[stageIndex].currPerformer = NULL;
    performanceStage.onStageTotal--;

    printf("DONE: ");
    printStage();

    printf(performerId < 10 ? "%s #0%d is done performing at %s \n\n" : "%s #%d is done performing at %s \n\n",
           performerTypeName, performerId, positionName);
    sem_post(&stageSem);
}

void *performerFunction(void *arg)
{
    struct performer *currPerformer = ((struct performer *)arg);
    char *performerTypeName = currPerformer->typeName;
    int performerTypeInt = currPerformer->performType;
    int performerId = currPerformer->id;
    char *positionName;
    int randPerformDuration = rand() % (1000000 - 100000 + 1) + 100000; // performance duration btw 1ms to 10ms
    int stageIndex = 0;
    int isCorrect = 0;

    sem_t *lock = performerTypeInt == 0 ? &jugglerLock : performerTypeInt == 1 ? &fdancerLock
                                                                               : &soloistLock;

    if (performerTypeInt < 2)
    {
        sem_wait(&orderSem);
        sem_wait(lock);
        int *totalPerformers = performerTypeInt == 0 ? &totalfDancers : &totalJugglers;
        (*totalPerformers)++;
        if (*totalPerformers == 1)
        {
            sem_wait(&emptySem);
        }
        sem_post(&orderSem);
        sem_post(lock);

        // perform(&currPerformer);

        sem_wait(&stageSem);
        sem_wait(&stageLock);
        for (int i = 0; i < 4; i++)
        {
            if (performanceStage.stagePositions[i].isFree)
            {
                performanceStage.stagePositions[i].isFree = 0;
                performanceStage.stagePositions[i].currPerformer = currPerformer;
                performanceStage.onStageTotal++;
                positionName = performanceStage.stagePositions[i].positionName;
                stageIndex = i;
                sem_post(&stageLock);
                break;
            }
        }
        sem_post(&stageLock);

        /* after the performer joins the stage, it prints its position on the stage and performs for a random duration */
        printf("JOIN: ");
        printStage();
        printf(performerId < 10 ? "%s #0%d joined at %s, performing for %d milliseconds\n\n" : "%s #%d joined at %s, performing for %d milliseconds\n\n",
               performerTypeName, performerId, positionName, randPerformDuration);
        usleep(randPerformDuration); // performer performing for a random duration

        performanceStage.stagePositions[stageIndex].isFree = 1;
        performanceStage.stagePositions[stageIndex].currPerformer = NULL;
        performanceStage.onStageTotal--;

        printf("DONE: ");
        printStage();

        printf(performerId < 10 ? "%s #0%d is done performing at %s \n\n" : "%s #%d is done performing at %s \n\n",
               performerTypeName, performerId, positionName);
        sem_post(&stageSem);

        sem_wait(lock);
        (*totalPerformers)--;
        if (!*totalPerformers)
        {
            sem_post(&emptySem);
        }
        sem_post(lock);
    }
    else
    {
        sem_wait(&emptySem);

        perform(currPerformer);

        // sem_wait(&stageSem);

        // sem_wait(&stageLock);
        // for (int i = 0; i < 4; i++)
        // {
        //     if (performanceStage.stagePositions[i].isFree)
        //     {
        //         performanceStage.stagePositions[i].isFree = 0;
        //         performanceStage.stagePositions[i].currPerformer = currPerformer;
        //         performanceStage.onStageTotal++;
        //         positionName = performanceStage.stagePositions[i].positionName;
        //         stageIndex = i;
        //         sem_post(&stageLock);
        //         break;
        //     }
        // }
        // sem_post(&stageLock);

        // /* after the performer joins the stage, it prints its position on the stage and performs for a random duration */
        // printf("JOIN: ");
        // printStage();
        // printf(performerId < 10 ? "%s #0%d joined at %s, performing for %d milliseconds\n\n" : "%s #%d joined at %s, performing for %d milliseconds\n\n",
        //        performerTypeName, performerId, positionName, randPerformDuration);
        // usleep(randPerformDuration * 10); // performer performing for a random duration

        // performanceStage.stagePositions[stageIndex].isFree = 1;
        // performanceStage.stagePositions[stageIndex].currPerformer = NULL;
        // performanceStage.onStageTotal--;

        // printf(performerId < 10 ? "%s #0%d is done performing at %s \n\n" : "%s #%d is done performing at %s \n\n",
        //        performerTypeName, performerId, positionName);

        // sem_post(&stageSem);
        sem_post(&emptySem);
    }
}

void createThreads(pthread_t *threads, int PERFORMER_TOTAL, struct performer *performers)
{
    for (int i = 0; i < PERFORMER_TOTAL; i++)
    {
        pthread_create(&threads[i], NULL, performerFunction, &performers[i]);
    }
}

void joinThreads(pthread_t *threads, int PERFORMER_TOTAL)
{
    for (int i = 0; i < PERFORMER_TOTAL; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

int main()
{
    int randSeed = getSeed();
    printf("Random seed is: %d\n", randSeed);
    // sleep(1);
    printf("Seeding the randomizer...\n");
    // sleep(1);
    srand(randSeed); // seed the randomizer

    sem_init(&stageSem, 0, 4);
    sem_init(&leaveStageSem, 0, 1);
    sem_init(&enterSem, 0, 1);
    sem_init(&printSem, 0, 1);
    sem_init(&napSem, 0, 0);

    sem_init(&stageLock, 0, 1);
    sem_init(&fdancerLock, 0, 1);
    sem_init(&jugglerLock, 0, 1);
    sem_init(&emptySem, 0, 1);
    sem_init(&orderSem, 0, 1);

    printf("---------------------------------------------------------------\n");
    printf("Welcome to teh summer spectacular. take ur seats. bye\n");
    printf("---------------------------------------------------------------\n\n");

    printf("Stage positions are as follows:\n");
    printf("JorL: [ __POS0__ __POS1__ __POS2__ __POS3__ ]\n\n");
    printf("---------------------------------------------------------------\n\n");

    sleep(1);

    // pthread_t jugglerThreads[JUGGLER_TOT], flamenco[]
    pthread_t *jugglerThreads = (pthread_t *)malloc(JUGGLER_TOT * sizeof(pthread_t));
    pthread_t *flamencoThreads = (pthread_t *)malloc(FLAMENCO_TOT * sizeof(pthread_t));
    pthread_t *soloistThreads = (pthread_t *)malloc(SOLOISTS_TOT * sizeof(pthread_t));

    createStage();
    createPerformers();

    createThreads(jugglerThreads, JUGGLER_TOT, jugglers);
    pthread_create(&soloistThreads[0], NULL, performerFunction, &soloists[0]);
    createThreads(flamencoThreads, FLAMENCO_TOT, flamencos);
    pthread_create(&soloistThreads[1], NULL, performerFunction, &soloists[1]);

    // createThreads(soloistThreads, SOLOISTS_TOT, soloists);

    joinThreads(jugglerThreads, JUGGLER_TOT);
    pthread_join(soloistThreads[0], NULL);
    joinThreads(flamencoThreads, FLAMENCO_TOT);
    pthread_join(soloistThreads[1], NULL);

    // joinThreads(soloistThreads, SOLOISTS_TOT);

    printf("---------------------------------------------------------------\n");
    printf("woohoo its over yay\n");
    printf("---------------------------------------------------------------\n");

    free(jugglerThreads);
    free(flamencoThreads);
    free(soloistThreads);

    return 0;
}
