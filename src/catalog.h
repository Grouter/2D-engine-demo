#if !defined(CATALOG_H)
#define CATALOG_H

struct ResourceCatalog {
    u64 size;

    struct Entry {
        char  *key;
        u64   index;
        Entry *next_in_hash;
    };

    Entry **entries;
};

inline void allocate_resource_catalog(ResourceCatalog &catalog, u64 size) {
    catalog.size = size;
    catalog.entries = (ResourceCatalog::Entry **)calloc((size_t)size, sizeof(ResourceCatalog::Entry *));
}

internal void catalog_put(ResourceCatalog &catalog, const char *key, u64 index) {
    u64 hashed_index = hash_string(key) % catalog.size;
    u64 key_size = strlen(key);

    ResourceCatalog::Entry *entry = catalog.entries[hashed_index];

    if (entry == nullptr) {
        catalog.entries[hashed_index] = (ResourceCatalog::Entry *)malloc(sizeof(ResourceCatalog::Entry));

        entry = catalog.entries[hashed_index];

        entry->key = (char *)malloc((size_t)(key_size + 1) * sizeof(char));
        strcpy_s(entry->key, (rsize_t)key_size + 1, key);

        entry->index = index;
        entry->next_in_hash = nullptr;

        return;
    }

    while (1) {
        if (strcmp(entry->key, key) == 0) { // We just update the value.
            entry->index = index;
            return;
        }

        if (entry->next_in_hash == nullptr) {   // We found the last one!
            ResourceCatalog::Entry *new_entry = (ResourceCatalog::Entry *)malloc(sizeof(ResourceCatalog::Entry));

            new_entry->key = (char *)malloc((size_t)(key_size + 1) * sizeof(char));
            strcpy_s(new_entry->key, (rsize_t)key_size + 1, key);
            new_entry->index = index;
            new_entry->next_in_hash = nullptr;

            entry->next_in_hash = new_entry;

            return;
        }

        entry = entry->next_in_hash;
    }
}

internal void catalog_remove(ResourceCatalog &catalog, const char *key) {
    u64 hashed_index = hash_string(key) % catalog.size;

    ResourceCatalog::Entry *walker = catalog.entries[hashed_index];

    if (walker == nullptr) {
        log_print("Trying to remove unexistent entry (key: %s)\n", key);
        return;
    }

    if (strcmp(walker->key, key) == 0) {    // Removing the first entry
        catalog.entries[hashed_index] = catalog.entries[hashed_index]->next_in_hash;

        free(walker->key);
        free(walker);

        return;
    }

    ResourceCatalog::Entry *prev = walker;
    walker = walker->next_in_hash;

    while (walker) {
        if (strcmp(walker->key, key) == 0) {    // Remove!
            prev->next_in_hash = walker->next_in_hash;

            free(walker->key);
            free(walker);

            return;
        }

        prev = walker;
        walker = walker->next_in_hash;
    }

    log_print("Trying to remove unexistent entry (key: %s)\n", key);
}

internal u64 catalog_get(ResourceCatalog &catalog, const char *key) {
    ResourceCatalog::Entry *walker;

    for (u64 i = 0; i < catalog.size; i++) {
        walker = catalog.entries[i];

        while(walker) {
            if (strcmp(walker->key, key) == 0) {
                return walker->index;
            }

            walker = walker->next_in_hash;
        }
    }

    return UINT64_MAX;
}

internal bool catalog_cointains(ResourceCatalog &catalog, const char *key) {
    ResourceCatalog::Entry *walker;

    for (u64 i = 0; i < catalog.size; i++) {
        walker = catalog.entries[i];

        while(walker) {
            if (strcmp(walker->key, key) == 0) {
                return true;
            }

            walker = walker->next_in_hash;
        }
    }

    return false;
}

internal void catalog_dump(ResourceCatalog &catalog) {
    for (u64 i = 0; i < catalog.size; i++) {
        ResourceCatalog::Entry *walker = catalog.entries[i];

        log_print("[%llu] o-> ", i);

        while(walker) {
            log_print("%s o-> ", walker->key);
            walker = walker->next_in_hash;
        }

        log_print("\n");
    }
}

internal void catalog_clear(ResourceCatalog &catalog) {
    ResourceCatalog::Entry *walker;
    ResourceCatalog::Entry *next_in_hash;

    for (u64 i = 0; i < catalog.size; i++) {
        walker = catalog.entries[i];

        while(walker) {
            next_in_hash = walker->next_in_hash;

            free(walker->key);
            free(walker);

            walker = next_in_hash;
        }

        catalog.entries[i] = nullptr;
    }
}

#endif
