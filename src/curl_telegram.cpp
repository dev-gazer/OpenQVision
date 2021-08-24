#include "curl_telegram.h"
#include <iostream>
#include <curl/curl.h>

using namespace std;

size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

void sendTelegram(string message, string chatId){
    string url = "https://api.telegram.org/bot<YOUR-BOT-TOKEN-HERE>/sendMessage?chat_id=" + chatId + "&parse_mode=Markdown&text=" + message;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    auto curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        std::string response_string;
        std::string header_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

        curl_easy_perform(curl);
        cout << response_string;
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        curl = NULL;
    }
}

void sendTelegramPhoto(string chat_id, string path_to_photo, string caption = NULL){
    CURL *curl;
    CURLcode response;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    string url = "https://api.telegram.org/bot<YOUR-BOT-TOKEN-HERE>/sendPhoto";
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_slist_append(headers, "Content-Type: multipart/form-data");
        curl_slist_append(headers, "charset=utf-8");
        curl_mime *mime;
        curl_mimepart *part;
        mime = curl_mime_init(curl);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "chat_id");
        curl_mime_data(part, chat_id.c_str(), CURL_ZERO_TERMINATED);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "photo");
        curl_mime_filedata(part, path_to_photo.c_str());
        curl_mime_type(part, "image/jpeg");
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "caption");
        curl_mime_data(part, caption.c_str(), CURL_ZERO_TERMINATED);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        response = curl_easy_perform(curl);
        curl_mime_free(mime);
        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}
