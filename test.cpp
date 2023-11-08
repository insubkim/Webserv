#include <string>
#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char** argv, char** envp){
  vector<string> env;
  for (int i = 0; i < argc; i++) {
    env.push_back(argv[i]);
  }
  for (int i = 0; i < env.size(); i++) {
    cout << env[i] << endl;
  }
  return 0;
}