#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define NAME_LENGTH 128

typedef enum { SLIP, LAND, TRAILER, STORAGE } LocationType;

typedef union {
    int slipNumber;
    char bayLetter;
    char trailerTag[NAME_LENGTH];
    int storageSpace;
} LocationInfo;

typedef struct {
    char name[NAME_LENGTH];
    int length;
    LocationType type;
    LocationInfo location;
    float amountOwed;
} Boat;

Boat *boats[MAX_BOATS] = { NULL };
int boatCount = 0;

// Function declarations
void loadBoatsFromFile(const char *filename);
void saveBoatsToFile(const char *filename);
void addBoat(const char *csvLine);
void removeBoat(const char *name);
void acceptPayment(const char *name, float amount);
void updateMonthCharges();
void printInventory();
int findBoatIndex(const char *name);
int compareBoats(const void *a, const void *b);
void sortBoats();
void freeBoatMemory();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <BoatData.csv>\n", argv[0]);
        return 1;
    }

    loadBoatsFromFile(argv[1]);

    char option;
    char input[NAME_LENGTH];
    float payment;

    printf("Welcome to the Boat Management 
System\n-------------------------------------\n");

    while (1) {
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it 
: ");
        scanf(" %c", &option);
        option = tolower(option);

        switch (option) {
            case 'i':
                printInventory();
                break;
            case 'a':
                printf("Please enter the boat data in CSV format                 
: ");
                scanf(" %[^\n]", input);
                addBoat(input);
                break;
            case 'r':
                printf("Please enter the boat name                               
: ");
                scanf(" %[^\n]", input);
                removeBoat(input);
                break;
            case 'p':
                printf("Please enter the boat name                               
: ");
                scanf(" %[^\n]", input);
                int index = findBoatIndex(input);
                if (index < 0) {
                    printf("No boat with that name\n");
                    break;
                }
                printf("Please enter the amount to be paid                       
: ");
                scanf("%f", &payment);
                acceptPayment(input, payment);
                break;
            case 'm':
                updateMonthCharges();
                break;
            case 'x':
                saveBoatsToFile(argv[1]);
                freeBoatMemory();
                printf("\nExiting the Boat Management System\n");
                return 0;
            default:
                printf("Invalid option %c\n", option);
        }
    }
}

void loadBoatsFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file %s\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        addBoat(line);
    }
    fclose(file);
}

void saveBoatsToFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Could not open file %s for writing\n", filename);
        return;
    }

    for (int i = 0; i < boatCount; i++) {
        Boat *b = boats[i];
        fprintf(file, "%s,%d,", b->name, b->length);
        switch (b->type) {
            case SLIP:
                fprintf(file, "slip,%d,", b->location.slipNumber);
                break;
            case LAND:
                fprintf(file, "land,%c,", b->location.bayLetter);
                break;
            case TRAILER:
                fprintf(file, "trailer,%s,", b->location.trailerTag);
                break;
            case STORAGE:
                fprintf(file, "storage,%d,", b->location.storageSpace);
                break;
        }
        fprintf(file, "%.2f\n", b->amountOwed);
    }
    fclose(file);
}

void addBoat(const char *csvLine) {
    if (boatCount >= MAX_BOATS) {
        printf("Marina is full, cannot add more boats\n");
        return;
    }

    Boat *newBoat = (Boat *)malloc(sizeof(Boat));
    if (!newBoat) {
        printf("Memory allocation failed\n");
        return;
    }

    char typeStr[NAME_LENGTH], extraInfo[NAME_LENGTH];
    sscanf(csvLine, "%[^,],%d,%[^,],%[^,],%f", newBoat->name, 
&newBoat->length, typeStr, extraInfo, &newBoat->amountOwed);

    // Convert type string to enum and handle location info
    if (strcasecmp(typeStr, "slip") == 0) {
        newBoat->type = SLIP;
        newBoat->location.slipNumber = atoi(extraInfo);
    } else if (strcasecmp(typeStr, "land") == 0) {
        newBoat->type = LAND;
        newBoat->location.bayLetter = extraInfo[0];
    } else if (strcasecmp(typeStr, "trailer") == 0) {
        newBoat->type = TRAILER;
        strncpy(newBoat->location.trailerTag, extraInfo, NAME_LENGTH);
    } else if (strcasecmp(typeStr, "storage") == 0) {
        newBoat->type = STORAGE;
        newBoat->location.storageSpace = atoi(extraInfo);
    } else {
        printf("Unknown location type '%s'\n", typeStr);
        free(newBoat);
        return;
    }

    boats[boatCount++] = newBoat;
    sortBoats(); // Keep boats sorted
}

void removeBoat(const char *name) {
    int index = findBoatIndex(name);
    if (index < 0) {
        printf("No boat with that name\n");
        return;
    }

    free(boats[index]);
    for (int i = index; i < boatCount - 1; i++) {
        boats[i] = boats[i + 1];
    }
    boats[--boatCount] = NULL;
}

void acceptPayment(const char *name, float amount) {
    int index = findBoatIndex(name);
    if (index < 0) {
        printf("No boat with that name\n");
        return;
    }

    Boat *b = boats[index];
    if (amount > b->amountOwed) {
        printf("That is more than the amount owed, $%.2f\n", 
b->amountOwed);
        return;
    }

    b->amountOwed -= amount;
}

void updateMonthCharges() {
    for (int i = 0; i < boatCount; i++) {
        Boat *b = boats[i];
        switch (b->type) {
            case SLIP:
                b->amountOwed += b->length * 12.50;
                break;
            case LAND:
                b->amountOwed += b->length * 14.00;
                break;
            case TRAILER:
                b->amountOwed += b->length * 25.00;
                break;
            case STORAGE:
                b->amountOwed += b->length * 11.20;
                break;
        }
    }
}

void printInventory() {
    for (int i = 0; i < boatCount; i++) {
        Boat *b = boats[i];
        printf("%-20s %3d'  ", b->name, b->length);
        switch (b->type) {
            case SLIP:
                printf("slip   # %d   ", b->location.slipNumber);
                break;
            case LAND:
                printf("land      %c   ", b->location.bayLetter);
                break;
            case TRAILER:
                printf("trailer %s   ", b->location.trailerTag);
                break;
            case STORAGE:
                printf("storage   # %d   ", b->location.storageSpace);
                break;
        }
        printf("Owes $%.2f\n", b->amountOwed);
    }
}

int findBoatIndex(const char *name) {
    for (int i = 0; i < boatCount; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int compareBoats(const void *a, const void *b) {
    Boat *boatA = *(Boat **)a;
    Boat *boatB = *(Boat **)b;
    return strcasecmp(boatA->name, boatB->name);
}

void sortBoats() {
    qsort(boats, boatCount, sizeof(Boat *), compareBoats);
}

void freeBoatMemory() {
    for (int i = 0; i < boatCount; i++) {
        free(boats[i]);
    }
}

