#include <stdio.h>
#include <curl/curl.h>

// 保存并打印内容
// static size_t writeCallback(void *contents, size_t size, size_t nitems, FILE *file) {
//   fprintf(stdout,"%s",contents);
//   return fwrite(contents, size, nitems, file);
// }

// 打印报头
// static size_t header(char* b, size_t size, size_t nitems, void *userdata) {
//   //fprintf(stderr,"%s", b);
//   //fprintf(stderr,"\n-------------------------------------------------\n");
//   return 0;
// }

int main(int argc, char* argv[])
{
	if(argc != 2) {
		fprintf(stderr, "URL is needed. \n");
		exit(1);
	}

    char * URL = argv[1];
    //static char * URL = "https://www.baidu.com";

    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();

    if(curl) {

        // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        // curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);



        // URL
        long response_code;
        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (CURLE_OK != res){
            fprintf(stderr,"CURLINFO_RESPONSE_CODE Error | Response code:$d\n", response_code);
            exit(1);
        } else {
            curl_easy_setopt(curl, CURLOPT_URL, URL);
        }

        // HEADER
        long header_size;
        res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE,&header_size);
        if(CURLE_OK == res){
            curl_easy_setopt(curl,CURLOPT_HEADER,1L);
            // curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header);
        } else {
            fprintf(stderr,"CURLINFO_HEADER_SIZE Error! Respose code: %d\n",res);
            exit(1);
        }

        // CONNECT
        long code;
        res = curl_easy_getinfo(curl, CURLINFO_HTTP_CONNECTCODE, &code);
        if(!CURLE_OK == res){
            fprintf(stderr,"CURLINFO_HTTP_CONNECTCODE Error! Response code: %d\n",res);
            exit(1);
        }


        // Start performing
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s | Response code: %d\n",curl_easy_strerror(res),res);
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}