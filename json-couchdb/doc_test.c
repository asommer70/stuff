#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */

// gcc -o doc_test doc_test.c -ljansson /usr/lib/x86_64-linux-gnu/libcurl.so.4.3.0

struct write_result
{
    char *data;
    int pos;
};

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
    struct write_result *result = (struct write_result *)stream;

    if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
    {
        fprintf(stderr, "error: too small buffer\n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

static char *request(const char *url)
{
    CURL *curl;
    CURLcode status;
    char *data;
    long code;

    curl = curl_easy_init();
    data = malloc(BUFFER_SIZE);
    if(!curl || !data)
        return NULL;

    struct write_result write_result = {
        .data = data,
        .pos = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

    status = curl_easy_perform(curl);
    if(status != 0)
    {
        fprintf(stderr, "error: unable to request data from %s:\n", url);
        fprintf(stderr, "%s\n", curl_easy_strerror(status));
        return NULL;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if(code != 200)
    {
        fprintf(stderr, "error: server responded with code %ld\n", code);
        return NULL;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    /* zero-terminate the result */
    data[write_result.pos] = '\0';

    return data;
}

json_t *get_couchdb_object_list(json_t *object) {
  json_t *rows;

  rows = json_object_get(object, "rows");
  if (!json_is_array(rows)) {
    fprintf(stderr, "error: rows is not an object\n");
    json_decref(object);
    return 1;
  }

  return rows;
}

int main(int argc,char * argv[]) {
  char *text;
  json_t *root;
  json_error_t error;

  // Adjust the URL according to your CouchDB server. 
  char url[] = "http://localhost:5984/formation/_design/forms/_view/forms";
  text = request(url);

  // Load the Json.
  root = json_loads(text, 0, &error);
  free(text);

  if(!root) {
    fprintf(stderr, "error: on line %d: %s url: %s\n", error.line, error.text, url);
    return 1;
  }

  // Get an array of form documents from the "rows".
  json_t *forms = get_couchdb_object_list(root);

  size_t x;
  for (x = 0; x < json_array_size(forms);  x++) {

      json_t *form = json_array_get(forms, x);
      if(!json_is_object(form))
      {
          fprintf(stderr, "error: row %d: is not an object\n", x + 1);
          json_decref(root);
          return 1;
      }

      json_t *doc = json_object_get(form, "value");
      if(!json_is_object(doc))
      {
          fprintf(stderr, "error: doc %d: is not an object\n", x + 1);
          json_decref(root);
          return 1;
      }

      json_t *_id;
      char *_id_v;
      _id = json_object_get(doc, "_id");
      _id_v = json_string_value(_id);

      printf("%s\n", _id_v);
     
      json_t *modifier_v = json_string("sommera");
      json_object_set(doc, "modifier", modifier_v);

      char *output = json_dumps(doc, json_object_size(doc));
      printf("%s\n", output);

      free(output);

  }

  json_decref(root);
  return 0;
}
