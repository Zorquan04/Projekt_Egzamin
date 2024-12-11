#include "utils.h"

void handle_error(const string& message) // obs�uga b��d�w
{
	char error_buf[256]; // bufor na komunikat b�edu
	strerror_s(error_buf, sizeof(error_buf), errno); // zapis komunikatu do bufora

	cerr << message << ": " << error_buf << endl;
}
