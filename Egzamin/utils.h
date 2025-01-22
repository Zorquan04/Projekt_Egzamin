#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <csignal>
#include <cstring>
#include <string>
#include <cerrno>
#include <limits>
#include <unistd.h>
#include <algorithm>
using namespace std;

void handle_error(const string&); // obs³uga b³êdów
void send_signal(pid_t, pid_t, pid_t); // wysy³anie alarmu przez dziekana
void cleanup(int, int, void*, void*, int); // sprz¹tanie zasobów
int get_direction(); // pobieranie kierunku od u¿ytkownika

#endif
