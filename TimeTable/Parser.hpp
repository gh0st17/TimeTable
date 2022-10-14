#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>
#include <filesystem>
#include <iomanip>

#include "Params.hpp"

class Parser {
private:
  struct group_predicate {
    bool operator()(pugi::xml_node node) const;
  };

  struct week_predicate {
    bool operator()(pugi::xml_node node) const;
  };

public:
  Parser() {};

  const std::string prepareHTML(std::string html);
  void parse(const Params& p,const std::string& url);
  void parse_group(Params& p, const std::string& url, const bool isPrint);
};

