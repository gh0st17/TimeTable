#include <Manager.hpp>

using namespace std;

int main(int argc, char* argv[]) {
  Params p;

  try {
    p.checkArgc(argc);
    p.setDepCourse(argv[1], argv[2]);
    p.fetchParams(argc, argv);

    Manager m(argc, p);
    m.run();
  }
  catch (bad_alloc const&) {
    p.error_code = -1;
    cerr << "Ошибка выделения памяти\n";
  }
  catch (const std::exception& e) {
    p.error_code = -2;
    cerr << e.what() << endl;
  }
  catch (const char* e) {
    cerr << e << endl;
  }

  return p.error_code;
}