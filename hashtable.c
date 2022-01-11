//*** CHUAT Romain ***//
//*** groupe 1B    ***//

#include "hashtable.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/*
 * value
 */

enum value_kind value_get_kind(const struct value *self) {
  return self -> kind;
}

bool value_is_nil(const struct value *self) {
  return self -> kind == VALUE_NIL;
}

bool value_is_boolean(const struct value *self) {
  return self -> kind == VALUE_BOOLEAN;
}

bool value_is_integer(const struct value *self) {
  return self -> kind == VALUE_INTEGER;
}

bool value_is_real(const struct value *self) {
  return self -> kind == VALUE_REAL;
}

bool value_is_custom(const struct value *self) {
  return self -> kind == VALUE_CUSTOM;
}


void value_set_nil(struct value *self) {
  if(!value_is_nil(self)){
    self -> kind = VALUE_NIL;
  }
}

void value_set_boolean(struct value *self, bool val) {
  if(!value_is_boolean(self)){
    self -> kind = VALUE_BOOLEAN;
  }
  self->as.boolean = val;
}

void value_set_integer(struct value *self, int64_t val) {
    if(!value_is_integer(self)){
    self -> kind = VALUE_INTEGER;
  }
  self->as.integer = val;
}

void value_set_real(struct value *self, double val) {
  if(!value_is_real(self)){
    self -> kind = VALUE_REAL;
  }
  self->as.real = val;
}

void value_set_custom(struct value *self, void *val) {
  if(!value_is_custom(self)){
    self -> kind = VALUE_CUSTOM;
  }
  self->as.custom = val;
}

bool value_get_boolean(const struct value *self) {
  if(self -> kind == VALUE_BOOLEAN){
    return self -> as.boolean;
  }
  return true;
}

int64_t value_get_integer(const struct value *self) {
  if(self -> kind == VALUE_INTEGER){
    return self -> as.integer;
  }
  return 0;
}

double value_get_real(const struct value *self) {
    if(self -> kind == VALUE_REAL){
    return self -> as.real;
  }
  return 0.0;
}

void *value_get_custom(const struct value *self) {
  if(self -> kind == VALUE_CUSTOM){
    return self -> as.custom;
  }
  return NULL;
}

struct value value_make_nil() {
  struct value res;
  res.kind = VALUE_NIL;
  return res;
}

struct value value_make_boolean(bool val) {
  struct value res;
  res.kind = VALUE_BOOLEAN;
  res.as.boolean = val;
  return res;
}

struct value value_make_integer(int64_t val) {
  struct value res;
  res.kind = VALUE_INTEGER;
  res.as.integer = val;
  return res;
}

struct value value_make_real(double val) {
  struct value res;
  res.kind = VALUE_REAL;
  res.as.real = val;
  return res;
}

struct value value_make_custom(void *val) {
  struct value res;
  res.kind = VALUE_CUSTOM;
  res.as.custom = val;
  return res;
}

/*
 * hashtable
 */

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

void hashtable_create(struct hashtable *self) {
  self -> size = HASHTABLE_INITIAL_SIZE;
  self -> buckets = malloc(sizeof(struct bucket*)*self->size);
  for(size_t i= 0; i< self-> size; i++){
    self -> buckets[i] = NULL;
  }
  self -> count = 0;
}

void hashtable_destroy(struct hashtable *self) {
    for(size_t i = 0; i< self ->size; i++){
      struct bucket *curr = self -> buckets[i];
      while(curr != NULL){
        struct bucket *temp = curr;
        curr = curr -> next;
        free(temp -> key);
        free(temp);
      }
    }
  free(self -> buckets);
}

size_t hashtable_get_count(const struct hashtable *self) {
  return self -> count;
}

size_t hashtable_get_size(const struct hashtable *self) {
  return self -> size;
}

size_t hash_fnv1a(const char *key){
  size_t h = FNV_OFFSET;
  while(*key){
    h ^= *key;
    h *= FNV_PRIME;
    key++;
  }
  return h;
}

bool hashtable_insert(struct hashtable *self, const char *key, struct value value) {

  size_t h = hash_fnv1a(key);
  size_t index = h%self->size ;

  if(!hashtable_contains(self,key)){
    struct bucket *new = malloc(sizeof(struct bucket));
    new -> key = malloc(sizeof(char)*strlen(key)+1);
    strcpy(new -> key, key);
    new -> next = NULL;
    new -> value = value;
    if(self -> buckets[index] == NULL){
      self -> buckets[index] = new;
    }else{
      struct bucket *curr = self -> buckets[index];
      while(curr -> next != NULL){
        curr = curr -> next;
      }
      curr -> next= new;
    }
    self -> count ++;
    double compression = (double)self->count/self->size; 
    if(compression > 0.5 ){
      hashtable_rehash(self);
    }
    return true;
  }else{
    struct bucket *curr = self -> buckets[index];
    while(curr !=NULL){
      if(strcmp(curr -> key, key)==0){
        curr -> value = value;
      }
      curr = curr -> next;
    }
  }
  return false;
}

bool hashtable_remove(struct hashtable *self, const char *key) {
  size_t h = hash_fnv1a(key);
  size_t index = h%self->size;
  if(self -> buckets[index] == NULL){
    return false;
  }
  if(strcmp(self -> buckets[index]->key,key)==0){
    struct bucket *temp = self -> buckets[index] -> next;
    free(self -> buckets[index] -> key);
    free(self -> buckets[index]);
    self -> buckets[index] = temp;
    self -> count --;
    return true;
  }else{
    struct bucket *curr = self -> buckets[index];
    while(curr != NULL){
      if(strcmp(curr-> next -> key,key)==0){
        struct bucket *temp = curr -> next;
        curr -> next  = temp -> next;
        free(temp-> key);
        free(temp);
        self -> count --;
        return true;
      }
      curr = curr -> next;
    }
  }
  return false;
}

bool hashtable_contains(const struct hashtable *self, const char *key) {
  size_t h = hash_fnv1a(key);
  size_t index = h%self->size ;
  struct bucket *search = self -> buckets[index];
  while(search != NULL){
    if(strcmp(search -> key, key) == 0){
        return true;
    }
    search = search -> next;
  }
  
  return false;
}

void hashtable_rehash(struct hashtable *self) {
  size_t new_size = self -> size * 2;
  struct bucket **new_buckets = malloc(sizeof(struct bucket*)*new_size);
  for(size_t i = 0; i <new_size; i++){
    new_buckets[i] = NULL;
  }
  for(size_t i =0;i< self -> size; i++){
    struct bucket *curr = self -> buckets[i];
    while(curr != NULL){
      size_t h = hash_fnv1a(curr -> key);
      size_t index = h%new_size;
      if(new_buckets[index] != NULL){
        struct bucket *temp = new_buckets[index];
        while(temp -> next != NULL){
          temp = temp -> next;
        }
        temp -> next = curr;
      }else{
        new_buckets[index] = curr;
      }
      struct bucket *next = curr -> next;
      curr -> next = NULL;
      curr = next;
    }
    
  }
  free(self -> buckets);
  self -> buckets = new_buckets;
  self -> size = new_size;
}


void hashtable_set_nil(struct hashtable *self, const char *key) {
  hashtable_insert(self, key, value_make_nil());
}

void hashtable_set_boolean(struct hashtable *self, const char *key, bool val) {
  hashtable_insert(self, key, value_make_boolean(val));
}

void hashtable_set_integer(struct hashtable *self, const char *key, int64_t val) {
   hashtable_insert(self, key, value_make_integer(val));
}

void hashtable_set_real(struct hashtable *self, const char *key, double val) {
   hashtable_insert(self, key, value_make_real(val));
}

void hashtable_set_custom(struct hashtable *self, const char *key, void *val) {
   hashtable_insert(self, key, value_make_custom(val));
}

struct value hashtable_get(struct hashtable *self, const char *key) {
  struct value res;
  size_t h = hash_fnv1a(key);
  size_t index = h%self->size ;
  while(self -> buckets[index] != NULL){
    if(strcmp(self->buckets[index]-> key, key)==0){
      return self -> buckets[index] -> value;
    }
    self -> buckets[index] = self -> buckets[index] -> next;
  }
  res = value_make_nil();
  return res;
}
