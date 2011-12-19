#pragma once

#include <exception>
#include <string>

namespace Apollo {
	class ApolloException : public std::exception {
	public:
		virtual const char* what() const throw() {
			return m_message.c_str();
		};

		ApolloException() : m_message("Exception thrown.") {}
		ApolloException(std::string message) : m_message(message) {}

		static void NotYetImplemented() { throw ApolloException("Not yet implemented."); }

        static __forceinline void DebugBreak() { __debugbreak(); }

	protected:
		std::string m_message;
	};

    class ApolloOOMException : public ApolloException {
    public:
        ApolloOOMException() : ApolloException("Out of memory.") {}
    };
}
