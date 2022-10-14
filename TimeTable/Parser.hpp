#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>
#include <filesystem>
#include <iomanip>

#include "Params.hpp"

class Parser {
private:
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

public:
  Parser() {};

  const std::string prepareHTML(std::string html);
  void parse(const Params& p,const std::string& url);
  void parse_group(Params& p, const std::string& url, const bool isPrint);
};

