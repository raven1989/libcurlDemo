#include<iostream>
#include<curl/curl.h> 
#include<stdlib.h> 
#include<cstring> 
#include<sstream>
#include"CharsetCnvtr.h"
#include<boost/algorithm/string/regex.hpp>
#include<boost/algorithm/string/split.hpp>
#include<boost/algorithm/string.hpp>
#include<boost/algorithm/string/trim.hpp>
#include<vector> 

using namespace std; 

struct CurlBuffer{
  CurlBuffer(size_t capacity){
    size_ = 0; capacity_  = capacity+1;
    buffer_ = (char*)malloc(capacity_);
    memset(buffer_, 0, capacity_);
  }
  ~CurlBuffer(){ if(!buffer_) delete buffer_;
  }
  size_t resize(size_t capacity){
    buffer_ = (char*)realloc(buffer_, capacity);
    capacity_ = capacity;
    return capacity_;
  }
  size_t write(void *from, size_t size) {
    size_t old_size = size_;
    size_t final_size = size + old_size;
    if(final_size+1>capacity_){
      resize(final_size+1);
    }
    memcpy(&(buffer_[old_size]), from, size); size_ = final_size;
    buffer_[size_] = 0;
    return size;
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

inline string get_html_attribute(boost::regex& key, string& html){
  boost::iterator_range<string::iterator> ir = boost::algorithm::find_regex(html, key);
  if(ir.begin() != ir.end()){
    string exp(ir.begin(), ir.end());
    vector<string> pair;
    boost::split(pair, exp, boost::is_any_of("="));
    if(pair.size()==2){
      return pair[1];
    }
  }
  return "";
}

inline string get_html_pcdata(boost::regex& reg, boost::regex& head, boost::regex& tail, string& html){
  boost::iterator_range<string::iterator> ir = boost::algorithm::find_regex(html, reg);
  if(ir.begin()!=ir.end()){
    string exp(ir.begin(), ir.end());
    boost::algorithm::erase_regex(exp, head);
    boost::algorithm::erase_regex(exp, tail); 
    return exp;
  }
  return "";
}

int main(int argc, char* argv[]) {
  CURL * curl = NULL;
  CURLcode res = CURLE_OK;

  CurlBuffer chunk(64*1024);
  CurlBuffer header(4*1024);
  // CurlBuffer chunk(1);

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

    // curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)&header);

    //Follow 3xx redirect, get 3xx headers in turn until no more such headers received, then get body
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    if(res == CURLE_OK){
      boost::regex reg_charset("charset(\\s)*=(\\s)*[0-9a-zA-Z_-]+");
      boost::regex reg_title("<title>.*?</title>");
      boost::regex reg_title_head("<title>");
      boost::regex reg_title_tail("</title>");
      string utf8 = "utf-8";
      string charset;
      //有些网站一行超过16K，就不处理了，比如yahoo写了很多js函数在里面
      char line[16*1024];
      istringstream headerReader(header.read());
      while(headerReader.getline(line, 1024).good()){
        cout << "header: " <<  line << endl;
        string sline(line);
        if(charset.empty()){
          charset = get_html_attribute(reg_charset, sline);
          boost::algorithm::trim(charset);
          if(!charset.empty()){
            cout << "charset: " << charset << endl;
          }
        }
      }
      // cout << "header: " << header.read() << endl;
      string title;
      istringstream bodyReader(chunk.read());
      istream* is = NULL;
      do{
        is = &bodyReader.getline(line, 4*1024);
        if(is->eof()){
          break;
        }
        if(is->fail()){
          is->clear();
          continue;
        } 
        // cout << "body: " <<  (charset.empty()? line:cnvt(charset, utf8, line) ) << endl;
        string sline( charset.empty()? line:cnvt(charset,utf8,line) );
        if(charset.empty()){
          charset = get_html_attribute(reg_charset, sline);
        }
        if(title.empty()){
          title = get_html_pcdata(reg_title, reg_title_head, reg_title_tail, sline);
          if(!title.empty()){
            boost::algorithm::trim(title);
            cout << "title: " << title << endl;
            break;
          }
        }
      }while(is);
      /* while(bodyReader.getline(line, 16*1024).good()){
       *   // cout << "body: " <<  (charset.empty()? line:cnvt(charset, utf8, line) ) << endl;
       *   string sline( charset.empty()? line:cnvt(charset,utf8,line) );
       *   if(charset.empty()){
       *     charset = get_html_attribute(reg_charset, sline);
       *   }
       *   if(title.empty()){
       *     title = get_html_pcdata(reg_title, reg_title_head, reg_title_tail, sline);
       *     if(!title.empty()){
       *       boost::algorithm::trim(title);
       *       cout << "title: " << title << endl;
       *       break;
       *     }
       *   }
       * }    */
      // cout << "html : " << chunk.read() << endl; 
      cout << "recieved(bytes) : " << chunk.size_ << endl; 
    }
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}
