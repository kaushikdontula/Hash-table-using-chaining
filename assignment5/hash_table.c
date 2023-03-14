/*
 * In this file, you will write the structures and functions needed to
 * implement a hash table.  Feel free to implement any helper functions
 * you need in this file to implement a hash table.  Make sure to add your
 * name and @oregonstate.edu email address below:
 *
 * Name:
 * Email:
 */

#include <stdlib.h>

#include "dynarray.h"
#include "list.h"
#include "hash_table.h"


/*
 * This is the structure that represents a hash table.  You must define
 * this struct to contain the data needed to implement a hash table.
 */
struct ht{
    struct dynarray* array;
    int size;
}; 

struct ht_node{
    void* value;
    void* key; 
};


/*
 * This function should allocate and initialize an empty hash table and
 * return a pointer to it.
 */
struct ht* ht_create(){

    //create hash table struct and initialize elements
    struct ht* hash_table = malloc(sizeof(struct ht));
    hash_table->size = 0;
    hash_table->array = dynarray_create();


    //creating linked lists for the capacity of the array
    for(int i=0; i<get_capacity(hash_table->array); i++){
        struct list* l = list_create();
        dynarray_insert(hash_table->array, l);
    }

    return hash_table;
}

struct ht_node* get_ht_node(struct ht* ht, void* key, int (*convert)(void*)){

    //calculate hash_num
    int hash_num = ht_hash_func(ht, key, convert);

    //get the linked list at the hash_num index of the dynarray
    struct list* list = dynarray_get(ht->array, hash_num);

    //if the linked list is not NULL
    if (list != NULL) {

        //iterate through linked list until the key is found
        struct node* node = get_head(list);
        while (node != NULL) {
            struct ht_node* temp = get_value(node);
            
            //matching key is found
            if (convert(temp->key) == convert(key)) {
                return temp;
            }
            node = get_next(node);
        }
    }

    // The key was not found
    return NULL;
}

/*
 * This function should free the memory allocated to a given hash table.
 * Note that this function SHOULD NOT free the individual elements stored in
 * the hash table.  That is the responsibility of the caller.
 *
 * Params:
 *   ht - the hash table to be destroyed.  May not be NULL.
 */
void ht_free(struct ht* ht){
    for(int i =0; i< get_capacity(ht->array); i++){
        struct list* temp = dynarray_get(ht->array, i);
        list_free(temp);
        //free(temp);
    }
    
    dynarray_free(ht->array);
    free(ht);
    return;
}

/*
 * This function should return 1 if the specified hash table is empty and
 * 0 otherwise.
 *
 * Params:
 *   ht - the hash table whose emptiness is to be checked.  May not be
 *     NULL.
 *
 * Return:
 *   Should return 1 if ht is empty and 0 otherwise.
 */
int ht_isempty(struct ht* ht){
    if(ht->size == 0){
        return 1;
    }
    else{
        return 0;
    }
}


/*
 * This function returns the size of a given hash table (i.e. the number of
 * elements stored in it, not the capacity).
 */
int ht_size(struct ht* ht){
    return ht->size;
}


/*
 * This function takes a key, maps it to an integer index value in the hash table,
 * and returns it. The hash algorithm is totally up to you. Make sure to consider
 * Determinism, Uniformity, and Speed when design the hash algorithm
 *
 * Params:
 *   ht - the hash table into which to store the element.  May not be NULL.
 *   key - the key of the element to be stored
 *   convert - pointer to a function that can be passed the void* key from
 *     to convert it to a unique integer hashcode
 */
int ht_hash_func(struct ht* ht, void* key, int (*convert)(void*)){
    
    //convert void* key to an int
    int index = convert(key);

    //use formula to calculate hashcode
    int hashcode = index%dynarray_size(ht->array);

    return hashcode;

}


/*
 * This function should insert a given element into a hash table with a
 * specified key.  Note that you cannot have two same keys in one hash table.
 * If the key already exists, update the value associated with the key.  
 * This function is passed a function pointer that is used to convert the key (void*) 
 * to a unique hashcode (int). 
 * Resolution of collisions is requried, use either chaining or open addressing.
 * If using chaining, double the number of buckets when the load factor is >= 4
 * If using open addressing, double the array capacity when the load factor is >= 0.75
 * load factor = (number of elements) / (hash table capacity)
 *
 * Params:
 *   ht - the hash table into which to insert an element.  May not be NULL.
 *   key - the key of the element
 *   value - the value to be inserted into ht.
 *   convert - pointer to a function that can be passed the void* key from
 *     to convert it to a unique integer hashcode
 */

void ht_insert(struct ht* ht, void* key, void* value, int (*convert)(void*)){

    //calculate hash_num (index)
    int hash_num = ht_hash_func(ht, key, convert);

    //create a linked list node* to get the head of the linked list at the dynarray index of hash_num
    struct node* temp = get_head(dynarray_get(ht->array, hash_num));

    //calculate load factor
    float load_factor = (float)ht->size/(dynarray_size(ht->array));
    
    //resize if load factor is too high
    if (load_factor >= 4){
        // printf("THE LOAD FACTOR WORKS: \n");

        //initialize new and old capacity
        int old_capacity = dynarray_size(ht->array);
        int new_capacity = (dynarray_size(ht->array)) *2;
        
        //create a new array
        struct dynarray* new_array = dynarray_create();

        //resize the new array to the new capacity size
        _dynarray_resize(new_array, new_capacity);

        //creates empty linked lists in every new array index
        for(int i =0; i < get_capacity(new_array); i++){
            dynarray_insert(new_array, list_create());
        }

        //for the size of old array rehash all elements within that array
        for (int i = 0; i < old_capacity; i++){

            //get the linked list in array at index i
            struct list* l = dynarray_get(ht->array, i);

            //get the head of linked list
            struct node* n = get_head(l);
            
            //while the head is not NULL traverse through the list
            while (n != NULL){

                //get the ht_node
                struct ht_node* ht_n = get_value(n);
                
                //create a new hash number for this ht_node
                int new_hash_num = convert(ht_n->key) % new_capacity;

                struct list* new_list = dynarray_get(new_array, new_hash_num);
                list_insert(new_list, ht_n);
                n = get_next(n);
            }
        }

        //replace the old array with the new array
        for(int i =0; i < get_capacity(ht->array); i++){
            struct list* temp = dynarray_get(ht->array, i);
            linked_list_free(temp);
        }
        dynarray_free(ht->array);
        ht->array = new_array;

    }

    //create a hashtable node and to see if it already exists, if it already exists update the value
    struct ht_node* ht_temp = get_ht_node(ht, key, convert);
    if(ht_temp != NULL){
        if(ht_temp->key == key){
            //free(ht_temp->value);
            ht_temp->value = value;
            return;
        }
    }
    //otherwise get the list at the dynarray index and insert a ht_node that is initialized with key and value
    else{
        ht->size++;
        struct list* l = dynarray_get(ht->array, hash_num);
        struct ht_node* insert = malloc(sizeof(struct ht_node));
        insert->key = key;
        insert->value = value;
        list_insert(l, insert);
    }

    return;
}


/*
 * This function should search for a given element in a hash table with a
 * specified key provided.   
 * This function is passed a function pointer that is used to convert the key (void*) 
 * to a unique hashcode (int). 
 * If the key is found, return the corresponding value (void*) of the element,
 * otherwise, return NULL
 *
 * Params:
 *   ht - the hash table into which to loop up for an element.  May not be NULL.
 *   key - the key of the element to search for
 *   convert - pointer to a function that can be passed the void* key from
 *     to convert it to a unique integer hashcode
 */
void* ht_lookup(struct ht* ht, void* key, int (*convert)(void*)){

    //calculate has_num
    int hash_num = ht_hash_func(ht, key, convert);

    //get the linked list at the hash_num index of the dynarray
    struct list* list = dynarray_get(ht->array, hash_num);

    //if the list we got is not NULL iterate through the list to find the key and return the value of that key
    if (list != NULL) {
        struct node* node = get_head(list);
        while (node != NULL) {
            struct ht_node* temp_ht = get_value(node);

            if (convert(temp_ht->key) == convert(key)) {
                // Found the node with the matching key
                return temp_ht->value;
            }
            node = get_next(node);
        }
    }

    // The key was not found
    return NULL;
}


/*
 * This function should remove a given element in a hash table with a
 * specified key provided.   
 * This function is passed a function pointer that is used to convert the key (void*) 
 * to a unique hashcode (int). 
 * If the key is found, remove the element and return, otherwise, do nothing and return 
 *
 * Params:
 *   ht - the hash table into which to remove an element.  May not be NULL.
 *   key - the key of the element to remove
 *   convert - pointer to a function that can be passed the void* key from
 *     to convert it to a unique integer hashcode
 */
void ht_remove(struct ht* ht, void* key, int (*convert)(void*)){

    //calculate hash_num for index of dynarray
    int hash_num = ht_hash_func(ht, key, convert);

    //get the linked list associated with the hash_num index of dynarray
    struct list* lis = dynarray_get(ht->array, hash_num);
    
    //get the hash table node with the key
    struct ht_node* temp = get_ht_node(ht, key, convert);

    //if list is not empty
    if(lis != NULL){

        //get the head of the list
        struct node* node = get_head(lis);

        //while the node is not NULL
        while (node != NULL) {

            //get the value of the node and set it to ht_node
            struct ht_node* temp_ht = get_value(node);
            
            //if the key is what were looking for
            if (convert(temp_ht->key) == convert(key)) {

                //remove the node
                node_remove(lis, temp_ht);
                ht->size--;
                free(temp_ht);
                return;
            }
            node = get_next(node);
        }
    }
    
    return;

} 



