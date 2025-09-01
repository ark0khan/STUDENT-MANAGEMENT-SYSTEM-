#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

enum { MAX_BOOKS = 100 };

typedef struct {
    char title[100];
    char genre[100];
    int year;
} Book;

Book inventory[MAX_BOOKS];
int book_count = 0;

pthread_mutex_t inventory_mutex;
sem_t sort_sem;

typedef struct {
    int start;
    int end;
    int criterion;
} SortArgs;

int current_criterion = 0;

int compare_books(const void *a, const void *b) {
    const Book *b1 = (const Book*)a;
    const Book *b2 = (const Book*)b;
    if (current_criterion == 0) {
        return strcmp(b1->title, b2->title);
    } else if (current_criterion == 1) {
        return strcmp(b1->genre, b2->genre);
    } else {
        return b1->year - b2->year;
    }
}

void *sort_thread_func(void *arg) {
    SortArgs *args = (SortArgs*)arg;
    int start = args->start;
    int end = args->end;
    if (start < 0) start = 0;
    if (end >= book_count) end = book_count - 1;
    int n = end - start + 1;
    if (n > 0) {
        qsort(&inventory[start], n, sizeof(Book), compare_books);
    }
    sem_post(&sort_sem);
    return NULL;
}

void load_inventory(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        char *title = strtok(line, ",");
        char *genre = strtok(NULL, ",");
        char *year_str = strtok(NULL, ",");
        if (title && genre && year_str && book_count < MAX_BOOKS) {
            strncpy(inventory[book_count].title, title, 99);
            inventory[book_count].title[99] = '\0';
            strncpy(inventory[book_count].genre, genre, 99);
            inventory[book_count].genre[99] = '\0';
            inventory[book_count].year = atoi(year_str);
            book_count++;
        }
        if (book_count >= MAX_BOOKS) break;
    }
    fclose(file);
}

void save_inventory(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not open file for writing.\n");
        return;
    }
    for (int i = 0; i < book_count; i++) {
        fprintf(file, "%s,%s,%d\n",
                inventory[i].title,
                inventory[i].genre,
                inventory[i].year);
    }
    fclose(file);
}

void sort_books() {
    if (book_count <= 1) {
        printf("Not enough books to sort.\n");
        return;
    }
    printf("Sort by: 0-Title, 1-Genre, 2-Year: ");
    int option;
    if (scanf("%d", &option) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    if (option < 0 || option > 2) {
        printf("Invalid sort criterion.\n");
        return;
    }
    current_criterion = option;
    int mid = book_count / 2;
    SortArgs args1 = {0, mid - 1, option};
    SortArgs args2 = {mid, book_count - 1, option};
    sem_init(&sort_sem, 0, 0);
    pthread_mutex_lock(&inventory_mutex);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, sort_thread_func, &args1);
    pthread_create(&t2, NULL, sort_thread_func, &args2);
    sem_wait(&sort_sem);
    sem_wait(&sort_sem);
    int i = args1.start;
    int j = args2.start;
    Book *temp = (Book*)malloc(book_count * sizeof(Book));
    int k = 0;
    while (i <= args1.end && j <= args2.end) {
        if (compare_books(&inventory[i], &inventory[j]) <= 0) temp[k++] = inventory[i++];
        else temp[k++] = inventory[j++];
    }
    while (i <= args1.end) temp[k++] = inventory[i++];
    while (j <= args2.end) temp[k++] = inventory[j++];
    for (int m = 0; m < k; m++) inventory[m] = temp[m];
    free(temp);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_unlock(&inventory_mutex);
    printf("Books sorted successfully.\n");
}

void display_books() {
    pthread_mutex_lock(&inventory_mutex);
    if (book_count == 0) {
        printf("No books in inventory.\n");
    } else {
        printf("\nCurrent Books (%d):\n\n", book_count);
        printf("S.No  %-25s %-15s %4s\n", "Title", "Genre", "Year");
        printf("----- ------------------------- --------------- ----\n");
        for (int i = 0; i < book_count; i++) {
            printf("%4d  %-25s %-15s %4d\n",
                   i + 1,
                   inventory[i].title,
                   inventory[i].genre,
                   inventory[i].year);
        }
    }
    pthread_mutex_unlock(&inventory_mutex);
}

void add_book() {
    pthread_mutex_lock(&inventory_mutex);
    if (book_count >= MAX_BOOKS) {
        printf("Bookshelf is full. Cannot add more books.\n");
        pthread_mutex_unlock(&inventory_mutex);
        return;
    }
    pthread_mutex_unlock(&inventory_mutex);
    Book newbook;
    printf("Enter title: ");
    fgets(newbook.title, sizeof(newbook.title), stdin);
    size_t len = strlen(newbook.title);
    if (len > 0 && newbook.title[len-1] == '\n') newbook.title[len-1] = '\0';
    printf("Enter genre: ");
    fgets(newbook.genre, sizeof(newbook.genre), stdin);
    len = strlen(newbook.genre);
    if (len > 0 && newbook.genre[len-1] == '\n') newbook.genre[len-1] = '\0';
    printf("Enter year: ");
    char year_str[20];
    fgets(year_str, sizeof(year_str), stdin);
    newbook.year = atoi(year_str);
    pthread_mutex_lock(&inventory_mutex);
    inventory[book_count++] = newbook;
    pthread_mutex_unlock(&inventory_mutex);
    printf("Book added successfully.\n");
}

void delete_book() {
    printf("Enter title to delete: ");
    char title[100];
    fgets(title, sizeof(title), stdin);
    size_t len = strlen(title);
    if (len > 0 && title[len-1] == '\n') title[len-1] = '\0';
    pthread_mutex_lock(&inventory_mutex);
    int index = -1;
    for (int i = 0; i < book_count; i++) {
        if (strcmp(inventory[i].title, title) == 0) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("Book '%s' not found.\n", title);
    } else {
        for (int i = index; i < book_count - 1; i++) {
            inventory[i] = inventory[i+1];
        }
        book_count--;
        printf("Book '%s' deleted successfully.\n", title);
    }
    pthread_mutex_unlock(&inventory_mutex);
}

int login() {
    const char correct_username[] = "admin";
    const char correct_password[] = "1234";
    char username[50];
    char password[50];
    printf("---Login Page---\n\n");
    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    size_t len = strlen(username);
    if (len > 0 && username[len-1] == '\n') username[len-1] = '\0';
    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    len = strlen(password);
    if (len > 0 && password[len-1] == '\n') password[len-1] = '\0';
    if (strcmp(username, correct_username) == 0 &&
        strcmp(password, correct_password) == 0) {
        printf("Login successful!\n\n");
        return 1;
    } else {
        printf("Incorrect username or password. Exiting program.\n");
        return 0;
    }
}

int main() {
    pthread_mutex_init(&inventory_mutex, NULL);
    const char *filename = "inventory.txt";
    load_inventory(filename);
    if (!login()) {
        pthread_mutex_destroy(&inventory_mutex);
        return 0;
    }
    int choice;
    do {
        printf("\n--- Bookshelf Management System ---\n");
        printf("1. Display all books\n");
        printf("2. Add a book\n");
        printf("3. Delete a book\n");
        printf("4. Sort books\n");
        printf("5. Save\n");
        printf("6. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid choice. Try again.\n");
            while (getchar() != '\n');
            continue;
        }
        getchar();
        switch (choice) {
            case 1: display_books(); break;
            case 2: add_book(); break;
            case 3: delete_book(); break;
            case 4: sort_books(); break;
            case 5: save_inventory(filename); printf("Inventory saved successfully.\n"); break;
            case 6: printf("Exiting program...\n"); break;
            default: printf("Invalid option. Please try aga in.\n");
        }
    } while (choice != 6);
    pthread_mutex_destroy(&inventory_mutex);
    sem_destroy(&sort_sem);
    return 0;
}
