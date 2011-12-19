#include "ApolloCommon.h"
#include "ApolloEngine.h"
#include <iostream>
#include <sstream>
using namespace std;

#if 0
class Material
{
};

class TextureMap
{
};

class TransformNode
{
};

struct Vec4d
{
    double x, y, z, w;
};

struct MaterialParameter
{
};

class Scene
{

public:
	Scene() 
    {
    }

	virtual ~Scene();
    
    void add( Apollo::Model* )
	{
	}
	void add( Apollo::Light* )
    {
    }

    Apollo::PerspectiveCamera* getCamera()
    {
        return nullptr;
    }

	// For efficiency reasons, we'll store texture maps in a cache
	// in the Scene.  This makes sure they get deleted when the scene
	// is destroyed.
	TextureMap* getTexture( string name );

	// These two functions are for handling ambient light; in the Phong model,
	// the "ambient" light is considered a property of the _scene_ as a whole
	// and hence should be set here.
	void addAmbient( const Apollo::Vector3& ambient ) { ambientIntensity += ambient; }

private:
	Apollo::Vector3 ambientIntensity;

	typedef std::map< std::string, TextureMap* > tmap;
	tmap textureCache;
};

/* These are some exceptions used by the parser class. 
   You shouldn't need to mess with them. 
*/

using std::string;
using std::ostream;

class Tokenizer;

class ParserException : public Apollo::ApolloParserException
{
  public:
    ParserException( const std::string& msg ) : ApolloParserException(msg)
    { }
};

class ParserFatalException
  : public ParserException
{
  public:
    ParserFatalException( const std::string& msg )
      : ParserException( msg )
    { }
};

class SyntaxErrorException
  : public ParserException
{
   public:
     SyntaxErrorException( const std::string& msg, const Tokenizer& tok );
     string formattedMessage( ) const { return _formattedMsg; }

  private:
     string _formattedMsg;
};




/* These are possible tokens we might encounter in a
   .ray file.  These include things like punctuation,
   keywords, etc.

   If you are adding your own token, you must add it 
   to the list below (I recommend at the bottom) and 
   also to the lookup table (see Token.cpp).  If you
   are adding a reserved word (you probably are) you
   need also to add it to the reserved words lookup 
   table in Token.cpp.
*/

enum SYMBOL {
  UNKNOWN,					// Placeholder
  EOFSYM,					// End of file
  SBT_RAYTRACER,

  IDENT,					// Ident (gets enclosed in quotes in trace files)
  SCALAR,				 	// Scalar values
  SYMTRUE,
  SYMFALSE,

  LPAREN, RPAREN,			// Punctuation
  LBRACE, RBRACE,
  COMMA,
  EQUALS,
  SEMICOLON,

  CAMERA,					// camera primitive
  POINT_LIGHT,				// lights
  DIRECTIONAL_LIGHT,
  AMBIENT_LIGHT,

  CONSTANT_ATTENUATION_COEFF,	// Terms affecting the intensity dropoff
  LINEAR_ATTENUATION_COEFF,		// of point lights (see the PointLight 
  QUADRATIC_ATTENUATION_COEFF,	// class)

  SPHERE,					// primitives
  BOX,
  SQUARE,
  CYLINDER,
  CONE,
  TRIMESH,  

  POSITION, VIEWDIR,		// keywords affecting primitives
  UPDIR, ASPECTRATIO,
  FOV,
  COLOR,
  DIRECTION,
  CAPPED,
  HEIGHT,
  BOTTOM_RADIUS,
  TOP_RADIUS,
  QUATERNIAN,               // ???

  POLYPOINTS, NORMALS,			// keywords affecting polygons
  MATERIALS, FACES,
  GENNORMALS,

  TRANSLATE, SCALE,			// Transforms
  ROTATE, TRANSFORM,

  MATERIAL, 				// Material settings
  EMISSIVE, AMBIENT, 
  SPECULAR, REFLECTIVE,
  DIFFUSE, TRANSMISSIVE,
  SHININESS, INDEX,
  NAME,
  MAP,
  LOOK_AT
};

// Helper functions
string getNameForToken( const SYMBOL kind );
SYMBOL lookupReservedWord( const string& name );

class Token {
  public:
    Token(SYMBOL kind) : _kind( kind ) { }

    SYMBOL kind() const { return _kind; }

    // Note that these errors should not ever be encountered at runtime,
    // and signify parser bugs of some kind.
    virtual std::string ident() const   
      { throw ParserFatalException("not an IdentToken"); return std::string(); }
    virtual double value() const   
      { throw ParserFatalException("not a ScalarToken"); return 0.0; }


    // Utility functions
    virtual void Print(std::ostream& out) const;
    virtual void Print() const;
    virtual string toString() const;

	virtual ~Token() {}
  protected:
    SYMBOL _kind;
};

class IdentToken : public Token {
  public:
    IdentToken(std::string ident) : Token(IDENT), _ident( ident ) { 
    }

    std::string ident() const { return _ident; }

    string toString() const;

  protected:
    std::string _ident;
};

class ScalarToken : public Token {
  public:
    ScalarToken(double value) : Token(SCALAR), _value( value ) { }
    
    double value() const { return _value; }

    string toString() const;

  protected:
    double _value;
};


class Buffer {
 public:
  Buffer(std::istream& file, bool printChars, bool printLines);
  
  Buffer& operator= (const Buffer&)
  {
      Apollo::ApolloException::NotYetImplemented();
  }

  char GetCh();			// Read and return next character
  bool isEOF() { return !(inStream); }	// Return whether is end of file

  void PrintLine(std::ostream& out) const;		// Print current line

  int  CurColumn() const { return ColNumber; }
  int  CurLine() const { return LineNumber; }	// Return current line #
  
protected:
  void  GetLine();		// Read next line from file

  std::istream& inStream;            // The file pointer to the source file

  std::string Line;                  // The line buffer
  std::string::const_iterator PositionInCurrentLine;

  int   ColNumber;		// The number of the current column
  int   LineNumber;             // The number of the line in the file
  mutable int   LastPrintedLine;        // The line number of the last printed line

  bool _printLines, _printChars; // printing flags
  bool _bHasData; // Whether the Buffer has data to read
};

/*
   The tokenizer's job is to convert a stream of characters
   into a stream of tokens that the parser can then handle.
   You probably aren't going to be modifying this file;
   most of the stuff that you might want to change is in
   Token.{h,cpp} and Parser.{h,cpp}.

   This tokenizer is based on the tokenizer from the
   PL0 project used for CSE401
   (http://www.cs.washington.edu/401).

*/

class Tokenizer {
  public:
    Tokenizer(istream& fp, bool printTokens);

    Tokenizer& operator= (const Tokenizer&)
      {
          Apollo::ApolloException::NotYetImplemented();
      }

    // destructively read & return the next token, skipping over whitespace
    auto_ptr<Token> Get();

    // non-destructively get the next token, pushing it back to be read again
    const Token* Peek();

    // Get() the next token, and check that it's of the expected SYMBOL type
    auto_ptr<Token> Read(SYMBOL expected);

    // read the next token only if it matches the expected token type.
    // Return whether it matches.
    bool CondRead(SYMBOL expected);

    // display the current source line onto the screen.
    void PrintLine( ostream& out) const { buffer.PrintLine(out); }

    // return the column number/line number of the current token.
    int CurColumn() const { return TokenColumn; }
    int CurLine() const { return buffer.CurLine(); }

    // Repeatedly scan tokens and throw them away.  Useful if this is the
    // last phase to be executed
    void ScanProgram();

protected:
    // private methods:

    // push the argument token back onto the scanner's token stream;
    // it will be returned by the next Get/Peek/Read/CondRead call
    Token* GetNext();
    void UnGet(Token* t);

    Token* SearchReserved(const string&) const; // Convert ident string into token

    void GetCh() { CurrentCh = buffer.GetCh(); }
    bool CondReadCh(char expected);        // consume a character, if it matches

    void SkipWhiteSpace();        // skip spaces, tabs, newlines

    Token* GetPunct();            // scan punctuation token
    Token* GetScalar();           // scan integer token
    Token* GetIdent();            // scan identifier token
    Token* GetQuotedIdent();


    // private data:

    Buffer buffer;                // The file buffer
    char CurrentCh;               // The current character in the current line

    Token* UnGetToken;            // The token that has been "ungot"

    int TokenColumn;              // The column where the last read token starts,
                                  // for generating error messages

    bool _printTokens;            // printing flag
};

typedef std::map<std::string, Material> mmap;

/*
  class Parser:
    The Parser is where most of the heavy lifting in parsing
    goes.  This particular parser reads in a stream of tokens
    from the Tokenizer and converts them into a scene in 
    memory.  

    If you really want to know, this parser is written
    as a top-down parser with one symbol of lookahead.
    See the docs on the website if you're interested in
    modifying this somehow.
*/

class Parser
{
  public:
    // We need the path for referencing files from the
    // base file.
    Parser( Tokenizer& tokenizer, string basePath )
      : _tokenizer( tokenizer ), _basePath( basePath )
      { }

    // Parse the top-level scene
    Scene* parseScene();

    Parser& operator=(const Parser&)
    {
        Apollo::ApolloException::NotYetImplemented();
    }
private:

    // Highest level parsing routines
    void parseTransformableElement( Scene* scene, TransformNode* transform, const Material& mat );
    void parseGroup( Scene* scene, TransformNode* transform, const Material& mat );
	  void parseCamera( Scene* scene );

    void parseGeometry( Scene* scene, TransformNode* transform, const Material& mat );


    // Parse lights
	Apollo::Light* parsePointLight( Scene* scene );
	Apollo::Light* parseDirectionalLight( Scene* scene );
	void parseAmbientLight( Scene* scene );

    // Parse geometry
    void      parseSphere(Scene* scene, TransformNode* transform, const Material& mat);
    void      parseBox(Scene* scene, TransformNode* transform, const Material& mat);
    void      parseSquare(Scene* scene, TransformNode* transform, const Material& mat);
    void      parseCylinder(Scene* scene, TransformNode* transform, const Material& mat);
    void      parseCone(Scene* scene, TransformNode* transform, const Material& mat);
    void      parseTrimesh(Scene* scene, TransformNode* transform, const Material& mat);
    void      parseFaces( std::list< Apollo::Vector3 >& faces );

    // Parse transforms
    void parseTranslate(Scene* scene, TransformNode* transform, const Material& mat);
    void parseRotate(Scene* scene, TransformNode* transform, const Material& mat);
    void parseScale(Scene* scene, TransformNode* transform, const Material& mat);
    void parseTransform(Scene* scene, TransformNode* transform, const Material& mat);

    // Helper functions for parsing expressions of the form:
    //   keyword = value;
    double parseScalarExpression();
    Apollo::Vector3 parseVec3dExpression();
    Vec4d parseVec4dExpression();
    bool parseBooleanExpression();
    Material* parseMaterialExpression(Scene* scene, const Material& mat);
    string parseIdentExpression();

    MaterialParameter parseVec3dMaterialParameter(Scene* scene);
    MaterialParameter parseScalarMaterialParameter(Scene* scene);


    // Helper functions for parsing things like vectors
    // and idents.
    double parseScalar();
    std::list<double> parseScalarList();
    Apollo::Vector3 parseVec3d();
    Vec4d parseVec4d();
    bool parseBoolean();
    Material* parseMaterial(Scene* scene, const Material& parent);
    string parseIdent();

  private:
    Tokenizer& _tokenizer;
    mmap materials;
    std::string _basePath;
};

SyntaxErrorException::SyntaxErrorException( const string& msg, const Tokenizer& tokenizer )
	: ParserException( msg )
{
  std::ostringstream out;
  tokenizer.PrintLine( out );
  out << "  ";
  for( int i = 0; i < tokenizer.CurColumn(); i++ ) {
    out << " ";
  }
  out << "^" << std::endl;

  out << "Line " << tokenizer.CurLine() << ": syntax error: "
    << what() << std::endl;

  _formattedMsg = out.str();

}

/* This function is more of a convenience than
   anything; for debugging purposes and intelligent
   error messages we'd like to be returning something
   other than "token #2843" for human-readable output;
   this lookup table solves that.

   If you add tokens, simply add them to this table
   with 
     tokenNames[ MY_TOKEN_NAME ] = "string representation";
*/ 
string getNameForToken( const SYMBOL kind )
{
  static std::map<int, string> tokenNames;

  if( tokenNames.empty() )
  {
    tokenNames[ EOFSYM ]            = "EOF";
    tokenNames[ SBT_RAYTRACER ]     = "SBT-raytracer";
    tokenNames[ IDENT ]             = "Identifier";
    tokenNames[ SCALAR ]            = "Scalar";
    tokenNames[ SYMTRUE ]           = "true";
    tokenNames[ SYMFALSE ]          = "false";
    tokenNames[ LPAREN ]            = "Left paren";
    tokenNames[ RPAREN ]            = "Right paren";
    tokenNames[ LBRACE ]            = "Left brace";
    tokenNames[ RBRACE ]            = "Right brace";
    tokenNames[ COMMA ]             = "Comma";
    tokenNames[ EQUALS ]            = "Equals";
    tokenNames[ SEMICOLON ]         = "Semicolon";
    tokenNames[ CAMERA ]            = "camera";
	tokenNames[ AMBIENT_LIGHT ]     = "ambient_light";
    tokenNames[ POINT_LIGHT ]       = "point_light";
    tokenNames[ DIRECTIONAL_LIGHT ] = "directional_light";
    tokenNames[ CONSTANT_ATTENUATION_COEFF ] = "constant_attenuation_coeff";
    tokenNames[ LINEAR_ATTENUATION_COEFF ] = "linear_attenuation_coeff";
    tokenNames[ QUADRATIC_ATTENUATION_COEFF ] = "quadratic_attenuation_coeff";
    tokenNames[ SPHERE ]            = "sphere";
    tokenNames[ BOX ]               = "box";
    tokenNames[ SQUARE ]            = "square";
    tokenNames[ CYLINDER ]          = "cylinder";
    tokenNames[ CONE ]              = "cone";
    tokenNames[ TRIMESH ]           = "trimesh";
    tokenNames[ POSITION ]          = "position";
    tokenNames[ VIEWDIR ]           = "viewdir";
    tokenNames[ UPDIR ]             = "updir";
    tokenNames[ ASPECTRATIO ]       = "aspectratio";
    tokenNames[ COLOR ]             = "color";
    tokenNames[ DIRECTION ]         = "direction";
    tokenNames[ CAPPED ]            = "capped";
    tokenNames[ HEIGHT ]            = "height";
    tokenNames[ BOTTOM_RADIUS ]     = "bottom_radius";
    tokenNames[ TOP_RADIUS ]        = "top_radius";
    tokenNames[ QUATERNIAN ]        = "quaternian";
    tokenNames[ POLYPOINTS ]        = "points";
    tokenNames[ HEIGHT ]            = "height";
    tokenNames[ NORMALS ]           = "normals";
    tokenNames[ MATERIALS ]         = "materials";
    tokenNames[ FACES ]             = "faces";
    tokenNames[ TRANSLATE ]         = "translate";
    tokenNames[ SCALE ]             = "scale";
    tokenNames[ ROTATE ]            = "rotate";
    tokenNames[ TRANSFORM ]         = "transform";
    tokenNames[ MATERIAL ]          = "material";
    tokenNames[ EMISSIVE ]          = "emissive";
    tokenNames[ AMBIENT ]           = "ambient";
    tokenNames[ SPECULAR ]          = "specular";
    tokenNames[ REFLECTIVE ]        = "reflective";
    tokenNames[ DIFFUSE ]           = "diffuse";
    tokenNames[ TRANSMISSIVE ]      = "transmissive";
    tokenNames[ SHININESS ]         = "shininess";
    tokenNames[ INDEX ]             = "index";
    tokenNames[ NAME ]              = "name";
    tokenNames[ MAP ]               = "map";
	tokenNames[ LOOK_AT ]			= "look_at";
  }
  // search tokenNames table
  std::map<int, string>::const_iterator itr = 
    tokenNames.find( kind );
  if( itr == tokenNames.end() )
    return string("Unknown token type");
  else
    return (*itr).second;

}

/* This function is used by the parser to lookup 
   "reserved" words (i.e., things like "sphere", "cone",
   etc.).  What you will be concerned with is adding
   entries to the reservedWords map as appropriate;
   if you add a new reserved word to the parser, 
   simply add it to the list below.  I.e., if you had
   the reserved word "regular17gon" as your new primitive,
   for example, and the SYMBOL representing it was
   "SEVENTEENGON", you'd add the line
      reservedWords["regular17gon"] = SEVENTEENGON;
   to the list below.
*/
SYMBOL lookupReservedWord(const string& ident) {
  static std::map<string, SYMBOL> reservedWords;

  if( reservedWords.empty() )
  {
    reservedWords["ambient_light"] = AMBIENT_LIGHT;
    reservedWords["ambient"] = AMBIENT;
    reservedWords["aspectratio"] = ASPECTRATIO;
    reservedWords["bottom_radius"] = BOTTOM_RADIUS;
    reservedWords["box"] = BOX;
    reservedWords["camera"] = CAMERA;
    reservedWords["capped"] = CAPPED;
    reservedWords["color"] = COLOR;
    reservedWords["colour"] = COLOR;
    reservedWords["cone"] = CONE;
    reservedWords["constant_attenuation_coeff"] = CONSTANT_ATTENUATION_COEFF;
    reservedWords["cylinder"] = CYLINDER;
    reservedWords["diffuse"] = DIFFUSE;
    reservedWords["direction"] = DIRECTION;
    reservedWords["directional_light"] = DIRECTIONAL_LIGHT;
    reservedWords["emissive"] = EMISSIVE;
    reservedWords["faces"] = FACES;
    reservedWords["false"] = SYMFALSE;
    reservedWords["fov"] = FOV;
    reservedWords["gennormals"] = GENNORMALS;
    reservedWords["height"] = HEIGHT;
    reservedWords["index"] = INDEX;
    reservedWords["linear_attenuation_coeff"] = LINEAR_ATTENUATION_COEFF;
    reservedWords["material"] = MATERIAL;
    reservedWords["materials"] = MATERIALS;
    reservedWords["map"] = MAP;
    reservedWords["name"] = NAME;
    reservedWords["normals"] = NORMALS;
    reservedWords["point_light"] = POINT_LIGHT;
    reservedWords["points"] = POLYPOINTS;
    reservedWords["polymesh"] = TRIMESH;
    reservedWords["position"] = POSITION;
    reservedWords["quadratic_attenuation_coeff"] = QUADRATIC_ATTENUATION_COEFF;
    reservedWords["quaternian"] = QUATERNIAN;
    reservedWords["reflective"] = REFLECTIVE;
    reservedWords["rotate"] = ROTATE;
    reservedWords["SBT-raytracer"] = SBT_RAYTRACER;
    reservedWords["scale"] = SCALE;
    reservedWords["shininess"] = SHININESS;
    reservedWords["specular"] = SPECULAR;
    reservedWords["sphere"] = SPHERE;
    reservedWords["square"] = SQUARE;
    reservedWords["top_radius"] = TOP_RADIUS;
    reservedWords["transform"] = TRANSFORM;
    reservedWords["translate"] = TRANSLATE;
    reservedWords["transmissive"] = TRANSMISSIVE;
    reservedWords["trimesh"] = TRIMESH;
    reservedWords["true"] = SYMTRUE;
    reservedWords["updir"] = UPDIR;
    reservedWords["viewdir"] = VIEWDIR;
	reservedWords["look_at"] = LOOK_AT;

  }

  // search ReservedWords table
  std::map<string, SYMBOL>::const_iterator itr = 
    reservedWords.find( ident );
  if( itr == reservedWords.end() )
    return UNKNOWN;
  else
    return (*itr).second;
}

string Token::toString() const
{
  return getNameForToken( kind() );
}

void Token::Print( ostream& out ) const {
  out << toString();
}

void Token::Print( ) const {
  Print( std::cout );
}

string IdentToken::toString( ) const {
  ostringstream oss( Token::toString() );
  oss << ": \"" << _ident << "\"";
  return oss.str();
}

string ScalarToken::toString( ) const {
  ostringstream oss( Token::toString() );
  oss << ": " << _value;
  return oss.str();
}
 


/*
   The tokenizer's job is to convert a stream of characters
   into a stream of tokens that the parser can then handle.
   You probably aren't going to be modifying this file;
   most of the stuff that you might want to change is in
   Token.{h,cpp} and Parser.{h,cpp}.

   This tokenizer is based on the tokenizer from the
   PL0 project used for CSE401
   (http://www.cs.washington.edu/401).

*/


//////////////////////////////////////////////////////////////////////////
//
// Tokenizer::Tokenizer(istream&) constructor
//
//   This constructor sets up the initial state that we need in order
// to start scanning.  Note that we will be passed an OPEN file.  This
// simplifies the scanner part, since we don't have to open it and
// error check to see if it exists.  We assume that the caller (which
// will be the main() function) sets up everything and passes us a VALID
// file pointer.
//

Tokenizer::Tokenizer(istream& fp, bool printTokens) 
  : buffer( fp, false, false )
{ 
    TokenColumn = 0;
    CurrentCh = ' ';
    UnGetToken = nullptr;
    _printTokens = printTokens;
}

//////////////////////////////////////////////////////////////////////////
//
// repeatedly scan tokens in and throw them away.  Useful if this is the
// last phase to be executed
// 
void Tokenizer::ScanProgram() {
    while (Get()->kind() != EOFSYM) ;
}


auto_ptr<Token> Tokenizer::Get() {
  return auto_ptr<Token>(GetNext());
}

//////////////////////////////////////////////////////////////////////////
//
// Token* Tokenizer::Get() method
//
// Advance through the source to find the next token. Returns peeked token,
// if there is one.
//

Token* Tokenizer::GetNext() {
  Token* T = nullptr;

  // First check to see if there is an UnGetToken. If there is, use it.
  if (UnGetToken != nullptr) {
    T = UnGetToken;
    UnGetToken = nullptr;
    return T;
  }

  // Otherwise, crank up the scanner and get a new token.

  // Get rid of any whitespace
  SkipWhiteSpace();

  // test for end of file
  if (buffer.isEOF()) {
    T = new Token(EOFSYM);

  } else {
    
    // Save the starting position of the symbol in a variable,
    // so that nicer error messages can be produced.
    TokenColumn = buffer.CurColumn();
    
    // Check kind of current character
    
    // Note that _'s are now allowed in identifiers.
    if (isalpha(CurrentCh) || '_' == CurrentCh) {
      // grab identifier or reserved word
      T = GetIdent();
    } else if ( '"' == CurrentCh)  {
      T = GetQuotedIdent(); 
    } else if (isdigit(CurrentCh) || '-' == CurrentCh || '.' == CurrentCh) {
      T = GetScalar();
    } else { 
      //
      // Check for other tokens
      //
      
      T = GetPunct();
    }
  }
  
  if (T == nullptr) {
    throw ParserFatalException("didn't get a token");
  }

  if (_printTokens) {
    std::cout << "Token read: ";
    T->Print();
    std::cout << std::endl;
  }

  return T;
}

//////////////////////////////////////////////////////////////////////////
//
// Skips spaces, tabs, newlines, and comments
//
void Tokenizer::SkipWhiteSpace() {
  while (isspace(CurrentCh) && CurrentCh ) {
    GetCh();
  }

  if( '/' == CurrentCh )  // Look for comments
  {
    GetCh();
    if( '/' == CurrentCh )
    {
      // Throw out everything until the end of the line
      while( '\n' != CurrentCh )
      {
        GetCh();
      }
    }
    else if ( '*' == CurrentCh )
    {
      int startLine = CurLine();
      while( true )
      {
        GetCh();
        if( '*' == CurrentCh )
        {
          GetCh();
          if( CondReadCh( '/' ) )
            break;
          else if ( buffer.isEOF() )
          {
            std::ostringstream ost;
            ost << "Unterminated comment in line ";
            ost << startLine;
            throw SyntaxErrorException( ost.str(), *this );
          }
        }
        else if ( buffer.isEOF() )
        {
          std::ostringstream ost;
          ost << "Unterminated comment in line ";
          ost << startLine;
          throw SyntaxErrorException( ost.str(), *this );
        }
      }
    }
    else
    {
      std::ostringstream ost;
      ost << "unexpected character: '" << CurrentCh << "'";
	  throw SyntaxErrorException( ost.str(), *this );
    }

    SkipWhiteSpace();  // We may need to throw out
                       //  more white space/comments
                       // This is admittedly tail recursion...
  }
}

Token* Tokenizer::GetQuotedIdent() {
  GetCh();   // Throw out beginning '"'

  std::ostringstream ident;
  while ( '"' != CurrentCh ) {
    if( '\n' == CurrentCh )
      throw SyntaxErrorException( "Unterminated string constant", *this );

    ident << CurrentCh;
    GetCh();
  }
  GetCh();
  return new IdentToken( ident.str() );
}

//////////////////////////////////////////////////////////////////////////
//
// Token* Tokenizer::GetIdent method
//
//   GetIdent scans an identifier-like token.  It returns an
//   identifier or a reserved word token.
//

Token* Tokenizer::GetIdent() {
  // an IDENTIFIER or a RESERVED WORD token
  std::ostringstream ident;
  while (isalnum(CurrentCh) || '_' == CurrentCh || '-' == CurrentCh) { 
    // While we still have something that can
    ident << CurrentCh;
    GetCh();
  }
  return SearchReserved(ident.str());
}

//////////////////////////////////////////////////////////////////////////
//
// Token* Tokenizer::GetInt method
//
//   GetInt scans an integer.  It returns an integer token.
//

Token* Tokenizer::GetScalar() {
  // an INTEGER token
  string ret( "" );
  while (isdigit(CurrentCh) || '-' == CurrentCh || '.' == CurrentCh || 'e' == CurrentCh ) {
    ret += CurrentCh;
    GetCh();
  }
  return new ScalarToken( atof( ret.c_str() ) );
}

//////////////////////////////////////////////////////////////////////////
//
// Token* Tokenizer::GetPunct() method
//
//   Gets a punctuation token from input stream and returns it.
//

Token* Tokenizer::GetPunct() {
  Token* T;

  switch (CurrentCh) {
  case '(':  GetCh(); T = new Token(LPAREN);     break;
  case ')':  GetCh(); T = new Token(RPAREN);     break;
  case '{':  GetCh(); T = new Token(LBRACE);     break;
  case '}':  GetCh(); T = new Token(RBRACE);     break;
  case ',':  GetCh(); T = new Token(COMMA);      break;
  case '=':  GetCh(); T = new Token(EQUALS);     break;
  case ';':  GetCh(); T = new Token(SEMICOLON);  break;

  default:
    std::ostringstream ost;
    ost << "unexpected character: '" << CurrentCh << "'";
    throw SyntaxErrorException(ost.str(), *this);
  }

  return T;
}

//////////////////////////////////////////////////////////////////////////
//
// void Tokenizer::UnGet(Token*) method
//
//   UnGet returns the last read token to the input, where it will be
//   returned for the next Get call.  At most 1 token can be pushed back
//   at a time this way and this token is pointed to by the pointer
//   TokenToUnGet.  TokenToUnGet must be non null.

void Tokenizer::UnGet(Token* TokenToUnGet) {
  if (UnGetToken != nullptr) {
    throw ParserFatalException("trying to UnGet more than one token");
  }
  UnGetToken = TokenToUnGet;
}

//////////////////////////////////////////////////////////////////////////
//
// Token* Tokenizer::Peek() method
//
//   Peek reads the next token and pushes it back on the token stream
//

const Token* Tokenizer::Peek() {
  Token* T = GetNext();
  UnGet(T);
  return T;
}

//////////////////////////////////////////////////////////////////////////
//
// Token* Tokenizer::Read(SYMBOL) method
//
//   Read gets the next token and checks that it's of the expected type.
//

auto_ptr<Token> Tokenizer::Read(SYMBOL kind) {
  auto_ptr<Token> T( Get() );
  if (T->kind() != kind) {
    string msg( getNameForToken( kind ) );
    msg.append( " expected, " );
	msg.append(getNameForToken( T->kind() ));
	msg.append(" found instead!");
    throw SyntaxErrorException(msg, *this);
  }
  return T;
}

//////////////////////////////////////////////////////////////////////////
//
// bool Tokenizer::CondRead(SYMBOL) method
//
//   CondRead gets the next token and checks that it's of the expected type.
//   If it is, then consume it and return true.  Otherwise, push it back
//   and return false.
//

bool Tokenizer::CondRead(SYMBOL kind) {
  const Token* T = Peek();
  if (T->kind() == kind) {
    Get( );
    return true;
  } else {
    return false;
  }
}

//////////////////////////////////////////////////////////////////////////
//
// Token* Tokenizer::SearchReserved(const string&) private method
//
//   SearchReserved() maps a character string to an IdentToken or one of
// several possible reserved word tokens, using a binary search on the
// ReservedWords structure.
//

//
// The list of all the Reserved words and their associated symbols,
// in alphabetical order so this can be searched using binary search.
//

typedef std::map<string, SYMBOL> ReservedWordsMap;

Token* Tokenizer::SearchReserved(const string& ident) const {
  SYMBOL tokSymbol = lookupReservedWord( ident );
  if( UNKNOWN == tokSymbol )
  {
    return new IdentToken( ident );
  }
  else
  {
    return new Token( tokSymbol );
  }
}

//////////////////////////////////////////////////////////////////////////
//
// bool Tokenizer::CondReadCh(char) private method
//
//   CondReadCh checks to see if the CurrentCh is the same as the argument.
// If so, CondReadCh eats that character (by calling GetCh())
// and returns true.  If not, just return false.
//

bool Tokenizer::CondReadCh(char c) {
  if (c == CurrentCh) {
    GetCh();
    return true;
  } else {
    return false;
  }
}

/*
  The Buffer class is essentially a wrapper for an istream.
  It is here mainly to keep track of the current file location
  (line number, column number) to print intelligent error messages.


  If you find yourself changing stuff in this file, you're probably
  doing something wrong.
*/

//////////////////////////////////////////////////////////////////////////
//
// Buffer::Buffer(FILE*,...) constructor
//
//   This constructor sets up the initial state that we need in order
// to read files.
//

Buffer::Buffer(istream& is, bool printChars, bool printLines)
  : inStream( is )
{ 
    PositionInCurrentLine = Line.begin();
    LineNumber            = 0;
    ColNumber             = 0;
    LastPrintedLine       = 0;
    
    _printChars = printChars;
    _printLines = printLines;
	_bHasData = false;
}


//////////////////////////////////////////////////////////////////////////
//
// char Buffer::GetCh() private method
//
//   GetCh() gets a new character from the buffer and returns it
//

char Buffer::GetCh() {
  if (!inStream) {
    return '\0';
  }

  // test for data
  if ( !_bHasData )
  {
	   // Nothing has been read-in yet, so read the first line
	   GetLine();
	   if ( !_bHasData )
	   {
		   return '\0';
	   }
  }
  else
  {
	  // advance position
	  PositionInCurrentLine++;
	  ColNumber++;
  }

  while (PositionInCurrentLine == Line.end() || Line.empty()) {
    // need to read another line of input from the file

    GetLine();
    if (!inStream) {
      return '\0';
    }
  }

  // extract character from line buffer
  char CurrentCh = *PositionInCurrentLine;

  if (_printChars) {
    std::cout << "Read character `" << CurrentCh << "'" << std::endl;
  }

  return CurrentCh;
}


//////////////////////////////////////////////////////////////////////////
//
// void Buffer::GetLine() private method
//
//   GetLine() reads a new line from the file into the Line buffer
// variable.  It also handles listings, if necessary.  Returns are false
// for EOF or true for got a new buffer.
//
// This function is weak, in that it can't handle arbitrarily long lines
// in a file.  Ideally it would be rewritten so that it can do this fine.
// The real difficulty is in being able to print out, on demand, the whole
// line of input, of arbitrary length.
//

void Buffer::GetLine() {
  std::getline(inStream, Line);
  Line.append( "\n" ); // because iostreams strip out the end-of-line char
  PositionInCurrentLine = Line.begin();

  _bHasData = ( PositionInCurrentLine != Line.end() && !Line.empty() );

  ColNumber = 0;
  LineNumber ++;

  if (_printLines) PrintLine( std::cout );
}


//////////////////////////////////////////////////////////////////////////
//
// void Buffer::PrintLine() method
//
//   This method displays the current line on the screen.
//

void Buffer::PrintLine( ostream& out ) const {
  if (LineNumber > LastPrintedLine) {
    out << "# " << Line << std::endl;
    LastPrintedLine = LineNumber;
  }
}


template< typename T >
auto_ptr<T> wrap_auto_ptr( T* pointer )
{
	return auto_ptr<T>( pointer );
}

Scene* Parser::parseScene()
{
  _tokenizer.Read(SBT_RAYTRACER);

  auto_ptr<Token> versionNumber( _tokenizer.Read(SCALAR) );

  if( versionNumber->value() > 1.1 )
  {
    ostringstream ost;
    ost << "SBT-raytracer version number " << versionNumber->value() << 
      " too high; only able to parser v1.1 and below.";
    throw ParserException( ost.str() );
  }

  Scene* scene = new Scene;
  auto_ptr<Material> mat( new Material );

  for( ;; )
  {
    const Token* t = _tokenizer.Peek();

    switch( t->kind() )
    {
      case SPHERE:
      case BOX:
      case SQUARE:
      case CYLINDER:
      case CONE:
      case TRIMESH:
      case TRANSLATE:
      case ROTATE:
      case SCALE:
      case TRANSFORM:
      case LBRACE:
         parseTransformableElement(scene, nullptr, *mat);
      break;
      case POINT_LIGHT:
         scene->add( parsePointLight( scene ) );
         break;
      case DIRECTIONAL_LIGHT:
         scene->add( parseDirectionalLight( scene ) );
         break;
      case AMBIENT_LIGHT:
         parseAmbientLight( scene );
         break;
      case CAMERA:
         parseCamera( scene );
         break;
      case MATERIAL:
		 {
           auto_ptr<Material> temp( parseMaterialExpression( scene, *mat ));
		   mat = temp;
		 }
         break;
      case SEMICOLON:
         _tokenizer.Read( SEMICOLON );
         break;
      case EOFSYM:
         return scene;
      default:
         throw SyntaxErrorException( "Expected: geometry, camera, or light information", _tokenizer );
    }
  }
}

void Parser::parseCamera( Scene* scene )
{
	bool hasViewDir( false ), hasUpDir( false );
	bool hasLookAt( false ), hasPosition( false );
	Apollo::Vector3 viewDir, upDir, pos;

	_tokenizer.Read( CAMERA );
	_tokenizer.Read( LBRACE );

	for( ;; )
	{
		const Token* t = _tokenizer.Peek();

		switch( t->kind() )
		{
		case POSITION:
			pos = parseVec3dExpression();
            scene->getCamera()->SetPosition( pos );
			hasPosition = true;
			break;

		case FOV:
			scene->getCamera()->SetFov( (FLOAT)parseScalarExpression() );
			break;

		case QUATERNIAN:
            Apollo::ApolloException::NotYetImplemented();
            /*
			quaternian = parseVec4dExpression();
			scene->getCamera().setLook(
				quaternian[0], quaternian[1], quaternian[2], quaternian[3] );
            */
			break;

		case ASPECTRATIO:
			scene->getCamera()->SetAspectRatio( (FLOAT)parseScalarExpression() );
			break;

		case VIEWDIR:
			viewDir = parseVec3dExpression();
			hasViewDir = true;
			break;

		case LOOK_AT:
			viewDir = parseVec3dExpression();
			hasLookAt = true;
			break;

		case UPDIR:
			upDir = parseVec3dExpression();
			hasUpDir = true;
			break;

		case RBRACE:
			// semantic checks
			if( hasLookAt ) {
				if( !hasPosition )
                {
                    scene->getCamera()->SetPosition(Apollo::Vector3(1, 0, 0));
                    scene->getCamera()->Orient(viewDir);
                }
				else
				{
                    scene->getCamera()->SetPosition(pos);
					if( hasUpDir )
                        scene->getCamera()->Orient(viewDir, upDir);
                    else
                        scene->getCamera()->Orient(viewDir);
				}
			}
			else if( hasViewDir )
			{
				if( !hasUpDir )
					throw SyntaxErrorException( "Expected: 'updir'", _tokenizer );
                scene->getCamera()->Orient(viewDir, upDir);
			}
			else if( hasUpDir ) {
				if( !hasViewDir && !hasLookAt )
					throw SyntaxErrorException( "Expected: 'viewdir'", _tokenizer );
			}

			_tokenizer.Read( RBRACE );
			return;

		default:
			throw SyntaxErrorException( "Expected: camera attribute", _tokenizer );
		}
	}
}

void Parser::parseTransformableElement( Scene* scene, TransformNode* transform, const Material& mat )
{
    const Token* t = _tokenizer.Peek();
    switch( t->kind() )
    {
      case SPHERE:
      case BOX:
      case SQUARE:
      case CYLINDER:
      case CONE:
      case TRIMESH:
      case TRANSLATE:
      case ROTATE:
      case SCALE:
      case TRANSFORM:
         parseGeometry(scene, transform, mat);
      break;
      case LBRACE:
         parseGroup(scene, transform, mat);
         break;
      default:
         throw SyntaxErrorException( "Expected: transformable element", _tokenizer );
    }
}

// parse a group of geometry, i.e., enclosed in {} blocks.
void Parser::parseGroup(Scene* scene, TransformNode* transform, const Material& mat )
{
  auto_ptr<Material> newMat;
  _tokenizer.Read( LBRACE );
  for( ;; )
  {
    const Token* t = _tokenizer.Peek();
    switch( t->kind() )
    {
      case SPHERE:
      case BOX:
      case SQUARE:
      case CYLINDER:
      case CONE:
      case TRIMESH:
      case TRANSLATE:
      case ROTATE:
      case SCALE:
      case TRANSFORM:
      case LBRACE:
        parseTransformableElement( scene, transform, newMat.get() ? *newMat : mat );
        break;
      case RBRACE:
        _tokenizer.Read( RBRACE );
        return;
      case MATERIAL:
		{
          auto_ptr<Material> temp(parseMaterialExpression(scene, mat));
          newMat = temp;
		}
      default:
        throw SyntaxErrorException( "Expected: '}' or geometry", _tokenizer );
    }
  }
}


void Parser::parseGeometry(Scene* scene, TransformNode* transform, const Material& mat)
{
  const Token* t = _tokenizer.Peek();
  switch( t->kind() )
  {
    case SPHERE:
      parseSphere(scene, transform, mat);
      return;
    case BOX:
      parseBox(scene, transform, mat);
      return;
    case SQUARE:
      parseSquare(scene, transform, mat);
      return;
    case CYLINDER:
      parseCylinder(scene, transform, mat);
      return;
    case CONE:
      parseCone(scene, transform, mat);
      return;
    case TRIMESH:
      parseTrimesh(scene, transform, mat);
      return;
    case TRANSLATE:
      parseTranslate(scene, transform, mat);
      return;
    case ROTATE:
      parseRotate(scene, transform, mat);
      return;
    case SCALE:
      parseScale(scene, transform, mat);
      return;
    case TRANSFORM:
      parseTransform(scene, transform, mat);
      return;
    default:
      throw ParserFatalException( "Unrecognized geometry type." );
  }
}


void Parser::parseTranslate(Scene* scene, TransformNode* transform, const Material& mat)
{
    Apollo::ApolloImageException::NotYetImplemented();
    /*
  _tokenizer.Read( TRANSLATE );
  _tokenizer.Read( LPAREN );
  double x = parseScalar();
  _tokenizer.Read( COMMA );
  double y = parseScalar();
  _tokenizer.Read( COMMA );
  double z = parseScalar();
  _tokenizer.Read( COMMA );

  // Parse child geometry
  parseTransformableElement( scene, 
    transform->createChild( Mat4d::createTranslation( x, y, z ) ), mat );

  _tokenizer.Read( RPAREN );
  _tokenizer.CondRead(SEMICOLON);

  return;
  */
}

void Parser::parseRotate(Scene* scene, TransformNode* transform, const Material& mat)
{
    Apollo::ApolloImageException::NotYetImplemented();
    /*
  _tokenizer.Read( ROTATE );
  _tokenizer.Read( LPAREN );
  double x = parseScalar();
  _tokenizer.Read( COMMA );
  double y = parseScalar();
  _tokenizer.Read( COMMA );
  double z = parseScalar();
  _tokenizer.Read( COMMA );
  double w = parseScalar();
  _tokenizer.Read( COMMA );

  // Parse child geometry
  parseTransformableElement( scene, 
    transform->createChild( Mat4d::createRotation( w, x, y, z ) ), mat );

  _tokenizer.Read( RPAREN );
  _tokenizer.CondRead(SEMICOLON);

  return;
  */
}


void Parser::parseScale(Scene* scene, TransformNode* transform, const Material& mat)
{
    Apollo::ApolloImageException::NotYetImplemented();
    /*
  _tokenizer.Read( SCALE );
  _tokenizer.Read( LPAREN );
  double x, y, z;

  x = parseScalar();
  _tokenizer.Read( COMMA );

  const Token* next = _tokenizer.Peek();
  if( SCALAR == next->kind() )
  {
     y = parseScalar();
     _tokenizer.Read( COMMA );
     z = parseScalar();
     _tokenizer.Read( COMMA );
  }
  else
  {
     y = x;
     z = x;
  }

  // Parse child geometry
  parseTransformableElement( scene, 
    transform->createChild( Mat4d::createScale( x, y, z ) ), mat );

  _tokenizer.Read( RPAREN );
  _tokenizer.CondRead(SEMICOLON);
  return;
  */
}


void Parser::parseTransform(Scene* scene, TransformNode* transform, const Material& mat)
{
    Apollo::ApolloImageException::NotYetImplemented();
    /*
  _tokenizer.Read( TRANSFORM );
  _tokenizer.Read( LPAREN );

  Vec4d row1 = parseVec4d();
  _tokenizer.Read( COMMA );
  Vec4d row2 = parseVec4d();
  _tokenizer.Read( COMMA );
  Vec4d row3 = parseVec4d();
  _tokenizer.Read( COMMA );
  Vec4d row4 = parseVec4d();
  _tokenizer.Read( COMMA );

  parseTransformableElement( scene, 
    transform->createChild( Mat4d(row1, row2, row3, row4) ), mat );

  _tokenizer.Read( RPAREN );
  _tokenizer.CondRead(SEMICOLON);

  return;
  */
}

void Parser::parseSphere(Scene* scene, TransformNode* transform, const Material& mat)
{
  Apollo::Sphere* sphere = 0;
  Material* newMat = 0;

  _tokenizer.Read( SPHERE );
  _tokenizer.Read( LBRACE );

  for( ;; )
  {
    const Token* t = _tokenizer.Peek();

    switch( t->kind() )
    {
      case MATERIAL:
        delete newMat;
        newMat = parseMaterialExpression( scene, mat );
        break;
      case NAME:
        parseIdentExpression();
        break;
      case RBRACE:
        _tokenizer.Read( RBRACE );
        sphere = new Sphere(scene, newMat ? newMat : new Material(mat));
        sphere->setTransform( transform );
        scene->add( sphere );
        return;
      default:
        throw SyntaxErrorException( "Expected: sphere attributes", _tokenizer );
        
    }
  }
}

void Parser::parseBox(Scene* scene, TransformNode* transform, const Material& mat)
{
  Box* box = 0;

  _tokenizer.Read( BOX );
  _tokenizer.Read( LBRACE );

  Material* newMat = 0;
  for( ;; )
  {
    const Token* t = _tokenizer.Peek();

    switch( t->kind() )
    {
      case MATERIAL:
        delete newMat;
        newMat = parseMaterialExpression( scene, mat );
        break;
      case NAME:
        parseIdentExpression();
        break;
      case RBRACE:
         _tokenizer.Read( RBRACE );
        box = new Box(scene, newMat ? newMat : new Material(mat) );
        box->setTransform( transform );
        scene->add( box );
        return;
      default:
        throw SyntaxErrorException( "Expected: box attributes", _tokenizer );
        
    }
  }
}

void Parser::parseSquare(Scene* scene, TransformNode* transform, const Material& mat)
{
  Square* square = 0;
  Material* newMat = 0;

  _tokenizer.Read( SQUARE );
  _tokenizer.Read( LBRACE );

  for( ;; )
  {
    const Token* t = _tokenizer.Peek();

    switch( t->kind() )
    {
      case MATERIAL:
        delete newMat;
        newMat = parseMaterialExpression( scene, mat );
        break;
      case NAME:
        parseIdentExpression();
        break;
      case RBRACE:
         _tokenizer.Read( RBRACE );
        square = new Square(scene, newMat ? newMat : new Material(mat));
        square->setTransform( transform );
        scene->add( square );
        return;
      default:
        throw SyntaxErrorException( "Expected: square attributes", _tokenizer );
        
    }
  }
}

void Parser::parseCylinder(Scene* scene, TransformNode* transform, const Material& mat)
{
  Cylinder* cylinder = 0;
  Material* newMat = 0;

  _tokenizer.Read( CYLINDER );
  _tokenizer.Read( LBRACE );

  for( ;; )
  {
    const Token* t = _tokenizer.Peek();

    switch( t->kind() )
    {
      case MATERIAL:
        delete newMat;
        newMat = parseMaterialExpression( scene, mat );
        break;
      case NAME:
        parseIdentExpression();
        break;
      case RBRACE:
         _tokenizer.Read( RBRACE );
        cylinder = new Cylinder(scene, newMat ? newMat : new Material(mat));
        cylinder->setTransform( transform );
        scene->add( cylinder );
        return;
      default:
        throw SyntaxErrorException( "Expected: cylinder attributes", _tokenizer );
    }
  }

}

void Parser::parseCone(Scene* scene, TransformNode* transform, const Material& mat)
{
  _tokenizer.Read( CONE );
  _tokenizer.Read( LBRACE );

  Cone* cone;
  Material* newMat = 0;

  double bottomRadius = 1.0;
  double topRadius = 0.0;
  double height = 1.0;
  bool capped = true;				// Capped by default

  for( ;; )
  {
    const Token* t = _tokenizer.Peek();

    switch( t->kind() )
    {
      case MATERIAL:
        delete newMat;
        newMat = parseMaterialExpression( scene, mat );
        break;
      case NAME:
         parseIdentExpression();
         break;
      case CAPPED:
        capped = parseBooleanExpression();
        break;
      case BOTTOM_RADIUS:
        bottomRadius = parseScalarExpression();
        break;
      case TOP_RADIUS:
        topRadius = parseScalarExpression();
        break;
      case HEIGHT:
        height = parseScalarExpression();
        break;
      case RBRACE:
        _tokenizer.Read( RBRACE );
        cone = new Cone( scene, newMat ? newMat : new Material(mat), 
          height, bottomRadius, topRadius, capped );
        cone->setTransform( transform );
        scene->add( cone );
        return;
      default:
        throw SyntaxErrorException( "Expected: cone attributes", _tokenizer );
    }
  }
}

void Parser::parseTrimesh(Scene* scene, TransformNode* transform, const Material& mat)
{
  Trimesh* tmesh = new Trimesh( scene, new Material(mat), transform);

  _tokenizer.Read( TRIMESH );
  _tokenizer.Read( LBRACE );

  bool generateNormals( false );
  list<Vec3d> faces;

  char* error;
  for( ;; )
  {
    const Token* t = _tokenizer.Peek();

    switch( t->kind() )
    {
      case GENNORMALS:
        _tokenizer.Read( GENNORMALS );
        _tokenizer.Read( SEMICOLON );
        generateNormals = true;
        break;

      case MATERIAL:
        tmesh->setMaterial( parseMaterialExpression( scene, mat ) );
        break;

      case NAME:
         parseIdentExpression();
         break;

      case MATERIALS:
        _tokenizer.Read( MATERIALS );
        _tokenizer.Read( EQUALS );
        _tokenizer.Read( LPAREN );
        if( RPAREN != _tokenizer.Peek()->kind() )
        {
          tmesh->addMaterial( parseMaterial( scene, tmesh->getMaterial() ) );
          for( ;; )
          {
             const Token* nextToken = _tokenizer.Peek();
             if( RPAREN == nextToken->kind() )
               break;
             _tokenizer.Read( COMMA );
             tmesh->addMaterial( parseMaterial( scene, tmesh->getMaterial() ) );
          }
        }
        _tokenizer.Read( RPAREN );
        _tokenizer.Read( SEMICOLON );
        break;

      case NORMALS:
        _tokenizer.Read( NORMALS );
        _tokenizer.Read( EQUALS );
        _tokenizer.Read( LPAREN );
        if( RPAREN != _tokenizer.Peek()->kind() )
        {
          tmesh->addNormal( parseVec3d() );
          for( ;; )
          {
             const Token* nextToken = _tokenizer.Peek();
             if( RPAREN == nextToken->kind() )
               break;
             _tokenizer.Read( COMMA );
             tmesh->addNormal( parseVec3d() );
          }
        }
        _tokenizer.Read( RPAREN );
        _tokenizer.Read( SEMICOLON );
        break;

      case FACES:
        _tokenizer.Read( FACES );
        _tokenizer.Read( EQUALS );
        _tokenizer.Read( LPAREN );
        if( RPAREN != _tokenizer.Peek()->kind() )
        {
          parseFaces( faces );
          for( ;; )
          {
             const Token* nextToken = _tokenizer.Peek();
             if( RPAREN == nextToken->kind() )
               break;
             _tokenizer.Read( COMMA );
             parseFaces( faces );
          }
        }
        _tokenizer.Read( RPAREN );
        _tokenizer.Read( SEMICOLON );
        break;

      case POLYPOINTS:
        _tokenizer.Read( POLYPOINTS );
        _tokenizer.Read( EQUALS );
        _tokenizer.Read( LPAREN );
        if( RPAREN != _tokenizer.Peek()->kind() )
        {
          tmesh->addVertex( parseVec3d() );
          for( ;; )
          {
             const Token* nextToken = _tokenizer.Peek();
             if( RPAREN == nextToken->kind() )
               break;
             _tokenizer.Read( COMMA );
             tmesh->addVertex( parseVec3d() );
          }
        }
        _tokenizer.Read( RPAREN );
        _tokenizer.Read( SEMICOLON );
        break;


      case RBRACE:
      {
        _tokenizer.Read( RBRACE );

        // Now add all the faces into the trimesh, since hopefully
        // the vertices have been parsed out
        for( list<Vec3d>::const_iterator vitr = faces.begin(); vitr != faces.end(); vitr++ )
        {
          if( !tmesh->addFace((int) (*vitr)[0], (int) (*vitr)[1], (int) (*vitr)[2] ) )
          {
            ostringstream oss;
            oss << "Bad face in trimesh: (" << (*vitr)[0] << ", " << (*vitr)[1] << 
              ", " << (*vitr)[2] << ")";
            throw ParserException( oss.str() );
          }
        }

        if( generateNormals )
          tmesh->generateNormals();

        if( error = tmesh->doubleCheck() )
          throw ParserException( error );

        scene->add( tmesh );
        return;
      }

      default:
        throw SyntaxErrorException( "Expected: trimesh attributes", _tokenizer );
    }
  }
}

void Parser::parseFaces( list< Vec3d >& faces )
{
  list< double > points = parseScalarList();

  // triangulate here and now.  assume the poly is
  // concave and we can triangulate using an arbitrary fan
  if( points.size() < 3 )
     throw SyntaxErrorException( "Faces must have at least 3 vertices.", _tokenizer );

  list<double>::const_iterator i = points.begin();
  double a = (*i++);
  double b = (*i++);
  while( i != points.end() )
  {
    double c = (*i++);
    faces.push_back( Vec3d( a, b, c ) );
    b = c;
  }
}

// Ambient lights are a bit special in that we don't actually
// create a separate Light for each ambient light; instead
// we simply sum all the ambient intensities and put them in
// the scene as the I_a coefficient.
void Parser::parseAmbientLight( Scene* scene )
{
  _tokenizer.Read( AMBIENT_LIGHT );
  _tokenizer.Read( LBRACE );
  if( _tokenizer.Peek()->kind() != COLOR )
    throw SyntaxErrorException( "Expected color attribute", _tokenizer );

  scene->addAmbient( parseVec3dExpression() );
  _tokenizer.Read( RBRACE );
  return;
}

PointLight* Parser::parsePointLight( Scene* scene )
{
  Vec3d position;
  Vec3d color;

  // Default to the 'default' system
  float constantAttenuationCoefficient = 0.0f;
  float linearAttenuationCoefficient = 0.0f;
  float quadraticAttenuationCoefficient = 1.0f;

  bool hasPosition( false ), hasColor( false );
  
  _tokenizer.Read( POINT_LIGHT );
  _tokenizer.Read( LBRACE );

  for( ;; )
  {
     const Token* t = _tokenizer.Peek();
     switch( t->kind() )
     {
       case POSITION:
         if( hasPosition )
           throw SyntaxErrorException( "Repeated 'position' attribute", _tokenizer );
         position = parseVec3dExpression();
         hasPosition = true;
         break;

       case COLOR:
         if( hasColor )
            throw SyntaxErrorException( "Repeated 'color' attribute", _tokenizer );
         color = parseVec3dExpression();
         hasColor = true;
         break;

       case CONSTANT_ATTENUATION_COEFF:
         constantAttenuationCoefficient = parseScalarExpression();
		 break;

       case LINEAR_ATTENUATION_COEFF:
         linearAttenuationCoefficient = parseScalarExpression();
		 break;
         
       case QUADRATIC_ATTENUATION_COEFF:
         quadraticAttenuationCoefficient = parseScalarExpression();
		 break;

       case RBRACE:
         if( !hasColor )
           throw SyntaxErrorException( "Expected: 'color'", _tokenizer );
         if( !hasPosition )
           throw SyntaxErrorException( "Expected: 'position'", _tokenizer );
         _tokenizer.Read( RBRACE );
         return new PointLight( scene, position, color, constantAttenuationCoefficient, 
           linearAttenuationCoefficient, quadraticAttenuationCoefficient );

        default:
          throw SyntaxErrorException( 
			  "expecting 'position' or 'color' attribute, or 'constant_attenuation_coeff', 'linear_attenuation_coeff', or 'quadratic_attenuation_coeff'", 
            _tokenizer );
     }
  }
}

DirectionalLight* Parser::parseDirectionalLight( Scene* scene )
{
  Vec3d direction;
  Vec3d color;

  bool hasDirection( false ), hasColor( false );

  _tokenizer.Read( DIRECTIONAL_LIGHT );
  _tokenizer.Read( LBRACE );

  for( ;; )
  {
     const Token* t = _tokenizer.Peek();
     switch( t->kind() )
     {
       case DIRECTION:
         if( hasDirection )
           throw SyntaxErrorException( "Repeated 'direction' attribute", _tokenizer );
         direction = parseVec3dExpression();
         hasDirection = true;
         break;

       case COLOR:
         if( hasColor )
            throw SyntaxErrorException( "Repeated 'color' attribute", _tokenizer );
         color = parseVec3dExpression();
         hasColor = true;
         break;

        case RBRACE:
          if( !hasColor )
            throw SyntaxErrorException( "Expected: 'color'", _tokenizer );
          if( !hasDirection )
            throw SyntaxErrorException( "Expected: 'position'", _tokenizer );
          _tokenizer.Read( RBRACE );
          return new DirectionalLight( scene, direction, color );

        default:
          throw SyntaxErrorException( "expecting 'position' or 'color' attribute", 
            _tokenizer );
     }
  }
}

// These ought to be done with template member functions, but compiler support for
// these is rather iffy...
double Parser::parseScalarExpression()
{
  // Throw out first token, which precedes the = sign
  _tokenizer.Get();
  _tokenizer.Read(EQUALS);
  double value( parseScalar() );
  _tokenizer.CondRead(SEMICOLON);
  return value;
}

bool Parser::parseBooleanExpression()
{
  _tokenizer.Get();
  _tokenizer.Read(EQUALS);
  bool value( parseBoolean() ); 
  _tokenizer.CondRead(SEMICOLON);
  return value;
}

Vec3d Parser::parseVec3dExpression()
{
  _tokenizer.Get();
  _tokenizer.Read(EQUALS);
  Vec3d value( parseVec3d() );
  _tokenizer.CondRead(SEMICOLON);
  return value;
}

Vec4d Parser::parseVec4dExpression()
{
  _tokenizer.Get();
  _tokenizer.Read(EQUALS);
  Vec4d value( parseVec4d() );
  _tokenizer.CondRead(SEMICOLON);
  return value;
}

Material* Parser::parseMaterialExpression( Scene* scene, const Material& parent )
{
  _tokenizer.Read(MATERIAL);
  _tokenizer.Read(EQUALS);
  Material* mat = parseMaterial( scene, parent );
  _tokenizer.CondRead( SEMICOLON );
  return mat;
}

string Parser::parseIdentExpression()
{
  _tokenizer.Get();
  _tokenizer.Read(EQUALS);
  string value( parseIdent() );
  _tokenizer.CondRead(SEMICOLON);
  return value;
}

double Parser::parseScalar()
{
  auto_ptr<Token> scalar( _tokenizer.Read( SCALAR ) );

  return scalar->value();
}

string Parser::parseIdent()
{
  auto_ptr<Token> scalar( _tokenizer.Read( IDENT ) );

  return scalar->ident();
}


list<double> Parser::parseScalarList()
{
  list<double> ret;

  _tokenizer.Read( LPAREN );
  if( RPAREN != _tokenizer.Peek()->kind() )
  {
    ret.push_back( parseScalar() );
    for( ;; )
    {
      const Token* nextToken = _tokenizer.Peek();
      if( RPAREN == nextToken->kind() )
        break;
      _tokenizer.Read( COMMA );
      ret.push_back( parseScalar() );
    }
  }
  _tokenizer.Read( RPAREN );

  return ret;

}

bool Parser::parseBoolean()
{
  const Token* next = _tokenizer.Peek();
  if( SYMTRUE == next->kind() )
  {
     _tokenizer.Read(SYMTRUE);
     return true;
  }
  if( SYMFALSE == next->kind() )
  {
     _tokenizer.Read(SYMFALSE);
     return false;
  }
  throw SyntaxErrorException( "Expected boolean", _tokenizer );
}

Vec3d Parser::parseVec3d()
{
  _tokenizer.Read( LPAREN );
  auto_ptr<Token> value1( _tokenizer.Read( SCALAR ) );
  _tokenizer.Read( COMMA );
  auto_ptr<Token> value2( _tokenizer.Read( SCALAR ) );
  _tokenizer.Read( COMMA );
  auto_ptr<Token> value3( _tokenizer.Read( SCALAR ) );
  _tokenizer.Read( RPAREN );

  return Vec3d( value1->value(), 
    value2->value(), 
    value3->value() );
}

Vec4d Parser::parseVec4d()
{
  _tokenizer.Read( LPAREN );
  auto_ptr<Token> value1( _tokenizer.Read( SCALAR ) );
  _tokenizer.Read( COMMA );
  auto_ptr<Token> value2( _tokenizer.Read( SCALAR ) );
  _tokenizer.Read( COMMA );
  auto_ptr<Token> value3( _tokenizer.Read( SCALAR ) );
  _tokenizer.Read( COMMA );
  auto_ptr<Token> value4( _tokenizer.Read( SCALAR ) );
  _tokenizer.Read( RPAREN );

  return Vec4d( value1->value(), 
    value2->value(), 
    value3->value(),
    value4->value() );
}

Material* Parser::parseMaterial( Scene* scene, const Material& parent )
{
  const Token* tok = _tokenizer.Peek();
  if( IDENT == tok->kind() )
  {
     return new Material(materials[ tok->ident() ]);
  }

  _tokenizer.Read( LBRACE );

  bool setReflective( false );
  string name;

  Material* mat = new Material(parent);

  for( ;; )
  {
    const Token* token = _tokenizer.Peek();
    switch( token->kind() )
    {
      case EMISSIVE:
        mat->setEmissive( parseVec3dMaterialParameter(scene) );
        break;

      case AMBIENT:
        mat->setAmbient( parseVec3dMaterialParameter(scene) );
        break;

      case SPECULAR:
      {
        MaterialParameter specular = parseVec3dMaterialParameter(scene);
        mat->setSpecular( specular );
        if( ! setReflective )
          mat->setReflective( specular );  // Default kr = ks if none specified
        break;
      }

      case DIFFUSE:
        mat->setDiffuse( parseVec3dMaterialParameter(scene) ); 
        break;

      case REFLECTIVE:
        mat->setReflective( parseVec3dMaterialParameter(scene) );
        setReflective = true;
        break;

      case TRANSMISSIVE:
        mat->setTransmissive( parseVec3dMaterialParameter(scene) );
        break;

      case INDEX:
        mat->setIndex( parseScalarMaterialParameter(scene) );
        break;

      case SHININESS:
        mat->setShininess( parseScalarMaterialParameter(scene) );
        break;

      case NAME:
         _tokenizer.Read(NAME);
         name = (_tokenizer.Read(IDENT))->ident();
         _tokenizer.Read( SEMICOLON );
         break;

      case RBRACE:
        _tokenizer.Read( RBRACE );
        if( ! name.empty() )
        {
           if( materials.find( name ) == materials.end() )
              materials[ name ] = *mat;
           else
           {
              ostringstream oss;
              oss << "Redefinition of material '" << name << "'.";
              throw SyntaxErrorException( oss.str(), _tokenizer );
           }
        }
        return mat;

      default:
         throw SyntaxErrorException( "Expected: material attribute", _tokenizer );

    }
  }
}

MaterialParameter Parser::parseVec3dMaterialParameter( Scene* scene )
{
  _tokenizer.Get();
  _tokenizer.Read(EQUALS);
  if( _tokenizer.CondRead( MAP ) )
  {
    _tokenizer.Read( LPAREN );
    string filename = _basePath;
    filename.append( "/" );
    filename.append(parseIdent());
    _tokenizer.Read( RPAREN );
    _tokenizer.CondRead(SEMICOLON);
    return MaterialParameter( scene->getTexture( filename ) );
  }
  else
  {
    Vec3d value( parseVec3d() );
    _tokenizer.CondRead(SEMICOLON);
    return MaterialParameter( value );
  }
}

MaterialParameter Parser::parseScalarMaterialParameter( Scene* scene )
{
  _tokenizer.Get();
  _tokenizer.Read(EQUALS);
  if( _tokenizer.CondRead(MAP) )
  {
    _tokenizer.Read( LPAREN );
    string filename = parseIdent();
    _tokenizer.Read( RPAREN );
    _tokenizer.CondRead(SEMICOLON);
    return MaterialParameter( scene->getTexture( filename ) );
  }
  else
  {
    double value = parseScalar();
    _tokenizer.CondRead(SEMICOLON);
    return MaterialParameter( value );
  }
}
#endif
