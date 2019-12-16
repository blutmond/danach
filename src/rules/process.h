#pragma once

#include <vector>
#include <string>

void Run(std::vector<const char*> argv);

// For legacy stdout based code...
void RunWithPipe(std::vector<const char*> argv, const char* out_filename);

// For new fangled I want to capture errors code.
int RunWithPipe(std::vector<const char*> argv,
                std::string* stdout_str,
                std::string* stderr_str);

void AssertWStatus(int wstatus);
