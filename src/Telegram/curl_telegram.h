#ifndef CURL_TELEGRAM_H
#define CURL_TELEGRAM_H

#include <iostream>

using namespace std;

void sendTelegramPhoto(string chat_id, string bot_token, string path_to_photo, string caption);
void sendTelegram(string message, string bot_token, string chatId);
size_t writeFunction(void* ptr, size_t size, size_t nmemb, string* data);

#endif
