#pragma once

#include <vector>

void Run(std::vector<const char*> argv);

// For legacy stdout based code...
void RunWithPipe(std::vector<const char*> argv, const char* out_filename);
