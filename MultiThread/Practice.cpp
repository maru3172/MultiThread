#include <iostream>

int sum;

int main()
{
	for (int i = 0; i < 50000000; ++i) sum = sum + 2;
	std::cout << "sum = " << sum << std::endl;
}