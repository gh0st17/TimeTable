#include "Parser.hpp"
#include <curl.h>

using namespace std;
using namespace std::filesystem;

static size_t writeCallback(void* contents, size_t size,
  size_t nmemb, void* userp) {
  ((string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

void fetchURL(const string& url, string* readBuffer, const char* proxy = NULL) {
  CURLcode res;
  CURL* curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookies.txt");
    if (proxy)
      curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
      "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.6) \
Gecko/20070725 Firefox/2.0.0.6");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, readBuffer);
    cout << "Загружаю страницу...";

    res = curl_easy_perform(curl);
    if (res == CURLE_OPERATION_TIMEDOUT)
      cout << "Таймаут операции превышен\n\n";
    else if (res == CURLE_OK)
      cout << "загружено\n\n";
    else if (res == CURLE_RECV_ERROR)
      cout << "Ошибка приема данных\n\n";
    else if (res == CURLE_SSL_CONNECT_ERROR)
      cout << "Сбой при подключении\n\n";

    curl_easy_cleanup(curl);
  }

  if (readBuffer->empty()) {
    delete readBuffer;
    throw "Пустой буффер";
  }
}

bool Parser::group_predicate::operator()(pugi::xml_node node) const {
  return !strcmp(node.name(), "h1") &&
    !strcmp(node.first_attribute().value(), "mb-5");
}

bool Parser::week_predicate::operator()(pugi::xml_node node) const {
  return !strcmp(node.name(), "h3") &&
    !strcmp(node.first_attribute().value(),
      "me-5 mb-2 fw-medium");
}

void Parser::prepareHTML(string* html) {
  const string block[] = { "head", "section",
    "header", "script", "form" };
  size_t p1, p2;

  for (const auto& x : block) {
    p1 = html->find("<" + x);
    p2 = html->find("</" + x + ">");
    while (p1 != string::npos && p2 != string::npos) {
      html->erase(p1, p2 + x.length() + 3 - p1);
      p1 = html->find("<" + x);
      p2 = html->find("</" + x + ">");
    }
  }

  const vector<string> single = { "</h5>", "&nbsp;", "&ndash;" };
  const string new_single[] = { " ", " ", "-" };

  for (size_t i = 0; i < single.size(); i++) {
    p1 = html->find(single[i]);
    while (p1 != string::npos) {
      html->replace(p1, single[i].length(), new_single[i].c_str(), 1);
      p1 = html->find(single[i]);
    }
  }
}

const string Parser::matchRegex(const string str, const regex r, const size_t n) {
  string::const_iterator strBegin(str.cbegin());
  smatch match;
  size_t i = 0;
  while (i++ < n && regex_search(strBegin, str.cend(), match, r)) {
    if (match.size() > 1)
      return match[n - 1];

    strBegin = match.suffix().first;
  }
  return match[0];
}

TimeTable Parser::parse(const Params& p, const string& url) {
  TimeTable tt;
  string text;
  Item item;
  Day day;

  pugi::xml_document* doc = new pugi::xml_document();
  string* buffer = new string();
  fetchURL(url, buffer);
  prepareHTML(buffer);
  doc->load_string(buffer->c_str());

  auto node = doc->find_node(group_predicate());
  tt.group = node.child_value();

  node = doc->find_node(week_predicate());
  if (node != NULL)
    tt.week = stoi(matchRegex(node.child_value(), regex(R"(\d+)")));

  size_t tp_size;
  pugi::xpath_node_set tp,
    doc_days = doc->select_nodes("/html/body/main/div/div/div/article/ul/li");

  for (const auto& doc_day : doc_days) {
    text = doc_day.node().select_node("div/div/span").node().child_value();
    boost::algorithm::trim(text);
    day.m_name = matchRegex(text, regex(R"(\s(\W+)$)"), 2);
    day.date = date(day.date.year(), month.at(day.m_name),
      stoi(matchRegex(text, regex(R"(\d{2})"))));
    for (const auto& doc_item : doc_day.node().select_nodes("div/div/div")) {
      text = doc_item.node().select_node("div/p").node().child_value();
      boost::algorithm::trim(text);
      item.name = text;
      if (doc_item.node().select_node("div/p/span/span") != NULL) {
        text = doc_item.node().select_node("div/p/span").node().child_value();
        boost::algorithm::trim(text);
        item.name += ' ' + text;
        text = doc_item.node().select_node("div/p/span/span").node().child_value();
      }
      else
        text = doc_item.node().select_node("div/p/span").node().child_value();
      boost::algorithm::trim(text);
      item.item_type = text;

      tp = doc_item.node().select_nodes("ul/li");
      tp_size = tp.size();
      for (size_t i = 0; i < tp_size; i++) {
        if (tp[i].node().first_child().name() != string("a"))
          text = tp[i].node().child_value(); // otherwise
        else {
          text = tp[i].node().first_child().child_value(); // prepod
          item.educator.push_back(text);
          continue;
        }

        if (i == 0) {
          unsigned short h = stoi(matchRegex(text, regex(R"(\d+)"))),
            m = stoi(matchRegex(text, regex(R"(\d+)"), 2));

          item.time = ptime(day.date, hours(h) + minutes(m));
        }
        else
          item.place.push_back(text);
      }
      
      day.items.push_back(item);
      item = Item();
    }
    tt.days.push_back(day);
    day = Day();
  }

  delete doc;
  delete buffer;
  return tt;
}

void Parser::parse_group(Params& p, const string& url, const bool isPrint) {
  pugi::xml_document* doc = new pugi::xml_document();

  if (exists(current_path().u8string() + "\\" + p.filename)) {
    cout << "Использую список групп из кэша\n\n";
    doc->load_file(p.filename.c_str());
  }
  else {
    string* buffer = new string();
    fetchURL(url, buffer);
    prepareHTML(buffer);
    doc->load_string(buffer->c_str());
    doc->save_file(p.filename.c_str());
    delete buffer;
  }

  string text;
  pugi::xpath_node_set nodes,
    groups = doc->select_nodes("/html/body/main/div/div/div/article/div/div");

  unsigned i{ 1 };
  for (const auto& group : groups) {
    nodes = group.node().select_nodes("a");
    for (const auto& node : nodes) {
      text = node.node().child_value();
      if (isPrint)
        cout << setw(2) << i++ << ") " << text << endl;
      p.group_names.push_back(text);
    }
  }

  delete doc;
}