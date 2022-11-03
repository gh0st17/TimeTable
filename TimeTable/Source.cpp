#include "Manager.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  try {
    Manager m(argc, argv);
    m.run();
  }
  catch (bad_alloc const&) {
    cerr << "Ошибка выделения памяти\n";
  }
  catch (const exception& e) {
    cerr << e.what() << endl;
  }
  return 0;
}