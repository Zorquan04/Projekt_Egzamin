#include "utils.h"

void handle_error(const string& message) // obs³uga b³êdów
{
	char error_buf[256]; // bufor na komunikat b³edu
	strerror_r(errno, error_buf, sizeof(error_buf)); // zapis komunikatu do bufora

	cerr << message << ": " << error_buf << endl;
}
