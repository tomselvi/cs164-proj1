 /* -*- mode: C++; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */

/* tokens.cc: Definitions related to AST_Token and its subclasses. */

/* Authors:  YOUR NAMES HERE */

#include <iostream>
#include "apyc.h"
#include "ast.h"
#include "apyc-parser.hh"

using namespace std;

/** Default print for tokens. */
void
AST_Token::print (ostream& out, int indent)
{
    out << "(<Token>)";
}

/** Default implementation. */
string
AST_Token::string_text () const
{
    throw logic_error ("unimplemented operation: string_text");
}

/** Default implementation. */
void
AST_Token::append_text(const string& s)
{
    throw logic_error ("unimplemented operation: append_text");
}

/** Represents an identifier. */
class ID_Token : public AST_Token {
private:

    void print (ostream& out, int indent) {
        out << "(id " << lineNumber () << " " << identifier << ")";
    }

    ID_Token* post_make() 
    {
        identifier = string(as_chars(), text_size());
        return this;
    }

    TOKEN_CONSTRUCTORS(ID_Token, AST_Token);

    string identifier;

};

TOKEN_FACTORY(ID_Token, ID);

/** Represents a type variable (ID). */
class Type_Token : public AST_Token {
private:
    
    void print (ostream& out, int indent) {
        out << "(type_var " << lineNumber () << " " << identifier << ")";
    }
    
    Type_Token* post_make()
    {
        identifier = string(as_chars(), text_size());
        return this;
    }

    TOKEN_CONSTRUCTORS(Type_Token, AST_Token);
    
    string identifier;
    
};

TOKEN_FACTORY(Type_Token, TYPE_VAR);

/** Represents a token for "ID::" where the id is printed. */
class IDType_Token : public AST_Token {
private:
    
    void print (ostream& out, int indent) {
        out << "(id " << lineNumber () << " " << identifier << ")";
    }
    
    IDType_Token* post_make()
    {
        identifier = string(as_chars(), text_size() - 2);
        return this;
    }
    
    TOKEN_CONSTRUCTORS(IDType_Token, AST_Token);
    
    string identifier;
    
};

TOKEN_FACTORY(IDType_Token, ID_TYPE_OP);


/** Represents a simple statement. */
class Simple_Token : public AST_Token {
private:

    void print (ostream& out, int indent) {
        out << "(" << token_name << " " << lineNumber () << ")";
    }

    Simple_Token* post_make() 
    {
        token_name = string(as_chars(), text_size());
        return this;
    }

    TOKEN_CONSTRUCTORS(Simple_Token, AST_Token);

    string token_name;

};

TOKEN_FACTORY(Simple_Token, SIMPLE_STMT);


/** Represents an integer literal. */
class Int_Token : public AST_Token {
private:

    void print (ostream& out, int indent) {
        out << "(int_literal " << lineNumber () << " " << value << ")";
    }

    /** Initialize value from the text of the lexeme, checking that
     *  the literal is in range.  [The post_make method may be
     *  overridden to provide additional processing during the
     *  construction of a node or token.] */
    Int_Token* post_make () {
        const char* s = as_chars() ;
        int base = 10;
        if (strlen(s) > 1 && (tolower(s[1]) == 'x')) {
            value = strtol(s, NULL, 16);
            base = 16;
        }
        else if (strlen(s) > 1 && s[0] == '0') {
            value = strtol(s, NULL, 8);
            base = 8;
        }
        else {
            value = atoi(s);
        }
        long bound = 1073741824;
        if ((value > bound) || (value < 0)) {
            switch (base) {
                case 10:
                    error(s, "All decimals must be in the range [0, 2^30].");
                    break;
                    
                case 16:
                    error(s, "All hexadecimals must be in the range [0, 2^30].");
                    break;
                    
                case 8:
                    error(s, "All octals must be in the range [0, 2^30].");
                    break;
                    
                default:
                    error(s, "All integer literals must be in the range [0, 2^30].");
                    break;
            }
        }
        return this;
    }

    long value;

    TOKEN_CONSTRUCTORS(Int_Token, AST_Token);

};

TOKEN_FACTORY(Int_Token, INT_LITERAL);

    
/** Represents a string. */
class String_Token : public AST_Token {
private:
    
    /** Set literal_text from the text of this lexeme, converting
     *  escape sequences as necessary. */
    String_Token* post_make () {
        if (syntax () == RAWSTRING) {
            literal_text = string (as_chars (), text_size ());
        } else {
            int v;
            const char* s = as_chars ();
            size_t i;
            i = 0;
            literal_text.clear ();
            while (i < text_size ()) {
                i += 1;
                if (s[i-1] == '\\') {
                    i += 1;
                    switch (s[i-1]) {
                    default: literal_text += '\\'; v = s[i-1]; break;
                    case '\n': continue;
                    case 'a': v = '\007'; break;
                    case 'b': v = '\b'; break;
                    case 'f': v = '\f'; break;
                    case 'n': v = '\n'; break;
                    case 'r': v = '\r'; break;
                    case 't': v = '\t'; break;
                    case 'v': v = '\v'; break;
                    case '\'': v = '\''; break;
                    case '"': case '\\': v = s[i-1]; break;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': 
                    { 
                        v = s[i-1] - '0';
                        for (int j = 0; j < 2; j += 1) {
                            if ('0' > s[i] || s[i] > '7')
                                break;
                            v = v*8 + (s[i] - '0');
                            i += 1;
                        }
                        break;
                    }
                    case 'x': {
                        if (i+2 > text_size () || 
                            !isxdigit (s[i]) || !isxdigit (s[i+1])) {
                            error (s, "bad hexadecimal escape sequence");
                            break;
                        }
                        sscanf (s+i, "%2x", &v);
                        i += 2;
                        break;
                    }
                    }
                } else
                    v = s[i-1];
                literal_text += (char) v;        
            }
        }
        return this;
    }

    void print (ostream& out, int indent) {
        out << "(string_literal " << lineNumber () << " \"";
        for (size_t i = 0; i < literal_text.size (); i += 1) {
            char c = literal_text[i];
            if (c < 32 || c == '\\' || c == '"') {
                out << "\\" << oct << setw (3) << setfill('0') << (int) c
                    << setfill (' ') << dec;
            } else
                out << c;
        }
        out << "\")";
    }

    string string_text () const {
        return literal_text;
    }

    void append_text(const string& s) {
        literal_text += s;
    }

    TOKEN_CONSTRUCTORS(String_Token, AST_Token);
    static const String_Token raw_factory;

    string literal_text;
};

TOKEN_FACTORY(String_Token, STRING);

/** A dummy token whose creation registers String_Token as the class
 *  to use for RAWSTRING tokens produced by the lexer.  (The
 *  TOKEN_FACTORY macro above registers String_Token as the class for
 *  non-raw the STRING tokens as well.)
 *  */ 
const String_Token String_Token::raw_factory (RAWSTRING);
