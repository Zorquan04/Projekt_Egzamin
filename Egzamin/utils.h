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

void handle_error(const string&); // obs�uga b��d�w
void send_signal(pid_t, pid_t, pid_t); // wysy�anie alarmu przez dziekana
void cleanup(int, int, void*, void*, int); // sprz�tanie zasob�w
int get_direction(); // pobieranie kierunku od u�ytkownika

#endif
