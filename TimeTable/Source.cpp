#include "Manager.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  Manager m(argc, argv);
  m.getTimeTable();
  m.printTimeTable();
  return 0;
}