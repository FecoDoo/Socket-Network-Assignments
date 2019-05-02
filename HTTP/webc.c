#include <stdio.h>
#include <curl/curl.h>
 
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
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    
    // URL
    long response_code;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (!CURLE_OK == res){
      printf("Response Error! Response code:$d\n", response_code);
      exit(1);
    }

    // HEADER
    long header_size;
    res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE,&header_size);
    if(CURLE_OK == res){
      res = curl_easy_setopt(curl,CURLOPT_HEADER,1L);
      if (CURLE_OK == res){
        fprintf(stderr,"\n-------------------------------------------------\n");
      } else {
        fprintf(stderr,"Header Error!");
      }
    } else {
      printf("Header Error! Respose code: %d\n",res);
      exit(1);
    }

    // CONNECT
    long code;
    res = curl_easy_getinfo(curl, CURLINFO_HTTP_CONNECTCODE, &code);
    if(!CURLE_OK == res){
      printf("Connect Error! Response code: %d\n",res);
      exit(1);
    }


    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s | RESPONSE: %d\n",curl_easy_strerror(res),res);
    /* always cleanup */
    //printf(stdout,"%s",res);
    fprintf(stderr,"\n-------------------------------------------------\n");
    curl_easy_cleanup(curl);
  }
 
  curl_global_cleanup();
 
  return 0;
}