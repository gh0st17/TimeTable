#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <limits>
#include <curl.h>

#undef max

#include "Params.hpp"

using namespace std;
using namespace std::filesystem;

struct group_predicate {
  bool operator()(pugi::xml_node node) const {
    return !strcmp(node.name(), "h1") &&
      !strcmp(node.first_attribute().value(), "mb-5");
  }
};

struct week_predicate {
  bool operator()(pugi::xml_node node) const {
    return !strcmp(node.name(), "h3") &&
      !strcmp(node.first_attribute().value(),
        "me-5 mb-2 fw-medium");
  }
};

const string base_url(const Params& p) {
  return "http://mai.ru/education/studies/schedule/index.php?group="
    + p.group_names[p.group - 1];
}

const string week_url(const Params& p) {
  return base_url(p) + "&week=" + to_string(p.week);
}

const string group_url(const Params& p) {
  return "https://mai.ru/education/studies/schedule/groups.php?department=Институт+№"
    + to_string(p.dep) + "&course=" + to_string(p.course);
}

const string proceedHTML(string html) {
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

  return html;
}

static size_t writeCallback(void* contents, size_t size,
                            size_t nmemb, void* userp) {
  ((string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

const string fetchURL(const string& url) {
  CURL* curl;
  CURLcode res;
  string readBuffer = "";

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookies.txt");
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
      "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.6) \
Gecko/20070725 Firefox/2.0.0.6");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    cout << "Загружаю страницу...";
    res = curl_easy_perform(curl);
    cout << "загружено\n\n";
    curl_easy_cleanup(curl);
  }
  if (readBuffer.empty())
    throw "Пустой буффер";
  return readBuffer;
}

void parse(const string& url) {
  pugi::xml_document* doc = new pugi::xml_document();
  doc->load_string(proceedHTML(fetchURL(url)).c_str());
  auto node = doc->find_node(group_predicate());
  cout << node.child_value() << "\n\n";

  node = doc->find_node(week_predicate());
  if (node != NULL)
    cout << node.child_value() << "\n\n";

  string text;
  pugi::xpath_node_set tp,
    days = doc->select_nodes("/html/body/main/div/div/div/article/ul/li");

  for (const auto& day : days) {
    text = day.node().select_node("div/div/span").node().child_value();
    boost::algorithm::trim(text);
    cout << text << endl;
    for (const auto& item : day.node().select_nodes("div/div/div")) {
      text = item.node().select_node("div/p").node().child_value();
      boost::algorithm::trim(text);
      cout << text;
      if (item.node().select_node("div/p/span/span") != NULL) {
        text = item.node().select_node("div/p/span").node().child_value();
        boost::algorithm::trim(text);
        cout << ' ' << text;
        text = item.node().select_node("div/p/span/span").node().child_value();
      }
      else
        text = item.node().select_node("div/p/span").node().child_value();
      boost::algorithm::trim(text);
      cout << " [" + text + ']' << endl;

      tp = item.node().select_nodes("ul/li");
      for (const auto& time_place : tp) {
        if (time_place.node().first_child().name() != string("a"))
          cout << time_place.node().child_value();
        else
          cout << time_place.node().first_child().child_value();

        if (time_place != tp.end()->node())
          cout << " / ";
      }
      cout << endl;
    }
    cout << endl;
  }

  delete doc;
}

void parse_group(Params& p, const bool isPrint) {
  pugi::xml_document *doc = new pugi::xml_document();
  if (exists(current_path().u8string() + "\\" + p.filename)) {
    cout << "Загружаю список групп из кэша\n\n";
    doc->load_file(p.filename.c_str());
  }
  else {
    doc->load_string(proceedHTML(fetchURL(group_url(p))).c_str());
    doc->save_file(p.filename.c_str());
  }

  string text;
  pugi::xpath_node_set nodes,
    groups = doc->select_nodes("/html/body/main/div/div/div/article/div/div");

  unsigned i = 1;
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

int main(int argc, char* argv[]) {
  try {
    Params p(argc, argv);

    if (p.clear) {
      unsigned cnt{ 0 };
      for (const auto& file : directory_iterator("./")) {
        if (file.path().extension() == ".xml") {
          cout << file.path().filename().u8string() << endl;
          remove(file.path());
          cnt++;
        }
      }
      cout << "Удалено файлов: " << cnt << endl;
    }
    else if (p.list)
      return 0;
    else if (p.group && p.week) 
      parse(week_url(p));
    else if (p.group && !p.week)
      parse(base_url(p));
    else {
      cout << "Введите номер группы: ";
      while (!(cin >> p.group) || !p.group || p.group > p.group_names.size()) {
        cout << "Введите номер группы: ";
        cin.clear();
        cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
      }
      parse(base_url(p));
    }
  }
  catch (bad_alloc const&) {
    cerr << "Ошибка выделения памяти\n";
  }
  catch (const exception& e) {
    cerr << e.what() << endl;
  }
  catch (const char* e) {
    cerr << e << endl;
  }
  return 0;
}