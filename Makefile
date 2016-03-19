CXX=g++
CXXFLAGS=-std=c++11 -g
LFLAGS=-lbluetooth
SRCDIR=src
SRC=$(wildcard $(SRCDIR)/*.cpp)
OBJDIR=obj
OBJ=$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
OUTPUT=main

.PHONY: all dirs clean

all: dirs $(OUTPUT)

dirs:
	@echo Creating directories
	@mkdir -p $(OBJDIR)

$(OUTPUT): $(OBJ)
	@echo [$(CXX)]: Linking $^
	@$(CXX) $(LFLAGS) $^ -o $@
	@echo Building $@ done!

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo [$(CXX)]: Compiling $<
	@$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	@echo Removing object files
	@rm -rf $(OBJDIR)
	@echo Removing binary file
	@rm -f main
