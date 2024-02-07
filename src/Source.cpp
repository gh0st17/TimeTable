#include <Manager.hpp>

using namespace std;

int main(int argsc, char* argv[]) {
  try {
    unsigned argc = argsc;
    Manager m(argc, argv);
    m.run();
  }
  catch (bad_alloc const&) {
    cerr << "Ошибка выделения памяти\n";
  }
  catch (const char* e) {
    cerr << e << endl;
  }
  return 0;
}