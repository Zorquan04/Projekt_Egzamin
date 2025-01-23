#include "utils.h"
#include "ipc.h"
#include "constants.h"

void handle_error(const string& message) // obs�uga b��d�w
{
    perror(message.c_str()); // wy�wietlenie zawarto�ci b��du
    raise(SIGINT); // wys�anie sygna�u przerwania
}

void send_signal(pid_t student, pid_t commissionA, pid_t commissionB) // przes�anie sygna�u o awarii oraz zako�czenie proces�w
{
    srand(static_cast<unsigned int>(time(NULL)));
    int random_time = rand() % 11 + 10; // mi�dzy 10 a 20s od wys�ania informacji o kierunku do student�w
    sleep(random_time);

    cout << "DZIEKAN PRZERYWA EGZAMIN - ALARM!" << endl;
    kill(student, SIGINT);
    kill(commissionA, SIGINT);
    kill(commissionB, SIGINT);
    cout << "Komisja B przeslala wyniki do dziekana." << endl;
}

void cleanup(int msg, int sem, void* ptr, int shm) // czyszczenie istniej�cych struktur (po przerwaniu awaryjnym)
{
    if (msg && msg != -1)
        destroy_msg(msg);
    if (sem && sem != -1)
        destroy_sem(sem);
    if (ptr && ptr != NULL)
        detach_shm(ptr);
    if (shm && shm != -1)
        destroy_shm(shm);
}

int get_direction() // pobranie kierunku pisz�cego egzamin przez u�ytkownika
{
    string input;
    while (true)
    {
        cout << "Wybierz numer kierunku, ktory podejdzie do egzaminu (1-5): ";
        getline(cin, input);

        try
        {
            size_t pos;
            int direction = stoi(input, &pos); // konwersja na int, `pos` wskazuje na pierwszy nieprzetworzony znak - obs�uga podania liczby z u�amkiem
            if (pos == input.size() && direction >= 1 && direction <= 5) // sprawdzamy zakres oraz czy pos zawiera identyczn� liczb� co podana przez u�ytkownika
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