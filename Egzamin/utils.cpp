#include "utils.h"

void handle_error(const string& message) // obs³uga b³êdów
{
	char error_buf[256]; // bufor na komunikat b³edu
	strerror_s(error_buf, sizeof(error_buf), errno); // zapis komunikatu do bufora

	cerr << message << ": " << error_buf << endl;
}
