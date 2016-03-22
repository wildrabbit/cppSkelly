#include "logUtils.h"

void logError(const char* string)
{
	std::cerr << "ERROR: " << __FILE__ << "[" << __LINE__ << "] - " << string << std::endl;
}

void logInfo(const char* string)
{
	std::cout << "INFO: " << string << std::endl;
}