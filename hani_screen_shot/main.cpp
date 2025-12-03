#include <iostream>
#include "screen_shot.h"
#include "video-generator.h"
using namespace std;
void ChoseOption();
int main()
{
    ChoseOption();
}

void ChoseOption()
{
    int option;
    cout << "what do you want screenshot[1] or screen recorder[2]: " << endl;
    cin >> option;

    if (option == 1)
    {
        screenShot(1);
    }
    else if (option == 2)
    {
        int time;
        cout << "How long do you want the recording to be?" << endl;
        cin >> time;
        int lastNumber = screenShot(time);
        videoGenerator(lastNumber);
    }
    else
    {
        cout << "this option " << option << "is not found" << endl;
    }
}
