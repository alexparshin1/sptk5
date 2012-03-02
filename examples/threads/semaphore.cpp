#include <sptk5/cutils>

#include <iostream>

using namespace std;
using namespace sptk;

int main()
{
    CSemaphore  semaphore;
    try {
        semaphore.wait(2000);
    }
    catch (exception& e) {
        cout << e.what() << endl;
    }
    return 0;
}
