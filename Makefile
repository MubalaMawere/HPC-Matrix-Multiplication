CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -fopenmp -Iinclude
TARGET := build/matrix_program
SOURCES := $(wildcard src/*.cpp)
VENV_DIR := .venv
VENV_PYTHON := $(VENV_DIR)/bin/python
VENV_PIP := $(VENV_DIR)/bin/pip
PYTHON_DEPS_STAMP := $(VENV_DIR)/.deps-installed

.PHONY: all run venv activate charts graphs clean clean-python

all: $(TARGET)

$(TARGET): $(SOURCES) | build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET)

venv: $(VENV_PYTHON)

$(VENV_PYTHON):
	python3 -m venv $(VENV_DIR)

activate: $(VENV_PYTHON)
	@echo "Starting a new shell with $(VENV_DIR) activated. Type 'exit' to leave it."
	@bash -ic 'source "$(abspath $(VENV_DIR))/bin/activate"; echo "Virtual environment active: $$VIRTUAL_ENV"; exec env PATH="$$PATH" VIRTUAL_ENV="$$VIRTUAL_ENV" PS1="$$PS1" bash --norc -i'

$(PYTHON_DEPS_STAMP): requirements.txt | $(VENV_PYTHON)
	$(VENV_PIP) install --retries 10 --timeout 120 -r requirements.txt
	touch $(PYTHON_DEPS_STAMP)

charts: $(PYTHON_DEPS_STAMP)
	$(VENV_PYTHON) tools/create_charts_and_graphs.py

graphs: charts

clean:
	rm -rf build

clean-python:
	rm -rf $(VENV_DIR)
