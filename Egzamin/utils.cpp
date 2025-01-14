#include "utils.h"

void handle_error(const string& message) // obs³uga b³êdów
{
	char error_buf[256]; // bufor na komunikat b³edu
	strerror_r(errno, error_buf, sizeof(error_buf)); // zapis komunikatu do bufora

	cerr << message << ": " << error_buf << endl;
}

int get_direction()
{
    string input;
    while (true)
    {
        cout << "Wybierz numer kierunku, ktory podejdzie do egzaminu (1-5): ";
        getline(cin, input);

        try
        {
            size_t pos;
            int direction = stoi(input, &pos); // konwersja na int, `pos` wskazuje na pierwszy nieprzetworzony znak - obs³uga podania liczby z u³amkiem
            if (pos == input.size() && direction >= 1 && direction <= 5) // sprawdzamy zakres oraz czy pos zawiera identyczn¹ liczbê co podana przez u¿ytkownika
                return direction;
            else
                cout << "Numer poza zakresem lub liczba z ulamkiem. Sprobuj ponownie." << endl;
        }
        catch (...)
        {
            cout << "Nieprawidlowy typ danych. Sprobuj ponownie." << endl;
        }
    }
}
