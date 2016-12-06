#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FILENAME "database"
#define VECT_LEN 20
#define NAME_LEN 100
#define MIN_LEN  5
#define MAX_DIFF 666

struct vector {
    double val[VECT_LEN];
    struct vector *vect;
};

struct name {
    char   name[NAME_LEN];
    struct vector *vect;
};

/* this function fill structs from file */
int fill_struct(struct name **names, char *data, int size)
{
    int curr = 0; /* current position of the file */
    int num = 0; /* current number of names */
    int maxnum = 0; /* current max number of names */
    int name_size = sizeof(struct name);

    while (curr < size) {
        if (data[curr] == 'n') {
            num++; /* increase number of names */
            if (num > maxnum) {
                maxnum += MIN_LEN; /* allocate memory for names (+5 names) */
                *names = (struct name *) realloc(*names, maxnum*name_size);
            }

            if (*names == NULL)
                return -1;

            curr += 2;
            int i;
            for (i = 0; i < NAME_LEN; i++) /* copy name until '\n' */
                if (data[curr + i] == '\n')
                    break;
                else
                    (*names)[num - 1].name[i] = data[curr + i];

            curr += i;
            (*names)[num - 1].vect = NULL;
        }

        if (data[curr] == 'v') {
            struct vector **ptr;

            ptr = &((*names)[num - 1].vect);
            while (*ptr != NULL) /* find first NULL pointer */
                ptr = &((*ptr)->vect);

            *ptr = (struct vector *) malloc(sizeof(struct vector));
            if (*ptr == NULL)
                return -1;

            (*ptr)->vect = NULL;

            int i;
            char *temp = data + curr + 1;
            for (i = 0; i < VECT_LEN; i++) /* getting values from database */
                (*ptr)->val[i] = strtod(temp, &temp);

            curr = temp - data;
        }

        curr++;
    }

    /* freeing unused memory*/
    if (num != maxnum)
        *names = (struct name *) realloc(*names, num*name_size);

    return num;
}

/* this function gets info from database */
int read_file(char *fname, struct name **names)
{
    struct stat params;
    int status;

    status = stat(fname, &params);
    if (status == -1)
        return -1; /* error reading file stats */

    char *data;
    data = (char *) malloc(params.st_size);
    if (data == NULL)
        return -1; /* can't allocate memory */

    int file;
    file = open(fname, O_RDONLY);
    if (file == -1)
        return -1; /* can't open file */

    status = read(file, data, params.st_size);
    close(file);
    if (status == -1)
        return -1; /* can't read file */

    int num = fill_struct(names, data, params.st_size);

    free(data);
    return num; /* number of different names */
}

/* this function frees all the allocated data */
void free_struct(struct name *names, int num)
{
    int i;
    for (i = 0; i < num; i++) {
        struct vector *ptr1;
        ptr1 = names[i].vect;

        while (ptr1 != NULL) {
            struct vector *ptr2;
            ptr2 = ptr1->vect;
            free(ptr1);
            ptr1 = ptr2;
        }
    }
    if (num)
        free(names);
}

/* this function writes vector to a file */
void print_vect(FILE *file, double *val)
{
    int i;
    fprintf(file, "v");
    for (i = 0; i < VECT_LEN; i++)
        fprintf(file, " %f", val[i]);
    fprintf(file, "\n");
}

/* this function writes struct to database */
void write_file(char *fname, struct name *names, int num)
{
    int i;
    FILE *file;
    file = fopen(fname, "w");

    for (i = 0; i < num; i++) {
        fprintf(file, "n %s\n", names[i].name);

        struct vector *ptr;
        ptr = names[i].vect;

        while (ptr != NULL) {
            print_vect(file, ptr->val);
            ptr = ptr->vect;
        }
    }

    fclose(file);
}

/*
 * this function ask where to add vector
 * return value:
 * -1 - delete current vector
 * [0, numbers of names) - add to that name
 * = number of names - add new name
 */
int select_name(struct name *names, int num)
{
    printf("Choose your option where to add this record:\n");

    int i;
    for (i = 0; i < num; i++)
        printf("%d) add to \"%s\";\n", i, names[i].name);

    printf("%d) add to a new name;\n", i++);
    printf("%d) delete this record.\n", i);
    printf("Write the number of your choise: ");
    scanf ("%d%*c", &i);

    if (i <= num)
        return i;
    else
        return -1;
}

/* this function adds a new name to names */
int add_name(struct name **names, int num)
{
    *names = (struct name *) realloc(*names, (++num)*sizeof(struct name));
    if (*names == NULL)
        return -1;

    (*names)[num - 1].vect = NULL;
    printf("Write name: ");
    fflush(stdout);
    fgets((*names)[num - 1].name, NAME_LEN, stdin);

    int i;
    for (i = 0; i < NAME_LEN; i++)
        if ((*names)[num - 1].name[i] == '\n') {
            (*names)[num - 1].name[i] = 0;
            break;
        }

    return num;
}

/* this function adds a new vector to the name with number 'where' */
int add_vector(struct name *names, int where, double *vect)
{
    struct vector **ptr;
    ptr = &(names[where].vect);

    while (*ptr != NULL)
        ptr = &((*ptr)->vect);

    *ptr = (struct vector *) malloc(sizeof(struct vector));
    if (*ptr == NULL)
        return -1;

    int i;
    for (i = 0; i < VECT_LEN; i++)
        (*ptr)->val[i] = vect[i];
    (*ptr)->vect = NULL;

    return 0;
}

/* this function updates database with new vector */
int update_database(double *vector)
{
    struct name *names = NULL;
    int num;
    int status;

    num = read_file(FILENAME, &names);
    if (num == -1) {
        printf("Can't read database\n");
        num = 0;
    }

    status = select_name(names, num);
    if (status == -1) {
        printf("This record was deleted\n");
        return 0;
    }

    if (status == num)
        num = add_name(&names, num);
    if (num == -1) {
        printf("Can't add new name to database\n");
        return -1;
    }

    status = add_vector(names, status, vector);
    if (status == -1)
        return -1;

    write_file(FILENAME, names, num);

    free_struct(names, num);

    return 0;
}

/* this function calculates differense between two vectors */
double get_diff(double *arr1, double *arr2)
{
    double res = 0;
    int i;
    for (i = 0; i < VECT_LEN; i++)
        res += fabs(arr1[i] - arr2[i]);
    return res;
}

/* this function compares new vector with all vectors from database */
int compare_vectors(double *vector, struct name *names, int num)
{
    double min_diff = MAX_DIFF;
    double diff;
    int res = -1;

    int i;
    for (i = 0; i < num; i++) {
        struct vector *ptr;
        ptr = names[i].vect;

        while (ptr != NULL) {
            diff = get_diff(ptr->val, vector);
            if (diff < min_diff) {
                min_diff = diff;
                res = i;
            }

            ptr = ptr->vect;
        }
    }

    return res;
}

/* this function determines who was talking */
int determine_name(double *vector)
{
    int status;
    int num;
    struct name *names = NULL;

    num = read_file(FILENAME, &names);
    if (num == -1) {
        printf("Can't read database\n");
        return -1;
    }

    status = compare_vectors(vector, names, num);
    if (status == -1)
        printf("Noone matchs =(\n");
    else
        printf("This is %s. I'am sure!\n", names[status].name);

    free_struct(names, num);

    return 0;
}
