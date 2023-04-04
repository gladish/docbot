#include "openai.h"
#include <cjson/cJSON.h>
#include <string.h>

#include <stdexcept>
#include <iostream>


namespace {
  static size_t writeCallback(void *data, size_t size, size_t nmemb, void *user_data)
  {
    std::string *buff = reinterpret_cast<std::string *>(user_data);
    buff->append(reinterpret_cast<char *>(data), size * nmemb);
    return size * nmemb;
  }

  static size_t readCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
  {
    std::string *buff = reinterpret_cast<std::string *>(userdata);
    size_t len = buff->size();
    if (len > size * nmemb)
      len = size * nmemb;
    memcpy(ptr, buff->c_str(), len);
    buff->erase(0, len);    
    return len;
  }

  std::string doGet(const std::string &url, const std::string &api_key)
  {
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // "https://api.openai.com/v1/models");

    std::string response_text;

    char api_key_header[128];
    snprintf(api_key_header, sizeof(api_key_header), "Authorization: Bearer %s", api_key.c_str());

    curl_slist *headers = curl_slist_append(nullptr, api_key_header);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_text);
  
    CURLcode res = curl_easy_perform(curl);

    if (curl)
      curl_easy_cleanup(curl);
    if (headers)
      curl_slist_free_all(headers);

    if (res != CURLE_OK)  
      throw::std::runtime_error(curl_easy_strerror(res));  
      
    return response_text;
  }

  cJSON *doGetJson(const std::string &url, const std::string &api_key)
  {
    std::string response_text = doGet(url, api_key);
    cJSON *json = cJSON_Parse(response_text.c_str());
    if (!json)
      throw std::runtime_error("Failed to parse JSON response");
    return json;
  }

  openai::Model model_from_json(cJSON *json)
  {
    openai::Model m;
        
    m.Id = cJSON_GetObjectItem(json, "id")->valuestring;
    m.Created = cJSON_GetObjectItem(json, "created")->valuedouble;
    m.OwnedBy = cJSON_GetObjectItem(json, "owned_by")->valuestring;

    cJSON *permissions = cJSON_GetObjectItem(json, "permissions");
    if (permissions) {
      m.Permissions.Id = cJSON_GetObjectItem(permissions, "id")->valuestring;
      m.Permissions.AllowCreateEngine = cJSON_GetObjectItem(permissions, "allow_create_engine")->type == cJSON_True;
      m.Permissions.AllowSampling = cJSON_GetObjectItem(permissions, "allow_sampling")->type == cJSON_True;
      m.Permissions.AllowLogProbs = cJSON_GetObjectItem(permissions, "allow_logprobs")->type == cJSON_True;
      m.Permissions.AllowSearchIndices = cJSON_GetObjectItem(permissions, "allow_search_indices")->type == cJSON_True;
      m.Permissions.AllowView = cJSON_GetObjectItem(permissions, "allow_view")->type == cJSON_True;
      m.Permissions.AllowFineTuning = cJSON_GetObjectItem(permissions, "allow_fine_tuning")->type == cJSON_True;
    }

    return m;
  }
}

namespace openai
{

Client::Client(const std::string &api_key)
  : m_api_key(api_key)
{

}

std::list<Model> Client::getModels()
{
  std::list<Model> model_list;

  cJSON *res = doGetJson("https://api.openai.com/v1/models", m_api_key);  

  cJSON *model = nullptr;
  cJSON_ArrayForEach(model, cJSON_GetObjectItem(res, "data"))
    model_list.push_back(model_from_json(model));  
   
  return model_list;
}

void Client::documentCode(const std::string &code)
{
  // std::string res = doGet(https://api.openai.com/v1/edits", "code-davinci-edit-001")

  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/edits");

  std::string response_text;

  cJSON *temp_obj = cJSON_CreateObject();    
  cJSON_AddStringToObject(temp_obj, "model", "code-davinci-edit-001");
  //cJSON_AddStringToObject(temp_obj, "model", "gpt-3.5-turbo");
  //cJSON_AddStringToObject(temp_obj, "prompt", code.c_str());
  cJSON_AddStringToObject(temp_obj, "input", code.c_str());
  //cJSON_AddNumberToObject(temp_obj, "temperature", 0.2);
  //cJSON_AddBoolToObject(temp_obj, "stream", false);
  //cJSON_AddNumberToObject(temp_obj, "max_tokens", 1000);


  std::string instruction = R"(
    Create doxygen documentation for this function.    
  )";

  cJSON_AddStringToObject(temp_obj, "instruction", instruction.c_str());
  char *s = cJSON_Print(temp_obj);
  cJSON_Delete(temp_obj);

  std::string post_data = s;
  free(s);

  char api_key_header[128];
  snprintf(api_key_header, sizeof(api_key_header), "Authorization: Bearer %s", m_api_key.c_str());

  curl_slist *headers = curl_slist_append(nullptr, api_key_header);
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_text);  
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, &readCallback);

  //printf("post_data: %s\n", post_data.c_str());

  curl_easy_setopt(curl, CURLOPT_READDATA, &post_data);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);  
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

  CURLcode res = curl_easy_perform(curl);

  if (curl)
    curl_easy_cleanup(curl);
  if (headers)
    curl_slist_free_all(headers);

  if (res != CURLE_OK)  
    throw::std::runtime_error(curl_easy_strerror(res));  

  cJSON *response = cJSON_Parse(response_text.c_str());
  cJSON *choices = cJSON_GetObjectItem(response, "choices");
  cJSON *choice = cJSON_GetArrayItem(choices, 0);
  cJSON *text = cJSON_GetObjectItem(choice, "text");

  std::cout << cJSON_GetStringValue(text) << std::endl;  
}


}