#include <iostream>

using namespace std;
void ChoseOption();
int main()
{
}

void ChoseOption()
{
    int option;
    cout << "what do you want screenshot[1] or screen recorder[2]: " << endl;
    cin >> option;

    if (option == 1)
    {
        // screenShot();
    }
    else if (option == 2)
    {
        // screenShot()
    }
    else
    {
        cout << "this option " << option << "is not found" << endl;
    }
}
