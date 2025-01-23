# Kompilator i flagi
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I$(PROJECT_DIR)/StudentApp -I$(PROJECT_DIR)/DeanApp -I$(PROJECT_DIR)/CommissionAApp -I$(PROJECT_DIR)/CommissionBApp

# Lokacje folderów
PROJECT_DIR = .
STUDENT_DIR = $(PROJECT_DIR)/StudentApp
DEAN_DIR = $(PROJECT_DIR)/DeanApp
COMMISSION_A_DIR = $(PROJECT_DIR)/CommissionAApp
COMMISSION_B_DIR = $(PROJECT_DIR)/CommissionBApp

# Pliki Ÿród³owe i nag³ówki
COMMON_SRCS = utils.cpp ipc.cpp exam_logic.cpp
COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)

STUDENT_MAIN = StudentApp/main_student.cpp
DEAN_MAIN = DeanApp/main_dean.cpp
COMMISSION_A_MAIN = CommissionAApp/main_commission_a.cpp
COMMISSION_B_MAIN = CommissionBApp/main_commission_b.cpp

# Cele
all: student dean commissionA commissionB

student: $(addprefix $(STUDENT_DIR)/, $(COMMON_OBJS)) $(STUDENT_DIR)/$(STUDENT_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

dean: $(addprefix $(DEAN_DIR)/, $(COMMON_OBJS)) $(DEAN_DIR)/$(DEAN_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

commissionA: $(addprefix $(COMMISSION_A_DIR)/, $(COMMON_OBJS)) $(COMMISSION_A_DIR)/$(COMMISSION_A_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

commissionB: $(addprefix $(COMMISSION_B_DIR)/, $(COMMON_OBJS)) $(COMMISSION_B_DIR)/$(COMMISSION_B_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^


# Wspólna regu³a dla plików .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Czyszczenie
clean:
	rm -f $(STUDENT_DIR)/*.o $(DEAN_DIR)/*.o $(COMMISSION_A_DIR)/*.o $(COMMISSION_B_DIR)/*.o student dean commissionA commissionB

.PHONY: all clean
