#ifndef EXCEPTION_ERROR_HPP
#define EXCEPTION_ERROR_HPP

#include <exception>
#include <string>

namespace djg {

/// \class computecl_error
/// \brief A run-time ComputeCL error.
///
class computecl_error : public std::exception
{
public:
    /// Creates a new computecl_error exception object for \p error.
    explicit computecl_error(const std::string & str) throw()
        : m_error_string(str)
    {
    }

    /// Destroys the computecl_error object.
    ~computecl_error() throw()
    {
    }

    /// Returns a string description of the error.
    std::string error_string() const throw()
    {
        return m_error_string;
    }

    /// Returns a C-string description of the error.
    const char* what() const throw()
    {
        return m_error_string.c_str();
    }


private:
    std::string m_error_string;
};

}

#endif // EXCEPTION_ERROR_HPP

