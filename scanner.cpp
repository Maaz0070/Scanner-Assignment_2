#include "scanner.hpp"
#include <fstream>


#define ALPHA 0
#define INTEGER 1
#define DEC 2
#define SPEC1 3
#define SPEC2 4 
#define QUOTE 5
#define SPACE 6
#define INVC 7


Scanner::Scanner(std::string fileName) {
/*
    input: takes in filename as a string which uses to open a file stream.
    output: a stream of tokens.
    Functionality: To read and check if the file can be opned, if not the code
                   exits with an error message.
*/
    file.open(fileName);
    if(!file){
        std::cerr <<"Error: file not found.";
        exit(1);
    } 

    special1char = {'+', '-', '*', '/', ';', '<', '>','(', ')', '[', ']', ',', ';', '"', '{', '}', '^', ':'}; // first
    special2char = {'=', '>'}; // second
    special3char = {">=", "<=", "<>", ".." , ":="}; //   
    invalidchar = {'?', '!'};


    lookupKeywords = {"program", "begin", "end", "var", "integer", "real", "procedure", "function", "if", "then", "else", "while", "do", "repeat", "until", "for", "to", "downto", "case", "of", "goto"},
    lookupOperators = {"+", "-", "*", "/", ":=", "=", "<>", "<", "<=", ">", ">=", "(", ")", "[", "]", ",", ":", ";", ".", "..", };
    line = 0;
    consumingString = false;
    isEOF = false;
    foundOp = false;
}

void getTokenType(Token& res, int state) {
  switch (state) {
      case 0:
        res.type = INVALID;
        break;
      case 1:
        res.type = WORD;
        break;
      case 2:
        res.type = INTEGERS;
        break;
      case 3:
        res.type = DECIMAL;
        break;
      case 4:
        res.type = SPECIAL1;
        break;
      case 5:
        res.type = SPECIAL2;
        break;
      case 6:
        res.type = QUOTES;
        break;
      default:
        res.type = INVALID;
        break;
    }
}

std::string toLower(std::string str) {
    std::string res = "";
    for (int i = 0; i < str.length(); i++) {
        res += tolower(str[i]);
    }
    return res;
}

void Scanner::look_up(Token& res) {
    res.isKeyword = false;
    if (res.type == WORD) {
      for(auto keyword : lookupKeywords) {
        if(keyword == toLower(res.value)){
            res.label = keyword;
            res.isKeyword = true;
            return;
        }
      }
      res.label = "WORD";
    } else if (res.type==SPECIAL1){
      for(auto op : lookupOperators) {
        if(op == res.value){
            res.label = op;
            res.isKeyword = true;
            return;
        } 
      }
    } 
    res.label = "STRING";
    return;

}

bool isWhitespace(char cur) {
  return cur == ' ' || cur == '\t' || cur == '\n' || cur == '\r' || cur == '\0';
}

Token Scanner::nextToken() {
    char cur = '1';
    int state = 0;
    std::string tmpString = "";
    Token res;
    res.value = "";
    bool flag = false;
    bool qflag = false;
    bool dflag = false;


    while (!file.eof() && cur != '\0') {
      cur = file.get();
      if (cur == EOF) {
        isEOF = true;
        getTokenType(res, state);
        look_up(res);
        return res;
      }
      getTokenType(res, state);

      if (state==0 && isWhitespace(cur)) {
        continue;
      }
      if (cur == '(' || cur == '[' || cur == '{') {
        opStack.push(cur);
        res.type = SPECIAL1;
        res.value = cur;
        return res;
      }
      if (cur == ')' || cur == ']' || cur == '}') {
        if (opStack.empty()) {
          res.type = INVALID;
          res.value = cur;
          return res;
        }
        char top = opStack.top();
        if ((top == '(' && cur == ')') || (top == '[' && cur == ']') || (top == '{' && cur == '}')) {
          opStack.pop();
          res.type = SPECIAL1;
          res.value = cur;
          return res;
        } else {
          res.type = INVALID;
          res.value = cur;
          return res;
        }
      }

      if (cur == '\''){
        qflag = !qflag;
      }
      
      if (special1char.find(cur) != special1char.end() && !qflag) {
        if (cur=='.')foundOp = !foundOp;
        if (foundOp) {
          state = 0;
          if (res.value != "") {
            getTokenType(res, state);
            return res;
          }
          continue;
        }
        

        char tmp = cur;
        std::string tmpString = "";
        cur = file.get();
        if (special1char.find(cur) != special1char.end() || special2char.find(cur) != special2char.end()) {
          tmpString.push_back(tmp);
          tmpString.push_back(cur);
          if (special3char.find(tmpString) != special3char.end()) {
            res.value = tmpString;
            getTokenType(res, 5);
            look_up(res);
            return res;
          } 
          
        } else {

        }
        file.putback(cur);
        cur = tmp;
        res.value = cur;
        getTokenType(res, 4);
        return res;
      }
      

      state = nextState(state, cur);
      if (state == 3 && cur == '.') {
        dflag = true;
        res.value += cur;
      }
      else if (state < 0 && dflag) {
        res.value += cur;
        errString = res.value;
      } 
      else if (state == 0) {
        if (errString.length() > 0) {
          std::cerr << "TOKEN ERROR at line " << line << ": " << " invalid string"  << errString << std::endl;
          errString = "";
          dflag = !dflag;
        }
        if (!isWhitespace(cur) && !(invalidchar.find(cur) != invalidchar.end())) res.value += cur;
        look_up(res);
        file.putback(cur);
        return res; 
      }
      else 
        if (!isWhitespace(cur)) res.value += cur;
    }
}

int Scanner::getTransition(char input) {
  if (isalpha(input) || input == '_' ) {
    return 0;
  } else if (isdigit(input)) {
    return 1;
  } else if (input == '.') {
    return 2;
  }
  else if (special1char.find(input) != special1char.end()) {
    return 3;
  } 
  else if (special2char.find(input) != special2char.end()) {
    return 4;
  }
  else if (input == '\'') {
    return 5;
  } 
  else if (isWhitespace(input)) {
    return 6;
  } 
  else if (invalidchar.find(input) != invalidchar.end()) {
    return 7;
  }
  else {
    return -1;
  }
}

int Scanner::nextState(int curState, char input) {
  int transition = getTransition(input);

  switch(curState) {
    // 
    case 0:
    // START
      if (transition == ALPHA) return 1;
      else if (transition == INTEGER) return 2;
      else if (transition == DEC) return 4;
      else if (transition == SPEC1) return 4;
      else if (transition == SPEC2) return 4;
      else if (transition == QUOTE) return 6;
      else if (transition == SPACE) return 0;
      else if (transition == INVC) return 7;
      else return -1;
      break;
    
    case 1:
      // WORD
      if (transition == ALPHA) return 1;
      else if (transition == INTEGER) return 1;
      else if (transition == DEC) return 0;
      else if (transition == SPEC1) return 0;
      else if (transition == SPEC2) return 0;
      else if (transition == QUOTE) return -1;
      else if (transition == SPACE) return 0;
      else if (transition == INVC) return 0;
      else return -1;
      break;
    case 2:
      // INTEGER
      if (transition == ALPHA) return -1;
      else if (transition == INTEGER) return 2;
      else if (transition == DEC) return 3;
      else if (transition == SPEC1) return 0;
      else if (transition == SPEC2) return 0;
      else if (transition == QUOTE) return -1;
      else if (transition == SPACE) return 0;
      else if (transition == INVC) return 0;
      else return -1;
      break;
    case 3:
      // DECIMAL
      if (transition == ALPHA) return -1;
      else if (transition == INTEGER) return 3;
      else if (transition == DEC) return -1;
      else if (transition == SPEC1) return 0;
      else if (transition == SPEC2) return 0;
      else if (transition == QUOTE) return -1;
      else if (transition == SPACE) return 0;
      else if (transition == INVC) return 0;
      else return -1;
      break;
    case 4:
      // SPECIAL1
      if (transition == ALPHA) return 0;
      else if (transition == INTEGER) return 0;
      else if (transition == DEC) return 0;
      else if (transition == SPEC1) return 0;
      else if (transition == SPEC2) return 5;
      else if (transition == QUOTE) return 0;
      else if (transition == SPACE) return 0;
      else if (transition == INVC) return 0;
      else return -1;
      break;
    case 5:
      // SPECIAL2
      if (transition == ALPHA) return 0;
      else if (transition == INTEGER) return 0;
      else if (transition == DEC) return 0;
      else if (transition == SPEC1) return 0;
      else if (transition == SPEC2) return 0;
      else if (transition == QUOTE) return 0;
      else if (transition == SPACE) return 0;
      else if (transition == INVC) return 0;
      else return -1;
      break;
    case 6:
      // QUOTE
      if (transition == ALPHA) return 6;
      else if (transition == INTEGER) return 6;
      else if (transition == DEC) return 6;
      else if (transition == SPEC1) return 6;
      else if (transition == SPEC2) return 6;
      else if (transition == QUOTE) return 0;
      else if (transition == SPACE) return 6;
      else if (transition == INVC) return 0;
      else return -1;
      break;
    // ERROR
    case 7:
      // QUOTE
      if (transition == ALPHA) return 0;
      else if (transition == INTEGER) return 0;
      else if (transition == DEC) return 0;
      else if (transition == SPEC1) return 0;
      else if (transition == SPEC2) return 0;
      else if (transition == QUOTE) return 0;
      else if (transition == SPACE) return 0;
      else if (transition == INVC) return 0;
      else return -1;
      break;
    case -1:
      if (transition == ALPHA) return -1;
      else if (transition == INTEGER) return -1;
      else if (transition == DEC) return -1;
      else if (transition == SPEC1) return -1;
      else if (transition == SPEC2) return -1;
      else if (transition == QUOTE) return -1;
      else if (transition == SPACE) return 0;
      else if (transition == INVC) return -1;
      else return -1;
      break;

    default:
      return -1;
      break;
  }
}