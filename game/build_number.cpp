#include "pch.h"

int build_number(const char* date)
{
	static const char* mon[12] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static char mond[12] =
	{ 31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31 };

	int m = 0;
	int d = 0;
	int y = 0;
	static int b = 0;

	if (b != 0)
		return b;

	for (m = 0; m < 11; m++)
	{
		if (_strnicmp(&date[0], mon[m], 3) == 0)
			break;
		d += mond[m];
	}

	d += atoi(&date[4]) - 1;

	y = atoi(&date[7]) - 1900;

	b = d + (int)((y - 1) * 365.25);

	if (((y % 4) == 0) && m > 1)
	{
		b += 1;
	}

	b -= RELEASE_DAY; // Oct 24 1996

	return b;
}