#include <iostream>
#include <curses.h>
#include "curses.h"
#include <string>
#include <ctime>
#include "CursesUtils.h"

using namespace std;

int main()
{
	InitializeCurses(false);

	string aString = "Testing 1, 2, 3 ..";

	mvprintw(10, 10, aString.c_str());

	GetChar();

	ShutdownCurses();

	return 0;
}