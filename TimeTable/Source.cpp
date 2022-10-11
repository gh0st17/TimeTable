#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <curl.h>

using namespace std;
using namespace std::filesystem;

struct group_predicate {
  bool operator()(pugi::xml_node node) const
  {
    return !strcmp(node.name(), "h1") &&
      !strcmp(node.first_attribute().value(), "mb-5");
  }
};

struct week_predicate {
  bool operator()(pugi::xml_node node) const
  {
    return !strcmp(node.name(), "h3") &&
      !strcmp(node.first_attribute().value(),
        "me-5 mb-2 fw-medium");
  }
};

const string proceedHTML(string html) {
  size_t p1 = html.find("<head"),
    p2 = html.find("</head>");
  html.erase(p1, p2 + 7 - p1);

  p1 = html.find("<section");
  p2 = html.find("</section>");
  html.erase(p1, p2 + 10 - p1);

  p1 = html.find("<header");
  p2 = html.find("</header>");
  html.erase(p1, p2 + 9 - p1);

  p1 = html.find("<script");
  p2 = html.find("</script>");
  while (p1 != string::npos && p2 != string::npos) {
    html.erase(p1, p2 + 9 - p1);
    p1 = html.find("<script");
    p2 = html.find("</script>");
  }

  p1 = html.find("<form");
  p2 = html.find("</form>");
  while (p1 != string::npos && p2 != string::npos) {
    html.erase(p1, p2 + 7 - p1);
    p1 = html.find("<form");
    p2 = html.find("</form>");
  }

  p1 = html.find("</h5>");
  while (p1 != string::npos) {
    html.erase(p1, 5);
    p1 = html.find("</h5>");
  }

  p1 = html.find("&nbsp;");
  while (p1 != string::npos) {
    html.replace(p1, 6, " ", 1);
    p1 = html.find("&nbsp;");
  }

  p1 = html.find("&ndash;");
  while (p1 != string::npos) {
    html.replace(p1, 7, "-", 1);
    p1 = html.find("&ndash;");
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
  pugi::xml_document doc;
  doc.load_string(proceedHTML(fetchURL(url)).c_str());
  auto node = doc.find_node(group_predicate());
  cout << node.child_value() << "\n\n";

  node = doc.find_node(week_predicate());
  if (node != NULL)
    cout << node.child_value() << "\n\n";

  string text, item_type;
  pugi::xpath_node_set days = doc.select_nodes("/html/body/main/div/div/div/article\
/ul/li");
  pugi::xpath_node_set items, time_place;

  pugi::xpath_node_set::const_iterator day = days.begin();
  pugi::xpath_node_set::const_iterator item;

  for (; day != days.end(); ++day) {
    text = day->node().select_node("div/div/span").node().child_value();
    boost::algorithm::trim(text);
    cout << text << endl;
    items = day->node().select_nodes("div/div/div");
    item = items.begin();
    for (; item != items.end(); ++item) {
      text = item->node().select_node("div/p").node().child_value();
      boost::algorithm::trim(text);
      cout << text;
      if (item->node().select_node("div/p/span/span") != NULL) {
        text = item->node().select_node("div/p/span").node().child_value();
        boost::algorithm::trim(text);
        cout << ' ' << text;
        item_type = item->node().select_node("div/p/span/span").node().child_value();
      }
      else
        item_type = item->node().select_node("div/p/span").node().child_value();
      boost::algorithm::trim(item_type);
      cout << " [" + item_type + ']' << endl;

      time_place = item->node().select_nodes("ul/li");
      for (size_t i = 0; i < time_place.size(); i++) {
        auto c = time_place[i].node().first_child().name();
        if (time_place[i].node().first_child().name() != string("a"))
          cout << time_place[i].node().child_value();
        else
          cout << time_place[i].node().first_child().child_value();

        if (i < time_place.size() - 1)
          cout << " / ";
      }
      cout << endl;
    }
    cout << endl;
  }
}

vector<string> parse_group(const string& url, const unsigned char& dep, 
                          const unsigned char& course, const bool print = false) {
  vector<string> group_names;

  pugi::xml_document doc;
  if (exists(current_path().u8string() + "\\" + 
      to_string(+dep) + '-' + to_string(+course) + ".xml")) {
    cout << "Загружаю список групп из кэша\n\n";
    doc.load_file(string(to_string(+dep) + '-' +
                         to_string(+course) + ".xml").c_str());
  }
  else {
    doc.load_string(proceedHTML(fetchURL(url)).c_str());
    doc.save_file(string(to_string(+dep) + '-' +
                         to_string(+course) + ".xml").c_str());
  }

  string text;
  pugi::xpath_node_set groups = doc.select_nodes("/html/body/main/div/div/div/article\
/div/div");

  pugi::xpath_node_set::const_iterator group = groups.begin(), node;
  pugi::xpath_node_set nodes;

  unsigned i = 1;
  for (; group != groups.end(); ++group) {
    nodes = group->node().select_nodes("a");
    node = nodes.begin();
    for (; node != nodes.end(); ++node) {
      text = node->node().child_value();
      if (print)
        cout << setw(2) << i++ << ") " << text << endl;
      group_names.push_back(text);
    }
  }

  return group_names;
}

void printUsage() {
  cout << "TimeTable.exe {Институт} {Курс} [Номер группы из списка] \
[номер недели 1-18]\n";
  cout << "TimeTable.exe {Институт} {Курс} --list\n";
  cout << "TimeTable.exe --clear\n\n";
  cout << "  --list  - Показать только список групп\n";
  cout << "  --clear - Очистить весь кэш\n";
  exit(1);
}

const string base_url(const string group) {
  return "http://mai.ru/education/studies/schedule/index.php?group="
    + group;
}

const string week_url(const string group,
  const unsigned char& week) {
  return "http://mai.ru/education/studies/schedule/index.php?group="
    + group + "&week=" + to_string(+week);
}

const string group_url(const unsigned char& dep,
  const unsigned char& course) {
  return "https://mai.ru/education/studies/schedule/groups.php?department=Институт+№"
    + to_string(+dep) + "&course=" + to_string(+course);
}

int main(int argc, char* argv[]) {
  if (argc == 2 && !strcmp(argv[1], "--clear")) {
    unsigned cnt{ 0 };
    for (const auto& file : directory_iterator("./")) {
      if (file.path().extension() == ".xml") {
        cout << file.path().filename().u8string() << endl;
        remove(file.path());
        cnt++;
      }
    }
    cout << "Удалено файлов: " << cnt << endl;
    return 0;
  }
  else if (argc < 2 || argc > 5)
    printUsage();

  try {
    unsigned char dep = stoi(argv[1]), course = stoi(argv[2]);
    if (dep > 12 || dep == 0 ||
        course > 6 || course == 0)
      printUsage();

    vector<string> group_names;

    if (argc == 3) {
      unsigned group = 0;
      group_names = parse_group(group_url(dep, course), dep, course, true);
      while (group == 0 || group > group_names.size()) {
        cout << "Введите номер группы: ";
        cin >> group;
      }
       parse(base_url(group_names[group - 1]));
    }
    else if (argc == 4 || argc == 5) {
      unsigned group = stoi(argv[3]);
      if (argc == 4) {
        if (strcmp(argv[3], "--list")) {
          group_names = parse_group(group_url(dep, course), dep, course);

          if (group <= group_names.size() && group != 0)
            parse(base_url(group_names[group - 1]));
          else
            throw "Такого номера группы не существует";
        }
        else
          parse_group(group_url(dep, course), dep, course, true);
      }
      else if (argc == 5) {
        unsigned char week = stoi(argv[4]);
        if (week > 18 || week == 0)
          printUsage();

        group_names = parse_group(group_url(dep, course), dep, course);

        if (group <= group_names.size() && group != 0)
          parse(week_url(group_names[group - 1], week));
        else
          throw "Такого номера группы не существует";
      }
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