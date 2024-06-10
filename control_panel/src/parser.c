#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *api_get_file_content(FILE* f) {
  char c = 0;
  size_t fsize = 0;
 
  // check if file exists
  if (!f) 
    return NULL;
 
  rewind(f);
  while((c=fgetc(f)) != EOF )
    fsize++;

  // check if file is empty
  if (fsize==0) {
    printf("INFO: File '%p' is empty, exit reading...\n",f);
    return NULL;
  }

  char *s = calloc(fsize+1,1);
  rewind(f);
 

  for(size_t spos = 0; spos < fsize-1; spos++)
    s[spos] = (c = fgetc(f));

  return s;
}


char *get_file_content(char* path) {
  FILE* f = fopen(path,"r");
  char* r = api_get_file_content(f);
  fclose(f);
  return r;
}

char *fget_file_content(FILE *f) {
  return api_get_file_content(f);
}


static char* api_get_value_from_key(char* buffer,char *buffer_copy,char* key) {
  buffer_copy = calloc(strlen(buffer)+1,1);
  strcpy(buffer_copy,buffer);
  char *token = strtok(buffer_copy,"\n");
  while (token != NULL) {
    if (strstr(token,key) && *(token+strlen(key)) == ':' )
    {
      // printf("\t[%c]\t", );
      char* pair = strtok(token,":");
      return (pair=strtok(NULL,":")); // printf ("Pair [%s] = { key: %s, value: %s }\n",token, pair, (pair=strtok(NULL,":")) );
    }
    token = strtok(NULL, "\n");
  }
  return NULL;
}

char* get_value_from_key(char* buffer,char* key) {
  char *temp;
  char *value = api_get_value_from_key(buffer,temp,key);
  
  if (!value)
    return NULL;

  char *str = calloc(strlen(value)+1,1);
  str = strcpy(str,value);
  free(temp);
  return str;
}

char* modify_buffer_value_at_key
     (char* oldbuffer, char* key, char* newvalue) {
 
  // if no keyval found, exit
  if (!get_value_from_key(oldbuffer,key))
    return NULL;

  char* begin_pos = strstr(oldbuffer,key);
  char* end_pos = begin_pos;

  while((*end_pos)!=0 && (*end_pos)!='\n')
    end_pos++;

  size_t size_before = begin_pos-oldbuffer;
  size_t size_after  = strlen(end_pos)    ;
  size_t keyval_size = strlen(oldbuffer)-(size_after+size_before);
  
  size_t key_size = 0;
  for(
    uint i = size_before; 
    oldbuffer[i] != ':';
    i++
  )
    key_size ++;

  size_t bufsize =  size_before+size_after+key_size+1+strlen(newvalue)+1;
  char* newbuffer = calloc(bufsize,1);

  // copy before key
  for(uint i = 0; i < size_before; i++)
    newbuffer[i] = oldbuffer[i];
  // copy key
  for(uint i = size_before; i < size_before+key_size; i++)
    newbuffer[i] = oldbuffer[i];
  // reput ':'
  newbuffer[size_before+key_size] = ':';
  // put new value
  strcat(newbuffer,newvalue);
  // copy reminder
  size_t edge = strlen(newbuffer);
  for(uint i = 0; i < size_after; i++)
    newbuffer[edge+i] = oldbuffer[size_before+keyval_size+i];
  
  return newbuffer;
}


char* append_buffer(char* oldbuf, char* append) {
  char* new_buffer = calloc(strlen(oldbuf)+strlen(append)+1,1);

  for(uint i = 0; i < strlen(oldbuf); i++) 
    new_buffer[i] = oldbuf[i];
  for(uint i = 0; i < strlen(append); i++)
    new_buffer[strlen(oldbuf)+i] = append[i];

  return new_buffer;
}
