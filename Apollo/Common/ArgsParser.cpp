#include "ArgsParser.h"
#include "Error.h"
#include "Macros.h"
#include <vector>

#define DEFAULT_MAX_NAMELENGTH 256
#define DEFAULT_MAX_VALUELENGTH 1024

using namespace std;

namespace Apollo {

ArgsParser::ArgsParser() {
	m_maxNameLength = DEFAULT_MAX_NAMELENGTH;
	m_maxValueLength = DEFAULT_MAX_VALUELENGTH;
	m_ignoreUnspecified = false;
    m_file = nullptr;
}

ArgsParser::~ArgsParser() {
    if (m_file) fclose(m_file);
}

void ArgsParser::Parse(const string& filename) {    
    try {
        m_stringBuffer.resize(m_maxValueLength);
        m_stringBuffer.clear();

        vector<char> bufferData(m_maxNameLength);
        char* buffer = &bufferData[0];

        fopen_s(&m_file, filename.c_str(), "r");
	    if (m_file == nullptr) {
            throw ArgsParserException("Invalid file: ." + filename);
	    }

	    while (!feof(m_file)) {
		    bool found = false;

		    // Extract the name of the parameter 
		    fscanf_s(m_file, "%s:", buffer, m_maxNameLength);
		    buffer = stripWhiteSpace(buffer);

		    // Strip the last :
		    if (buffer[strlen(buffer) - 1] == ':') {
			    buffer[strlen(buffer) - 1] = '\0';
		    }

		    // Try to match with all of the types 
		    if (!found) found =	assignBool(buffer);
		    if (!found) found = assignChar(buffer);
		    if (!found) found = assignInt(buffer);
		    if (!found) found = assignFloat(buffer);
		    if (!found) found = assignString(buffer);

		    // If not found - check the error type and do some clean up 
		    if (!found) {
                // Found something in the file the parser is not expecting.  Throw a warning.
                ApolloException::NotYetImplemented();
                // Clean up trailing newlines 
			    fgets(buffer, DEFAULT_MAX_NAMELENGTH, m_file);
		    } else {
		        // Take it out of the required buffer
			    map<string, bool>::iterator iter;
			    iter = requiredParams.find(buffer);
			    if (iter != requiredParams.end()) {
				    requiredParams[buffer] = true;
			    }
		    }
	    }

	    // Check for required 
	    map<string, bool>::iterator iter = requiredParams.begin();
	    for (; iter != requiredParams.end(); iter++) {
		    if (iter->second == false) {
			    throw ArgsParserException("Required parameter missing.");
		    }
	    }
    } catch (const bad_alloc&) {
        if (m_file) {
            fclose(m_file);
            m_file = nullptr;
        }
        throw ApolloOOMException();
    } catch (const ApolloException& e) {
        if (m_file) {
            fclose(m_file);
            m_file = nullptr;
        }
        throw e;
    }
    
    if (m_file) {
        fclose(m_file);
        m_file = nullptr;
    }
}

bool ArgsParser::assignBool(const char* name) {
	/* See if the map contains the name */
	map<string, bool*>::iterator iter;
	iter = boolArgs.find(name);
	if (iter == boolArgs.end()) return false;

	/* Parse the file for the value */
	char buffer[DEFAULT_MAX_NAMELENGTH];
	char* buf;

	fgets(buffer, DEFAULT_MAX_NAMELENGTH, m_file);
	buf = stripWhiteSpace(buffer);

	if (!strcmp(buf, "true")) {
		*(iter->second) = true;
	} else if (!strcmp(buf, "false")) {
		*(iter->second) = false;
	} else {
		error(name, buf);
	}

	return true;
}

bool ArgsParser::assignChar(const char* name) {
	/* See if the map contains the name */
	map<string, char*>::iterator iter;
	iter = charArgs.find(name);
	if (iter == charArgs.end()) return false;

	/* Parse the file for the value */
	fgets(&m_stringBuffer[0], DEFAULT_MAX_NAMELENGTH, m_file);
	char* buf = stripWhiteSpace(&m_stringBuffer[0]);
	*(iter->second) = buf[0];
	return true;
}

bool ArgsParser::assignInt(const char* name) {
	/* See if the map contains the name */
	map<string, int*>::iterator iter;
	iter = intArgs.find(name);
	if (iter == intArgs.end()) return false;

	/* Parse the file for the value */
	int value;
	fscanf_s(m_file, "%d", &value);
	*(iter->second) = value;
	return true;
}

bool ArgsParser::assignFloat(const char* name) {
	/* See if the map contains the name */
	map<string, float*>::iterator iter;
	iter = floatArgs.find(name);
	if (iter == floatArgs.end()) return false;

	/* Parse the file for the value */
	float value;
	fscanf_s(m_file, "%f", &value);
	*(iter->second) = value;
	return true;
}

bool ArgsParser::assignString(const char* name) {
	/* See if the map contains the name */
	map<string, string*>::iterator iter;
	iter = stringArgs.find(name);
	if (iter == stringArgs.end())
		return false;

	/* Parse the file for the value */
	fgets(&m_stringBuffer[0], DEFAULT_MAX_NAMELENGTH, m_file);
	char* buf = stripWhiteSpace(&m_stringBuffer[0]);

	*(iter->second) = buf;
	return true;
}

char* ArgsParser::stripWhiteSpace(char* buffer) {
	while (*buffer == ' ' || *buffer == '\n' || *buffer == '\t')
		buffer++;
	size_t len = strlen(buffer);
	while (buffer[len - 1] == ' ' || buffer[len - 1] == '\n' || buffer[len - 1] == '\t') {
		buffer[len - 1] = '\0';
		len--;
	}
	return buffer;
}

void ArgsParser::error(const char* name, const char* value) {
    string message = "Invalid parameter: " + string(name) + ". Value of " + value + " is not valid.";
    throw ArgsParserException(message);
}

void ArgsParser::AddBoolArg(const string& argName, bool &value, bool required) {
	boolArgs[argName] = &value;
	if (required) {
		requiredParams[argName] = false;
	}
}
void ArgsParser::AddCharArg(const string& argName, char &value, bool required) {
	charArgs[argName] = &value;
	if (required) {
		requiredParams[argName] = false;
	}
}
void ArgsParser::AddIntArg(const string& argName, int &value, bool required) {
	intArgs[argName] = &value;
	if (required) {
		requiredParams[argName] = false;
	}
}
void ArgsParser::AddFloatArg(const string& argName, float &value, bool required) {
	floatArgs[argName] = &value;
	if (required) {
		requiredParams[argName] = false;
	}
}
void ArgsParser::AddStringArg(const string& argName, string &value, bool required) {
	stringArgs[argName] = &value;
	if (required) {
		requiredParams[argName] = false;
	}
}

void ArgsParser::SetMaxParameterNameLength(unsigned int length) {
	m_maxNameLength = length;
}

void ArgsParser::SetMaxParameterValueLength(unsigned int length) {
	m_maxValueLength = length;
}

void ArgsParser::IgnoreUnspecifiedParams(bool ignore) {
	m_ignoreUnspecified = ignore;
}

}