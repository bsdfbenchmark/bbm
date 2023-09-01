#ifndef _BBM_STRING_UTIL_
#define _BBM_STRING_UTIL_

#include <string>

/************************************************************************/
/*! \file string_util.h

  \brief Helper method for processing strings

*************************************************************************/

namespace bbm {
  namespace string {
  
  /**********************************************************************/
  /*! \brief remove the white space at the front and back of a string
   **********************************************************************/
  std::string remove_whitespace(const std::string& str)
  {
    // trivial case: empty
    if(str.empty()) return str;

    // otherwise, search for first non-white space chars
    const char* whiteSpace = " \r\n\t\v";

    size_t s = str.find_first_not_of(whiteSpace);
    size_t e = str.find_last_not_of(whiteSpace);

    if(s > e) return std::string();
    else return str.substr(s, e-s+1);
  }
  
  /**********************************************************************/
  /*! \brief remove surrounding backets
   **********************************************************************/
  std::string remove_brackets(const std::string& str)
  {
    auto newStr = remove_whitespace(str);

    const std::string openbracket = "[{(";
    const std::string closebracket = "]})";

    bool matching_brackets = !newStr.empty() && (openbracket.find(newStr.front()) != std::string::npos) && (openbracket.find(newStr.front()) == closebracket.find(newStr.back()));
    if(!matching_brackets) throw std::runtime_error("Mismatch brackets in expression: " + str);

    return newStr.substr(1, newStr.size() - 2);
  }

  /**********************************************************************/
  /*! \brief Remove comments from string
   **********************************************************************/
  std::string remove_comment(const std::string& str, const std::string& comment_marker)
  {
    size_t pos = str.find(comment_marker);
    if(pos == 0 || str.empty()) return std::string();
    else if(pos != std::string::npos) return str.substr(0, pos);
    else return str;
  }
  
  /**********************************************************************/
  /*! \brief Return the keyword substring appearing an open bracket, and the
      arguments appearing in the brackets: e.g., keyword(arguments)
   **********************************************************************/
  std::pair<std::string, std::string> get_keyword(const std::string& str)
  {
    size_t b = str.find_first_of('(');
    if(b == std::string::npos) throw std::runtime_error("Expected open bracket in expression: " + str);

    return std::pair( b!=0 ? remove_whitespace( str.substr(0, b) ) : "", remove_whitespace( str.substr(b) ) );
  }
  
  /**********************************************************************/
  /*! \brief split a string of the form "key = val" in key and value. If no
      '=', then return an empty key.
  ***********************************************************************/
  std::pair<std::string, std::string> split_eq(const std::string& str)
  {
    // search for '='; if none found return str (without whitespace)
    size_t eqpos = str.find_first_of('=');
    if(eqpos == std::string::npos) return std::pair("", remove_whitespace(str));

    // else extract key and value
    std::string key = str.substr(0, eqpos);
    std::string value = str.substr(eqpos+1);

    // return without whitespace
    return std::pair( remove_whitespace(key), remove_whitespace(value) );
  }

  /**********************************************************************/
  /*! \brief Split a string based on comma's if not surrounded by brackets
    *********************************************************************/
  std::vector<std::string> split_args(const std::string& str)
  {
    const std::string openbracket = "[{(";
    const std::string closebracket = "]})";
    std::vector<size_t> bracket_stack;
    std::string current_word;
    std::vector<std::string> result;
    
    for(auto c : str)
    {
      // check if bracket is being opened
      auto opos = openbracket.find(c);
      if(opos != std::string::npos)
      {
        bracket_stack.push_back(opos);
      }

      // check if corresponding bracket is closed
      auto cpos = closebracket.find(c);
      if(cpos != std::string::npos)
      {
        if(bracket_stack.empty() || bracket_stack.back() != cpos) throw std::runtime_error("Mismatched brackets in expression: " + str);
        bracket_stack.pop_back();
      }
      
      // otherwise did we encounter a comma and there is no bracket open
      if(c == ',' && bracket_stack.empty())
      {
        result.push_back(remove_whitespace(current_word));
        current_word = "";
      }
      
      // else grow current word 
      else current_word += c;
    }
    
    // process last word
    if(!bracket_stack.empty()) throw std::runtime_error("Bracket not closed in expression:" + str);
    else if(current_word != "") result.push_back(remove_whitespace(current_word));
    
    // Done.
    return result;
  }

  } // end string namespace
} // end bbm namespace

#endif /* _BBM_STRING_UTIL_ */
