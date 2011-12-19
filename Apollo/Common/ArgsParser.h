#pragma once

/* This is suppose to be a simple generic file input parser.
 *
 * It assumes that the format of the file will be:
 *     PARAM_NAME1: value
 *	   PARAM_NAME2: value
 *
 * All arguments should be on a seperate line. All parameter names must be unique.
 *
 * The implementation is simple and naive - it has running time NxM where N
 * is the number of requested arguments and M is the number of lines in the file.
 * This should not be used for anything time consuming.
 *
 * Boolean arguments should have the value be either "true" or "false".
 * 
 */

#include <map>
#include <vector>
#include "Types.h"
#include "Error.h"

namespace Apollo{
    class ArgsParserException : public ApolloException {
    public:
        ArgsParserException(const std::string& msg) : ApolloException("ArgsParser: " + msg) {}
    };

    class ArgsParser {
    public:
	    ArgsParser(void);
	    ~ArgsParser(void);

	    void Parse(const std::string& file);
        
        /* Sets the maximum length of the parameter name. Uses default if not specified.
	     */
	    void SetMaxParameterNameLength(UINT32 length);
	
	    /* Sets the maximum length of the parameter value. Uses default if not specified.
	     */
	    void SetMaxParameterValueLength(UINT32 length);

	    /* Sets whether the parser should ignore fields that have not been specified.
	     * If ignore is set to true - parser will not report an error on parse.
	     * If ignore is set to false - parser will continue to parse values it can but
	     * error on parse.
	     * Default value is false.
	     */
	    void IgnoreUnspecifiedParams(bool ignore);

	    /* Functions to configure the parser.  For all the functions below,
	     * the first parameter is the exact (case-sensitive) name for the argument,
	     * the second parameter is the value that will be assigned upon parsing and
	     * the optional third parameter specifies whether that value is required
	     * in the file.
	     *
	     */
	    void AddBoolArg(const std::string& argName, bool& value, bool required=false); 
	    void AddCharArg(const std::string& argName, char& value, bool required=false);
	    void AddIntArg(const std::string& argName, int& value, bool required=false);
	    void AddFloatArg(const std::string& argName, float& value, bool required=false);
	    void AddStringArg(const std::string& argName, std::string& value, bool required=false); 

    private:
	    std::map<std::string, bool*> boolArgs;
	    std::map<std::string, char*> charArgs;
	    std::map<std::string, int*> intArgs;
	    std::map<std::string, float*> floatArgs;
	    std::map<std::string, std::string*> stringArgs;

	    std::map<std::string, bool> requiredParams;
        
        std::vector<char> m_stringBuffer;

	    UINT32 m_maxNameLength;
	    UINT32 m_maxValueLength;
	    bool m_ignoreUnspecified;
	    FILE* m_file;

	    bool assignBool(const char* name);
	    bool assignChar(const char* name);
	    bool assignInt(const char* name);
	    bool assignFloat(const char* name);
	    bool assignString(const char* name);

	    char* stripWhiteSpace(char* buffer);

	    void error(const char* name, const char* value);
    };
}
