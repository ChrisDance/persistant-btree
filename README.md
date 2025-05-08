# Persistant B-tree
This project implements a B-tree data structure with disk persistence.

## Overview

The implementation features:

- Persistent storage using file-backed pages
- In-memory operation mode for testing
- LRU node caching system to minimize disk access
- Key-value storage with support for variable length data
- Full B-tree operations: insert, delete, search, and traversal

## Technical Details

- **Page Size**: 4096 bytes
- **Maximum Keys**: 10 keys per node (configurable via `MAX_KEYS`)
- **Minimum Degree (t)**: (MAX_KEYS + 1) / 2 
- **Node Structure**: Each node contains keys, values, and child pointers
- **Data Storage**: Each key can store up to `DATA_SIZE` bytes of data.

## Components

### Core Classes

- **BTree**: Main interface for tree operations
- **BTreeNode**: Handles node-level operations (split, merge, borrow)
- **Header**: Manages file metadata and page allocation bitmap, and points to the root node.
- **Pager**: Handles disk I/O operations
- **NodeCache**: Implements LRU caching for in-memory nodes

## Building the Project

The project uses a simple Makefile for compilation:

```bash
make       # Build the binary
make clean # Remove binary and object files
```

## Usage

Run the binary:

```bash
./bin              # Run with persistent storage in test.db
./bin memory       # Run in memory-only mode (no persistence)
```

### Operations Menu

The program provides an interactive menu with the following options:

1. Insert a key-value pair
2. Remove a key
3. Search for a key
4. Add 50 test keys
5. Search for 50 test keys
6. Traverse the tree (display all key-value pairs)
7. Exit

## Implementation Notes

### B-tree Properties

- Every node has at most `MAX_KEYS` keys
- Every non-leaf node (except root) has at least `t-1` keys
- The root has at least 1 key (unless the tree is empty)
- All leaves are at the same level
- A non-leaf node with k keys has k+1 children

### Storage Format

- **Header Page**: Contains root node index and allocation bitmap, the root node's index is not nessessarly at the beginning of the file.
- **Node Pages**: Contains node metadata, keys, values, and child pointers

### Cache System

The implementation uses an LRU (Least Recently Used) caching system that:
- The node_cache abstraction means that the tree can operate on nodes as if they are all in memory.
- Maintains up to `MAX_CACHE_SIZE` (20) nodes in memory
- Uses a replacement policy based on access frequency
- Automatically flushes dirty nodes to disk


## Motivation
B-Trees are an answer to the question, "what do you when your tree is so big it won't fit in memory?". As a web dev to who uses SQL daily, I was
interested to learn how this data structure worked, and interested in the practical limitations (Disk I/O speed) that motivate it. The main resource I used to build this was the book "SQLite Database System Design and Implementation (2015)".


## License

This B-tree implementation is provided for educational purposes.
