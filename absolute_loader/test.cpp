#include "bitset"
#include "iostream"

int main()
{
  int i = 10;

  std::cout << std::bitset<4>(i).to_string();
}
