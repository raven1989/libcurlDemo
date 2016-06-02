#include<iostream>
#include<curl/curl.h>
#include<stdlib.h>
#include<cstring>

using namespace std;

struct CurlBuffer{
  CurlBuffer(size_t capacity){
    size_ = 0;
    capacity_  = capacity+1;
    buffer_ = (char*)malloc(capacity_);
    memset(buffer_, 0, capacity_);
  }
  ~CurlBuffer(){
    if(!buffer_) delete buffer_;
  }
  size_t resize(size_t capacity){
    buffer_ = (char*)realloc(buffer_, capacity);
    capacity_ = capacity;
  }
  size_t write(void *from, size_t size) {
    size_t old_size = size_;
    size_t final_size = size + old_size;
    if(final_size+1>capacity_){
      resize(final_size+1);
    }
    memcpy(&(buffer_[old_size]), from, size);
    size_ = final_size;
    buffer_[size_] = 0;
  }
  string read() {
    string res = buffer_;
    return res;
  }
  size_t size_;
  size_t capacity_;
  char* buffer_;
};

size_t write_curl_buffer(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t real_size = size * nmemb;
  CurlBuffer * mem = (CurlBuffer*)userp;
  mem->write(contents, real_size);
  return real_size;
}

int main(int argc, char* argv[]) {
  CURL * curl = NULL;
  CURLcode res = CURLE_OK;

  CurlBuffer chunk(16*1024);
  // CurlBuffer chunk(1);

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    res = curl_easy_perform(curl);
    if(res == CURLE_OK){
      cout << "html : " << chunk.read() << endl; 
      cout << "recieved(bytes) : " << chunk.size_ << endl; 
    }
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}
