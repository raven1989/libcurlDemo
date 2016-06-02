#include<iostream>
#include<curl/curl.h>

using namespace std;

int main(int argc, char* argv[]) {
  CURL * curl = NULL;
  CURLcode res = CURLE_OK;

  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }

  return 0;
}
