#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */

// gcc -Wall -o get_forms get_forms.c -ljansson /usr/lib/x86_64-linux-gnu/libcurl.so.4.3.0

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


int main(int argc,char * argv[]) {
  char *text;
  json_t *root;
  json_error_t error;

  // Adjust the URL according to your CouchDB server. 
  char url[] = "http://sommer-access:5984/formation/_design/forms/_view/forms";
  text = request(url);
  printf("%s\n", text);

  // Load the Json.
  root = json_loads(text, 0, &error);
  free(text);

  if(!root) {
    fprintf(stderr, "error: on line %d: %s url: %s\n", error.line, error.text, url);
    return 1;
  }

  json_decref(root);
  return 0;
}
