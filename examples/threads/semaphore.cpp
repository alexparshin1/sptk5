#include <sptk5/cutils>
#include <sptk5/cthreads>

using namespace std;
using namespace sptk;

int main()
{
    CSemaphore  semaphore;
    try {
        semaphore.post();
        cout << "Semaphore posted       (Ok)" << endl;
        if (semaphore.wait(1000))
            cout << "Semaphore was posted   (Ok)" << endl;
        else
            cout << "Semaphore wait timeout (Error)" << endl;
        if (semaphore.wait(1000))
            cout << "Semaphore was posted   (Error)" << endl;
        else
            cout << "Semaphore wait timeout (Ok)" << endl;
    }
    catch (exception& e) {
        cout << e.what() << endl;
    }
    return 0;
}
