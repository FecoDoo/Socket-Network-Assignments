#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

struct url_data {
    size_t size;
    char* data;
};

static size_t writeCallback(void *contents, size_t size, size_t nitems, FILE *file) {
  return fwrite(contents, size, nitems, file);
}
size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data) {
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;

    data->size += (size * nmemb);

#ifdef DEBUG
    fprintf(stderr, "data at %p size=%ld nmemb=%ld\n", ptr, size, nmemb);
#endif
    tmp = realloc(data->data, data->size + 1); /* +1 for '\0' */

    if(tmp) {
        data->data = tmp;
    } else {
        if(data->data) {
            free(data->data);
        }
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}
static size_t hdf(char* b, size_t size, size_t nitems, void *userdata) {
    printf("%s", b);
    return 0;
}

char *handle_url(char* url) {
    CURL *curl;
    

    struct url_data data;
    data.size = 0;
    data.data = malloc(4096); /* reasonable size initial buffer */

    if(NULL == data.data) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return NULL;
    }

    data.data[0] = '\0';

    CURLcode res;

    curl = curl_easy_init();
    FILE *fp=fopen("save.txt","w");
    if (!fp) {
        fprintf(stderr, "Could not open output file.\n");
        return 1;
    }
    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);;
        res = curl_easy_perform(curl);

        // char *hdd;
        // curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, hdf);     
        // curl_easy_setopt(curl, CURLOPT_HEADERDATA, hdd);
        // res = curl_easy_perform(curl);

        curl_easy_setopt(curl, CURLOPT_HEADER, 1);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, hdf);

        res = curl_easy_perform(curl);
        
        long response_code;
        res=curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        if(response_code==0)
            printf("wrong url\n");

        if(CURLE_OK == res)
            printf("response_code: %ld bytes\n", response_code);

        long size;
        res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &size);
        if(CURLE_OK == res)
            printf("Header size: %ld bytes\n", size);

        long code;
        res = curl_easy_getinfo(curl, CURLINFO_HTTP_CONNECTCODE, &code);
        if(CURLE_OK == res) 
            printf("http_connectcode: %ld bytes\n", code);
        
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    return data.data;
}




int main(int argc, char* argv[]) {
    char* data;

    if(argc != 2) {
        fprintf(stderr, "wrong input arg.\n");
        return 1;
    }
    data = handle_url(argv[1]);

    if(data) {
        printf("%s\n", data);
        free(data);
    }

    return 0;
}