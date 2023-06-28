#include "error.h"

Error::Error(Problem problem)
{
	m_problem = problem;
}

const char* Error::what()
{
	switch (m_problem)
	{
	case Problem::OutOfRange:
		return "Out of range.";
	case Problem::Interrupt:
		return "Interrupted by the player.";
	case Problem::FileError:
		return "Error at work with files.";
	case Problem::NoSources:
		return "No valid source files.";
	default:
		return "Unspecified problem.";
	}
}

Problem Error::problem() const
{
	return m_problem;
}
