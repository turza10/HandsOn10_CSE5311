#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// Hash node structure for chaining
typedef struct HashNode
{
    int key;
    int value;
    struct HashNode *next;
    struct HashNode *prev;
} HashNode;

// Hash table structure
typedef struct HashTable
{
    HashNode **buckets;
    int capacity;
    int size;
    int (*hashFunction)(int, int);
} HashTable;

// Function prototypes
HashNode *createNode(int key, int value);
HashTable *createHashTable(int capacity, int (*hashFunction)(int, int));
void insert(HashTable *ht, int key, int value);
void insertWithoutResize(HashTable *ht, int key, int value);
bool search(HashTable *ht, int key, int *value);
bool removeKey(HashTable *ht, int key);
void printHashTable(HashTable *ht);
void freeHashTable(HashTable *ht);
int simpleHash(int key, int capacity);
int improvedHash(int key, int capacity);

// Create a new hash node
HashNode *createNode(int key, int value)
{
    HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
    if (!newNode)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    newNode->key = key;
    newNode->value = value;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

// Create a new hash table
HashTable *createHashTable(int capacity, int (*hashFunction)(int, int))
{
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (!ht)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    ht->buckets = (HashNode **)calloc(capacity, sizeof(HashNode *));
    if (!ht->buckets)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(ht);
        exit(1);
    }

    ht->capacity = capacity;
    ht->size = 0;
    ht->hashFunction = hashFunction;

    return ht;
}

// Insert without checking for resize
void insertWithoutResize(HashTable *ht, int key, int value)
{
    // Calculate hash index
    int index = ht->hashFunction(key, ht->capacity);

    // Check if key already exists
    HashNode *current = ht->buckets[index];
    while (current)
    {
        if (current->key == key)
        {
            // Update existing key
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Create new node
    HashNode *newNode = createNode(key, value);

    // Insert at head of list (chaining)
    newNode->next = ht->buckets[index];
    if (ht->buckets[index])
    {
        ht->buckets[index]->prev = newNode;
    }
    ht->buckets[index] = newNode;

    // Increment size
    ht->size++;
}

// Insert a key-value pair into the hash table
void insert(HashTable *ht, int key, int value)
{
    // Calculate hash index
    int index = ht->hashFunction(key, ht->capacity);

    // Check if key already exists
    HashNode *current = ht->buckets[index];
    while (current)
    {
        if (current->key == key)
        {
            // Update existing key
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Create new node
    HashNode *newNode = createNode(key, value);

    // Insert at head of list (chaining)
    newNode->next = ht->buckets[index];
    if (ht->buckets[index])
    {
        ht->buckets[index]->prev = newNode;
    }
    ht->buckets[index] = newNode;

    // Increment size
    ht->size++;

    // Check for resizing (grow)
    double loadFactor = (double)ht->size / ht->capacity;
    if (loadFactor > 0.75)
    {
        // Resize and rehash
        int newCapacity = ht->capacity * 2;
        HashNode **newBuckets = (HashNode **)calloc(newCapacity, sizeof(HashNode *));
        if (!newBuckets)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }

        // Create a temporary hash table structure
        HashTable newHt;
        newHt.buckets = newBuckets;
        newHt.capacity = newCapacity;
        newHt.size = 0;
        newHt.hashFunction = ht->hashFunction;

        // Rehash all existing elements
        for (int i = 0; i < ht->capacity; i++)
        {
            HashNode *node = ht->buckets[i];
            while (node)
            {
                HashNode *next = node->next;

                // Reset node's links
                node->next = NULL;
                node->prev = NULL;

                // Rehash this node
                int newIndex = newHt.hashFunction(node->key, newHt.capacity);
                node->next = newHt.buckets[newIndex];
                if (newHt.buckets[newIndex])
                {
                    newHt.buckets[newIndex]->prev = node;
                }
                newHt.buckets[newIndex] = node;
                newHt.size++;

                node = next;
            }
        }

        // Free old buckets and update hash table
        free(ht->buckets);
        ht->buckets = newHt.buckets;
        ht->capacity = newHt.capacity;
        ht->size = newHt.size;

        printf("Hash table resized up to %d\n", ht->capacity);
    }

    // Check for resizing (shrink)
    if (ht->capacity > 10 && loadFactor < 0.25)
    {
        // Resize and rehash
        int newCapacity = ht->capacity / 2;
        HashNode **newBuckets = (HashNode **)calloc(newCapacity, sizeof(HashNode *));
        if (!newBuckets)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }

        // Create a temporary hash table structure
        HashTable newHt;
        newHt.buckets = newBuckets;
        newHt.capacity = newCapacity;
        newHt.size = 0;
        newHt.hashFunction = ht->hashFunction;

        // Rehash all existing elements
        for (int i = 0; i < ht->capacity; i++)
        {
            HashNode *node = ht->buckets[i];
            while (node)
            {
                HashNode *next = node->next;

                // Reset node's links
                node->next = NULL;
                node->prev = NULL;

                // Rehash this node
                int newIndex = newHt.hashFunction(node->key, newHt.capacity);
                node->next = newHt.buckets[newIndex];
                if (newHt.buckets[newIndex])
                {
                    newHt.buckets[newIndex]->prev = node;
                }
                newHt.buckets[newIndex] = node;
                newHt.size++;

                node = next;
            }
        }

        // Free old buckets and update hash table
        free(ht->buckets);
        ht->buckets = newHt.buckets;
        ht->capacity = newHt.capacity;
        ht->size = newHt.size;

        printf("Hash table resized down to %d\n", ht->capacity);
    }
}

// Search for a key in the hash table
bool search(HashTable *ht, int key, int *value)
{
    int index = ht->hashFunction(key, ht->capacity);

    HashNode *current = ht->buckets[index];
    while (current)
    {
        if (current->key == key)
        {
            if (value)
            {
                *value = current->value;
            }
            return true;
        }
        current = current->next;
    }

    return false;
}

// Remove a key from the hash table
bool removeKey(HashTable *ht, int key)
{
    int index = ht->hashFunction(key, ht->capacity);

    HashNode *current = ht->buckets[index];
    while (current)
    {
        if (current->key == key)
        {
            // Update links
            if (current->prev)
            {
                current->prev->next = current->next;
            }
            else
            {
                ht->buckets[index] = current->next;
            }

            if (current->next)
            {
                current->next->prev = current->prev;
            }

            // Free node
            free(current);

            // Decrement size
            ht->size--;

            // Check for resizing (shrink)
            double loadFactor = (double)ht->size / ht->capacity;
            if (ht->capacity > 10 && loadFactor < 0.25)
            {
                // Resize and rehash
                int newCapacity = ht->capacity / 2;
                HashNode **newBuckets = (HashNode **)calloc(newCapacity, sizeof(HashNode *));
                if (!newBuckets)
                {
                    fprintf(stderr, "Memory allocation failed\n");
                    exit(1);
                }

                // Create a temporary hash table structure
                HashTable newHt;
                newHt.buckets = newBuckets;
                newHt.capacity = newCapacity;
                newHt.size = 0;
                newHt.hashFunction = ht->hashFunction;

                // Rehash all existing elements
                for (int i = 0; i < ht->capacity; i++)
                {
                    HashNode *node = ht->buckets[i];
                    while (node)
                    {
                        HashNode *next = node->next;

                        // Reset node's links
                        node->next = NULL;
                        node->prev = NULL;

                        // Rehash this node
                        int newIndex = newHt.hashFunction(node->key, newHt.capacity);
                        node->next = newHt.buckets[newIndex];
                        if (newHt.buckets[newIndex])
                        {
                            newHt.buckets[newIndex]->prev = node;
                        }
                        newHt.buckets[newIndex] = node;
                        newHt.size++;

                        node = next;
                    }
                }

                // Free old buckets and update hash table
                free(ht->buckets);
                ht->buckets = newHt.buckets;
                ht->capacity = newHt.capacity;
                ht->size = newHt.size;

                printf("Hash table resized down to %d\n", ht->capacity);
            }

            return true;
        }
        current = current->next;
    }

    return false;
}

// Print the hash table
void printHashTable(HashTable *ht)
{
    printf("Hash Table (size: %d, capacity: %d):\n", ht->size, ht->capacity);
    for (int i = 0; i < ht->capacity; i++)
    {
        printf("[%d]: ", i);
        HashNode *current = ht->buckets[i];
        while (current)
        {
            printf("(%d:%d) -> ", current->key, current->value);
            current = current->next;
        }
        printf("NULL\n");
    }
    printf("\n");
}

// Free the hash table
void freeHashTable(HashTable *ht)
{
    for (int i = 0; i < ht->capacity; i++)
    {
        HashNode *current = ht->buckets[i];
        while (current)
        {
            HashNode *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->buckets);
    free(ht);
}

// Simple hash function
int simpleHash(int key, int capacity)
{
    return key % capacity;
}

// Improved hash function
int improvedHash(int key, int capacity)
{
    // Using a prime number multiplier and bitwise operations
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = (key >> 16) ^ key;
    return key % capacity;
}

int main()
{
    printf("=== Hash Table Implementation ===\n\n");

    // Create a hash table with simple hash function
    HashTable *ht = createHashTable(10, simpleHash);

    // Insert some key-value pairs
    insert(ht, 5, 500);
    insert(ht, 15, 1500); // Will hash to the same bucket as 5
    insert(ht, 25, 2500); // Will hash to the same bucket as 5 and 15
    insert(ht, 6, 600);
    insert(ht, 16, 1600); // Will hash to the same bucket as 6

    // Print the hash table
    printf("After insertions:\n");
    printHashTable(ht);

    // Search for keys
    int value;
    if (search(ht, 15, &value))
    {
        printf("Found key 15 with value %d\n", value);
    }
    else
    {
        printf("Key 15 not found\n");
    }

    if (search(ht, 7, &value))
    {
        printf("Found key 7 with value %d\n", value);
    }
    else
    {
        printf("Key 7 not found\n");
    }

    // Remove a key
    printf("\nRemoving key 15...\n");
    if (removeKey(ht, 15))
    {
        printf("Key 15 removed successfully\n");
    }
    else
    {
        printf("Key 15 not found\n");
    }

    // Print the hash table after removal
    printf("\nAfter removal:\n");
    printHashTable(ht);

    // Free the hash table
    freeHashTable(ht);

    printf("=== Dynamic Resizing Demonstration ===\n\n");

    // Create a new hash table for resizing demo
    HashTable *resizableHt = createHashTable(10, improvedHash);

    // Insert many elements to trigger resizing
    printf("Inserting elements to trigger resizing...\n");
    for (int i = 0; i < 100; i++)
    {
        insert(resizableHt, i, i * 100);
    }

    printf("\nFinal hash table state:\n");
    printf("Size: %d, Capacity: %d\n", resizableHt->size, resizableHt->capacity);

    // Remove many elements to trigger shrinking
    printf("\nRemoving elements to trigger shrinking...\n");
    for (int i = 0; i < 80; i++)
    {
        removeKey(resizableHt, i);
    }

    printf("\nFinal hash table state after removals:\n");
    printf("Size: %d, Capacity: %d\n", resizableHt->size, resizableHt->capacity);

    // Free the resizable hash table
    freeHashTable(resizableHt);

    printf("\n=== Hash Function Comparison ===\n\n");

    // Create hash tables with different hash functions
    HashTable *simpleHt = createHashTable(10, simpleHash);
    HashTable *improvedHt = createHashTable(10, improvedHash);

    // Insert same keys into both
    srand(time(NULL));
    for (int i = 0; i < 20; i++)
    {
        int key = rand() % 100;
        int value = rand() % 1000;
        insert(simpleHt, key, value);
        insert(improvedHt, key, value);
    }

    // Count collisions
    int simpleCollisions = 0;
    int improvedCollisions = 0;

    for (int i = 0; i < simpleHt->capacity; i++)
    {
        int count = 0;
        HashNode *current = simpleHt->buckets[i];
        while (current)
        {
            count++;
            current = current->next;
        }
        if (count > 1)
        {
            simpleCollisions += (count - 1);
        }
    }

    for (int i = 0; i < improvedHt->capacity; i++)
    {
        int count = 0;
        HashNode *current = improvedHt->buckets[i];
        while (current)
        {
            count++;
            current = current->next;
        }
        if (count > 1)
        {
            improvedCollisions += (count - 1);
        }
    }

    printf("Simple hash function collisions: %d\n", simpleCollisions);
    printf("Improved hash function collisions: %d\n", improvedCollisions);

    // Free both hash tables
    freeHashTable(simpleHt);
    freeHashTable(improvedHt);

    return 0;
}