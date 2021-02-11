/* Phase 1 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h> // importing string.h... are we allowed to use this?

#define MAX_LINES 512

struct job // job linkedlist node
{
    int id;
    int length;
    // other meta data to be added?
    struct job *next;
};

int main(int argc, char **argv)
{
    struct job *head = NULL;
    struct job *current = NULL;
    char fileLines[5][MAX_LINES]; // setting a max of 512 lines from input file, don't think it'll exceed
    char *policy = argv[1];
    char *testFile = argv[2];
    char *timeSlice = argv[3];
    int timeSliceInt = atoi(timeSlice); // THIS IS CAUSING A SEG FAULT AHHHH

    /* Insert job at end of linked list as opposed to at start (first in) */
    void appendJob(struct job * *head, int id, int length)
    {
        /* make new node */
        struct job *link = (struct job *)malloc(sizeof(struct job));

        struct job *last = *head;

        /* add data */
        link->id = id;
        link->length = length;

        /* will be last node */
        link->next = NULL;

        /* if list empt, new node is head */
        if (*head == NULL)
        {
            *head = link;
            return;
        }

        /* otherwise go till last node */
        while (last->next != NULL)
            last = last->next;

        /* change last node next */
        last->next = link;
        return;
    }

    void openFile()
    {
        int idCounter = 0; // use this value to iterate backwards later on
        FILE *fp = fopen(testFile, "r");
        int curLine = 0;

        // I'm storing data in reverse here, keep that in mind!
        if (fp != NULL)
        {

            while ((fgets(fileLines[idCounter], MAX_LINES, fp) != NULL))
            {
                appendJob(&head, idCounter, atoi(fileLines[idCounter]));
                idCounter++;
            }
        }
        else
        {
            printf("Could not find specified file. Check your argument and try again.\n");
            exit(1);
        }
    }

    // if not providing between 3 and 4 arguments, scream!
    if (!(argc >= 3 && argc <= 4))
    {
        printf("Requires at least 2 arguments: Policy, tests/x.in.\n");
        return 0;
    }

    // // time slice is invalid, should be between 0 and 100 realistically
    if (!(timeSliceInt >= 0 && timeSliceInt < 100))
    {
        printf("Time slice value invalid. Choose between 0 and 100.\n");
        return 0;
    }

    /* Open le file, fill le array */
    openFile();

    // printJobs();
    printf("Execution trace with %s:\n", policy);
    // switch case with if else, not working with ints
    if (strcmp(policy, "FIFO") == 0)
    {
        int idCounter = 0; // use this value to iterate backwards later on
        struct job *ptr = head;
        while (ptr != NULL)
        {
            printf("Job %d ran for: %d\n", ptr->id, ptr->length);
            ptr = ptr->next;
            idCounter--;
        }
    }

    else if (strcmp(policy, "SJF") == 0)
    {
        printf("SJF\n");
    }

    else if (strcmp(policy, "RR") == 0)
    {
        if (argc != 4)
        {
            printf("You must provide a time slice value for RR\n");
            return 0;
        }
        printf("RR\n");
    }

    else
    {
        printf("Enter FIFO SJF or RR as the policy\n");
    }
    printf("End of execution with %s.\n", policy);

    return 0;
    // printf("Hello, please help me schedule!");
}