#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define NAME_LEN 127

typedef enum {
    SLIP, LAND, TRAILOR, STORAGE
} PlaceType;

typedef union {
    int slip_num;
    char bay_letter;
    char trailor_license[10];
    int storage_space;
} ExtraInfo;

typedef struct {
    char name[NAME_LEN + 1];
    float length;
    PlaceType place;
    ExtraInfo extra;
    float amount_owed;
} Boat;

Boat *boats[MAX_BOATS] = {NULL};
int num_boats = 0;

int compare_boats(const void *a, const void *b) {
    const Boat *boat_a = *(const Boat **)a;
    const Boat *boat_b = *(const Boat **)b;
    return strcmp(boat_a->name, boat_b->name);
}

void load_data(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }
    char line[1024];
    while (fgets(line, sizeof(line), file) && num_boats < MAX_BOATS) {
        Boat *boat = (Boat *)malloc(sizeof(Boat));
        char place_str[10];
        sscanf(line, "%[^,],%f,%[^,],%[^,],%f", boat->name, &boat->length, place_str, boat->extra.trailor_license, &boat->amount_owed);

        if (strcmp(place_str, "slip") == 0) {
            boat->place = SLIP;
            boat->extra.slip_num = atoi(boat->extra.trailor_license);
        } else if (strcmp(place_str, "land") == 0) {
            boat->place = LAND;
            boat->extra.bay_letter = boat->extra.trailor_license[0];
        } else if (strcmp(place_str, "trailor") == 0) {
            boat->place = TRAILOR;
        } else if (strcmp(place_str, "storage") == 0) {
            boat->place = STORAGE;
            boat->extra.storage_space = atoi(boat->extra.trailor_license);
        }
        boats[num_boats++] = boat;
    }
    fclose(file);
}

void save_data(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        return;
    }
    for (int i = 0; i < num_boats; i++) {
        Boat *boat = boats[i];
        switch (boat->place) {
            case SLIP:
                fprintf(file, "%s,%.0f,slip,%d,%.2f\n", boat->name, boat->length, boat->extra.slip_num, boat->amount_owed);
                break;
            case LAND:
                fprintf(file, "%s,%.0f,land,%c,%.2f\n", boat->name, boat->length, boat->extra.bay_letter, boat->amount_owed);
                break;
            case TRAILOR:
                fprintf(file, "%s,%.0f,trailor,%s,%.2f\n", boat->name, boat->length, boat->extra.trailor_license, boat->amount_owed);
                break;
            case STORAGE:
                fprintf(file, "%s,%.0f,storage,%d,%.2f\n", boat->name, boat->length, boat->extra.storage_space, boat->amount_owed);
                break;
        }
    }
    fclose(file);
}


void add_boat(const char *csv_line) {
    if (num_boats == MAX_BOATS) {
        printf("Error: maximum number of boats reached\n");
        return;
    }
    Boat *boat = (Boat *)malloc(sizeof(Boat));
    char place_str[10];
    sscanf(csv_line, "%[^,],%f,%[^,],%[^,],%f", boat->name, &boat->length, place_str, boat->extra.trailor_license, &boat->amount_owed);

    if (strcmp(place_str, "slip") == 0) {
        boat->place = SLIP;
        boat->extra.slip_num = atoi(boat->extra.trailor_license);
    } else if (strcmp(place_str, "land") == 0) {
        boat->place = LAND;
        boat->extra.bay_letter = boat->extra.trailor_license[0];
    } else if (strcmp(place_str, "trailor") == 0) {
        boat->place = TRAILOR;
    } else if (strcmp(place_str, "storage") == 0) {
        boat->place = STORAGE;
        boat->extra.storage_space = atoi(boat->extra.trailor_license);
    }

    boats[num_boats++] = boat;
    qsort(boats, num_boats, sizeof(Boat *), compare_boats);
}

void remove_boat(const char *name) {
    int found = 0;
    for (int i = 0; i < num_boats; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            free(boats[i]);
            for (int j = i; j < num_boats - 1; j++) {
                boats[j] = boats[j + 1];
            }
            boats[--num_boats] = NULL;
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("No boat with that name\n");
    }
}

void accept_payment(const char *name) {
    int found = 0;
    for (int i = 0; i < num_boats; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            found = 1;
            float payment;
            printf("Please enter the amount to be paid: ");
            scanf("%f", &payment);

            if (payment > boats[i]->amount_owed) {
                printf("That is more than the amount owed, $%.2f\n", boats[i]->amount_owed);
            } else {
                boats[i]->amount_owed -= payment;
                printf("Payment accepted. New amount owed: $%.2f\n", boats[i]->amount_owed);
            }
            break;
        }
    }

    if (!found) {
        printf("No boat with that name\n");
    }
}

void update_month() {
    for (int i = 0; i < num_boats; i++) {
        switch (boats[i]->place) {
            case SLIP:
                boats[i]->amount_owed += boats[i]->length * 12.50;
                break;
            case LAND:
                boats[i]->amount_owed += boats[i]->length * 14.00;
                break;
            case TRAILOR:
                boats[i]->amount_owed += boats[i]->length * 25.00;
                break;
            case STORAGE:
                boats[i]->amount_owed += boats[i]->length * 11.20;
                break;
        }
    }
    
}

void print_boats() {
    
    for (int i = 0; i < num_boats; i++) {
        Boat *boat = boats[i];
        printf("%-20s %2.0f'        ", boat->name, boat->length);
        switch (boat->place) {
            case SLIP:
                printf("%-10s # %-13d ", "slip", boat->extra.slip_num);
                break;
            case LAND:
                printf("%-10s %-15c ", "land", boat->extra.bay_letter);
                break;
            case TRAILOR:
                printf("%-10s %-15s ", "trailor", boat->extra.trailor_license);
                break;
            case STORAGE:
                printf("%-10s # %-13d ", "storage", boat->extra.storage_space);
                break;
        }
        printf("Owes $%-10.2f\n", boat->amount_owed);
    }
}


void free_boats() {
    for (int i = 0; i < num_boats; i++) {
        free(boats[i]);
        boats[i] = NULL;
    }
    num_boats = 0;
}

void menu() {
    char option;
    char buffer[256];
    while (1) {
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &option);
        option = toupper(option);

        switch (option) {
            case 'I':
                print_boats();
                break;
            case 'A':
                printf("Please enter the boat data in CSV format: ");
                scanf(" %[^\n]", buffer);
                add_boat(buffer);
                break;
            case 'R':
                printf("Please enter the boat name: ");
                scanf(" %[^\n]", buffer);
                remove_boat(buffer);
                break;
            case 'P':
                printf("Please enter the boat name: ");
                scanf(" %[^\n]", buffer);
		
                float payment;
		accept_payment(buffer);
                
                break;
            case 'M':
                update_month();
                break;
            case 'X':
                printf("Exiting the Boat Management System\n");
                return;
            default:
                printf("Invalid option %c\n", option);
                break;
        }
    }
}

int main(int argc, char *argv[]) {

    load_data(argv[1]);
    qsort(boats, num_boats, sizeof(Boat *), compare_boats);

    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");
    menu();

    save_data(argv[1]);
    free_boats();

    return EXIT_SUCCESS;
}