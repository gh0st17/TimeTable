#include <Parser.hpp>
#include <curl/curl.h>

using namespace std;

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  ((string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

bool fetchURL(const string& url, string& readBuffer, const char* proxy = nullptr) {
  bool res = false;
  CURLcode curl_res;
  CURL* curl = curl_easy_init();

  if (!curl)
    throw std::runtime_error("Ошибка инициализации libcurl");

  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curlGuard(curl, curl_easy_cleanup);

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
  curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");
  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookies.txt");
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
  if (proxy)
    curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
  curl_easy_setopt(curl, CURLOPT_USERAGENT,
    "Mozilla/5.0 (Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 \
(KHTML, like Gecko) Chrome/51.0.2704.103 Safari/537.36");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  cout << "Загружаю страницу" << (proxy ? " через прокси" : "") << "...";

  curl_res = curl_easy_perform(curl);
  if (curl_res == CURLE_OPERATION_TIMEDOUT)
    cout << "Таймаут операции превышен\n\n";
  else if (curl_res == CURLE_OK) {
    cout << "загружено\n\n";
    res = true;
  }
  else if (curl_res == CURLE_RECV_ERROR)
    cout << "Ошибка приема данных\n\n";
  else if (curl_res == CURLE_SSL_CONNECT_ERROR ||
            curl_res == CURLE_COULDNT_CONNECT)
    cout << "Сбой при подключении\n\n";

  curl_easy_cleanup(curl);
  return res;
}

bool Parser::group_name_predicate::operator()(pugi::xml_node node) const {
  return !strcmp(node.name(), "h1") &&
    !strcmp(node.first_attribute().value(), "mb-5");
}

bool Parser::week_predicate::operator()(pugi::xml_node node) const {
  return !strcmp(node.name(), "h3") &&
    !strcmp(node.first_attribute().value(),
      "me-5 mb-2 fw-medium");
}

void Parser::prepareHTML(string& html) {
  const string block[] = { "head", "section",
    "header", "script", "form" };
  size_t p1, p2;

  for (const auto& x : block) {
    p1 = html.find("<" + x);
    p2 = html.find("</" + x + ">");
    while (p1 != string::npos && p2 != string::npos) {
      html.erase(p1, p2 + x.length() + 3 - p1);
      p1 = html.find("<" + x);
      p2 = html.find("</" + x + ">");
    }
  }

  const vector<string> single = { "</h5>", "&nbsp;", "&ndash;" };
  const string new_single[] = { " ", " ", "-" };

  for (size_t i = 0; i < single.size(); i++) {
    p1 = html.find(single[i]);
    while (p1 != string::npos) {
      html.replace(p1, single[i].length(), new_single[i].c_str(), 1);
      p1 = html.find(single[i]);
    }
  }
}

bool Parser::loadDocument(const Params& p, pugi::xml_document& doc, const string& url) {
  bool res;
  string buffer;

  if (p.proxy.empty())
    res = fetchURL(url, buffer);
  else
    res = fetchURL(url, buffer, p.proxy.c_str());

  prepareHTML(buffer);
  doc.load_string(buffer.c_str());

  return res;
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

void Parser::parse(TimeTable& tt, const Params& p, const string& url, unsigned retry) {
  string text, month_name;
  Item item;
  Day day;

  pugi::xml_document doc;
  if (!loadDocument(p, doc, url) && retry <= 3) {
    cout << "Повторная попытка " << retry << " через " << p.sleep << " секунд\n";
    this_thread::sleep_for(chrono::seconds(p.sleep));
    parse(tt, p, url, retry++);
    return;
  }

  auto node = doc.find_node(group_name_predicate());
  if (!node)
    throw "Ошибка в документе";
  tt.group = node.child_value();

  if (p.session) {
    vector<string> splitted;
    boost::algorithm::split(splitted, tt.group, boost::is_any_of(" "));
    tt.group = splitted.back();
  }

  node = doc.find_node(week_predicate());
  if (!node.empty())
    tt.week = stoi(matchRegex(node.child_value(), regex(R"(\d+)")));

  size_t tp_size;
  pugi::xpath_node_set tp;

  auto doc_days = [&]() {
    if (p.session)
      return doc.select_nodes(session_path);
    else
      return doc.select_nodes(schedule_path);
  }();

  if (doc_days.empty()) {
    throw "Расписание не найдено";
  }

  for (const auto& doc_day : doc_days) {
    text = doc_day.node().select_node("div/div/span").node().child_value();
    boost::algorithm::trim(text);
    month_name = matchRegex(text, regex(R"(\s(\W+)$)"), 2);
    day.pdate = date(day.pdate.year(), month.at(month_name),
      stoi(matchRegex(text, regex(R"(\d{2})"))));
    for (const auto& doc_item : doc_day.node().select_nodes("div/div/div")) {
      text = doc_item.node().select_node("div/p").node().child_value();
      boost::algorithm::trim(text);
      item.name = text;
      if (!doc_item.node().select_node("div/p/span/span").node().empty()) {
        text = doc_item.node().select_node("div/p/span").node().child_value();
        boost::algorithm::trim(text);
        item.name += ' ' + text;
        text = doc_item.node().select_node("div/p/span/span").node().child_value();
      }
      else
        text = doc_item.node().select_node("div/p/span").node().child_value();

      boost::algorithm::trim(text);
      for (const auto& ch : { "(", ")" })
        boost::algorithm::erase_all(text, ch);
      item.item_type = text;

      tp = doc_item.node().select_nodes("ul/li");
      tp_size = tp.size();
      for (size_t i = 0; i < tp_size; i++) {
        if (tp[i].node().first_child().name() != string("a"))
          text = tp[i].node().child_value();
        else {
          text = tp[i].node().first_child().child_value();
          item.educators.push_back(text);
          continue;
        }

        if (i == 0) {
          unsigned short hour = stoi(matchRegex(text, regex(R"(\d+)"))),
            minute = stoi(matchRegex(text, regex(R"(\d+)"), 2));

          item.time = ptime(day.pdate, hours(hour) + minutes(minute));
        }
        else
          item.places.push_back(text);
      }
      
      day.items.push_back(item);
      item = Item();
    }

    tt.days.push_back(day);
    day = Day();
  }
}

void Parser::parse_group(Params& p, const string& url, const bool isPrint) {
  if (!std::filesystem::exists(p.work_path + "/groups"))
    std::filesystem::create_directory(p.work_path + "/groups");

  pugi::xml_document doc;
  string filename = p.work_path + "/groups/" + p.filename;

  if (std::filesystem::exists(filename)) {
    cout << "Использую список групп из кэша\n\n";
    doc.load_file(filename.c_str());
  }
  else {
    loadDocument(p, doc, url);
    doc.save_file(filename.c_str());
  }

  string text;
  pugi::xpath_node_set nodes, groups = doc.select_nodes(group_path);
  pugi::xml_attribute attr;

  for (const auto& group : groups) {
    nodes = group.node().select_nodes("a");

    attr = group.node().find_attribute
      (
        [](pugi::xml_attribute& attr) {
          return !strcmp(attr.name(), "role") && !strcmp(attr.value(), "tabpanel");
        }
      );
    if (attr.empty())
      continue;

    for (const auto& node : nodes) {
      text = node.node().child_value();

      if (text.empty())
        continue;

      p.group_names.push_back(text);
    }
  }

  sort(p.group_names.begin(), p.group_names.end());
  if (isPrint)
    for (unsigned i = 0; const auto& group : p.group_names)
      cout << setw(2) << i++ << ") " << group << endl;
}