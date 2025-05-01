#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <cstring>
#include "btree.h"

void displayMenu()
{
    std::cout << "\nB-tree Operations Menu:" << '\n';
    std::cout << "1. Insert a key-value pair" << '\n';
    std::cout << "2. Remove a key" << '\n';
    std::cout << "3. Search for a key" << '\n';
    std::cout << "4. Add 50 keys" << '\n';
    std::cout << "5. Search for 50 keys" << '\n';
    std::cout << "6. Traverse the tree" << '\n';
    std::cout << "7. Exit" << '\n';
    std::cout << "Enter your choice: ";
}

void handleInsert(BTree &btree)
{
    std::string input, data;
    int key;

    std::cout << "Enter key to insert: ";
    getline(std::cin, input);
    std::stringstream ss_insert(input);

    if (ss_insert >> key)
    {
        std::cout << "Enter data (max " << DATA_SIZE - 1 << " characters): ";
        getline(std::cin, data);

        char buffer[DATA_SIZE] = {0};
        memcpy(buffer, data.c_str(), std::min(data.size(), static_cast<size_t>(DATA_SIZE - 1)));

        btree.insert(key, buffer);
        std::cout << "Key-value pair inserted: " << key << " -> \"" << data << "\"" << '\n';
    }
    else
    {
        std::cout << "Invalid key. Please enter a number." << '\n';
    }
}

void handleRemove(BTree &btree)
{
    std::string input;
    int key;

    std::cout << "Enter key to remove: ";
    std::getline(std::cin, input);
    std::stringstream ss_remove(input);

    if (ss_remove >> key)
    {
        btree.remove(key);
        std::cout << "Key " << key << " removed (if present)." << '\n';
    }
    else
    {
        std::cout << "Invalid key. Please enter a number." << '\n';
    }
}

void handleSearch(BTree &btree)
{
    std::string input;
    int key;

    std::cout << "Enter key to search: ";
    std::getline(std::cin, input);
    std::stringstream ss_search(input);

    if (ss_search >> key)
    {
        char result[DATA_SIZE] = {0};
        if (btree.get(key, result))
        {
            std::cout << "Key " << key << " found with data: \"" << result << "\"" << '\n';
        }
        else
        {
            std::cout << "Key " << key << " not found in the tree." << '\n';
        }
    }
    else
    {
        std::cout << "Invalid key. Please enter a number." << '\n';
    }
}

void insertTestKeys(BTree &btree)
{
    std::cout << "Adding 50 keys" << '\n';
    char buffer[DATA_SIZE] = "Test data";

    for (int i = 0; i < 50; i++)
    {
        btree.insert(i, buffer);
    }
    std::cout << "50 keys added successfully." << '\n';
}

void searchTestKeys(BTree &btree)
{
    std::cout << "Getting 50 keys" << '\n';
    char buffer[DATA_SIZE];
    int foundCount = 0;

    for (int i = 0; i < 50; i++)
    {
        if (btree.get(i, buffer))
        {
            foundCount++;
        }
        else
        {
            std::cout << "Key " << i << " not in tree" << '\n';
        }
    }
    std::cout << "Found " << foundCount << " keys out of 50." << '\n';
}

int main(int argc, char **argv)
{

    BTree btree;
    if (argc > 1 && std::strcmp(argv[1], "memory") == 0)
    {
        btree.init(true, true);
    }
    else
    {

        btree.init(!btree.openFile("test.db"), false);
    }

    std::string input;
    int choice;

    std::cout << "B-tree Management System" << '\n';
    std::cout << "Maximum keys per node: " << MAX_KEYS << '\n';
    std::cout << "Minimum degree t: " << t << '\n';
    std::cout << "Data size per key: " << DATA_SIZE << " bytes" << '\n';

    while (1)
    {
        displayMenu();
        std::getline(std::cin, input);
        std::stringstream ss(input);

        if (!(ss >> choice))
        {
            std::cout << "Invalid input. Please enter a number." << '\n';
            continue;
        }

        switch (choice)
        {
        case 1:
            handleInsert(btree);
            break;

        case 2:
            handleRemove(btree);
            break;

        case 3:
            handleSearch(btree);
            break;

        case 4:
            insertTestKeys(btree);
            break;

        case 5:
            searchTestKeys(btree);
            break;

        case 6:
            std::cout << "Tree traversal:" << '\n';
            btree.traverse();
            break;

        case 7:
            std::cout << "Exiting the program..." << '\n';
            exit(0);
            break;

        default:
            std::cout << "Invalid choice. Please select a valid option (1-7)." << '\n';
            break;
        }
    }

    return 0;
}