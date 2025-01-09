#include "utils.h"

void handle_error(const string& message) // obs�uga b��d�w
{
	char error_buf[256]; // bufor na komunikat b�edu
	strerror_r(errno, error_buf, sizeof(error_buf)); // zapis komunikatu do bufora

	cerr << message << ": " << error_buf << endl;
}
