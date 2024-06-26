#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>
#include <filesystem>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <memory>
#include <regex>
#include <map>

#include <Params.hpp>

using namespace boost::gregorian;
using namespace boost::posix_time;

struct Item {
  std::vector<std::string> educators;
  std::vector<std::string> places;
  std::string item_type;
  std::string name;
  ptime time;
};

using Items = std::vector<Item>;

struct Day {
  date pdate = day_clock::local_day();
  Items items;
};

using Days = std::vector<Day>;

struct TimeTable {
  unsigned short week{ 0 };
  Days days;
  std::string group;
};

class Parser {
private:
  const char* schedule_path = "/html/body/main/div/div/div/article/ul/li";
  const char* session_path  = "/html/body/main/div/div/div/article/div/ul/li";
  const char* group_path    = "/html/body/main/div/div/div/article/div/div";

  struct week_predicate {
    const bool operator()(pugi::xml_node node) const;
  };

  const std::map<std::string, unsigned short> month = { {"января", 1}, {"февраля", 2},
    {"марта", 3}, {"апреля", 4}, {"мая", 5}, {"июня", 6}, {"июля", 7}, {"августа", 8},
    {"сентября", 9}, {"октября", 10}, {"ноября", 11}, {"декабря", 12} };

  void prepareHTML(std::string& html) const;
  const bool loadDocument(const Params& p, pugi::xml_document& doc, const std::string& url) const;
  const pugi::xpath_node_set download_days(TimeTable& tt, const Params& p, const std::string& url) const;
  const std::string matchRegex(const std::string str, const std::regex r, const size_t i = 1) const;

public:
  Parser() {};

  void parse(TimeTable& tt, const Params& p, const std::string& url) const;
  void parse_group(Params& p, const std::string& url, const bool isPrint) const;
};

