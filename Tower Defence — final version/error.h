#pragma once
#include <exception>

enum class Problem
{
	Unspecified, OutOfRange, Interrupt, FileError,
	NoSources
};

class Error : public std::exception
{
	Problem m_problem;
public:
	Error(Problem problem = Problem::Unspecified);
	const char* what();
	Problem problem() const;
};
