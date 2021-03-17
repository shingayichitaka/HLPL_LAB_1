
#include "std_lib_facilities.h"

class Token {
public:
	char kind;      
	double value;   
	string name;    
	Token(char ch) :kind(ch), value(0) { }                  
	Token(char ch, double val) :kind(ch), value(val) { }    
    Token(char ch, string n) :kind(ch), name(n) { }         
};

// stream of tokens
class Token_stream {
public:
    Token_stream();         
	Token get();            
    void putback(Token t);  
    void ignore(char c);    
private:
	bool full;         
	Token buffer;       

};

Token_stream::Token_stream()
    :full(false), buffer(0) { }  // no Token in buffer

const char let = 'L';           // declaration token
const char con = 'C';           // constant token
const char quit = 'q';          // t.kind==quit means that t is q quit Token
const char help = 'h';          // help token
const char print = ';';         // t.kind==print means that t is a print Token
const char number = '8';        // t.kind==number means that t is a number Token
const char name = 'a';          // name token
const char square_root = 's';   // square root token
const char power = 'p';         // power function token
const string declkey = "let";   // declaration keyword
const string conkey = "const";  // constant keyword
const string sqrtkey = "sqrt";  // keyword for square root
const string powkey = "pow";    // keyword for power function
const string quitkey = "quit";  // keyword to quit
const string helpkey = "help";  // keyword for help

Token Token_stream::get()
{
	if (full) { 
        full = false;
        return buffer;
    }

	char ch;
    cin.get(ch);    
    while (isspace(ch)) {
        if (ch == '\n') return Token(print); 
        cin.get(ch);
    }

	switch (ch) {
    case print:
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
    case '%':
    case '=':
    case ',':
		return Token(ch);   
	case '.':   
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':   
	{	cin.putback(ch);    
		double val;
		cin >> val;         
		return Token(number,val);
	}
	default:
		if (isalpha(ch)) {
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch=='_')) s+=ch;
			cin.putback(ch);
            if (s == declkey) return Token(let);            // declaration keyword
            if (s == conkey) return Token(con);             // constant keyword
            if (s == sqrtkey) return Token(square_root);    // square root keyword
            if (s == powkey) return Token(power);           // power function keyword
            if (s == helpkey) return Token(help);           // help keyword
            if (s == quitkey) return Token(quit);           // quit keyword
			return Token(name,s);
		}
		error("Bad token");
	}
}


void Token_stream::putback(Token t)
{
    if (full) error("putback() into full buffer");
    buffer = t;    
}
void Token_stream::ignore(char c)
{
    
	if (full && c==buffer.kind) {
		full = false;
		return;
	}
	full = false;

    
	char ch = 0;
	while (cin>>ch)
		if (ch==c) return;
}

class Variable {
public:
	string name;
	double value;
    bool is_const;
	Variable(string n, double v, bool b) :name(n), value(v), is_const(b) { }
};

class Symbol_table {
public:
    double get(string s);                           
    void set(string s, double d);                   
    bool is_declared(string var);                   
    double declare(string var,double val,bool b);   
private:
    vector<Variable> var_table; 
};

double Symbol_table::get(string s)
{
	for (int i = 0; i<var_table.size(); ++i)
		if (var_table[i].name == s) return var_table[i].value;
	error("get: undefined variable ",s);
}


void Symbol_table::set(string s, double d)
{
	for (int i = 0; i<var_table.size(); ++i)
		if (var_table[i].name == s) {
            if (var_table[i].is_const) error(s," is a constant");
			var_table[i].value = d;
			return;
		}
	error("set: undefined variable ",s);
}


bool Symbol_table::is_declared(string var)
{
	for (int i = 0; i<var_table.size(); ++i)
		if (var_table[i].name == var) return true;
	return false;
}
double Symbol_table::declare(string var, double val, bool b)
{
    if (is_declared(var)) error(var," declared twice");
    var_table.push_back(Variable(var,val,b));
    return val;
}



Token_stream ts;        
Symbol_table st;        
double expression();    

double my_pow(double base, int expo)
{
    if (expo == 0) {
        if (base == 0) return 0;    
        return 1;                   
    }
    double res = base;              
    for (int i = 2; i<=expo; ++i)   
        res *= base;
    return res;
}

double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':   
	{	double d = expression();
		t = ts.get();
		if (t.kind != ')') error("')' expected");
        return d;
	}
	case '-':
		return - primary();
    case '+':
        return primary();
	case number:
		return t.value;
    case name:
    {   Token t2 = ts.get();    
        if (t2.kind == '=') {   
            double d = expression();
            st.set(t.name,d);
            return d;
        }
        else {  
            ts.putback(t2);
            return st.get(t.name);
        }
    }
    case square_root:   
    {   t = ts.get();
        if (t.kind != '(') error("'(' expected");
        double d = expression();
        if (d < 0) error("Square roots of negative numbers... nope!");
        t = ts.get();
        if (t.kind != ')') error("')' expected");
        return sqrt(d);
    }
    case power: 
    {   t = ts.get();
        if (t.kind != '(') error("'(' expected");
        double d = expression();
        t = ts.get();
        if (t.kind != ',') error("',' expected");
        t = ts.get();
        if (t.kind != number) error("second argument of 'pow' is not a number");
        int i = int(t.value);
        if (i != t.value) error("second argument of 'pow' is not an integer");
        t = ts.get();
        if (t.kind != ')') error("')' expected");
        return my_pow(d,i);
    }
	default:
		error("primary expected");
	}
}

double term()
{
	double left = primary();
    Token t = ts.get(); 

	while (true) {
		switch(t.kind) {
		case '*':
			left *= primary();
            t = ts.get();
			break;
		case '/':
		{	double d = primary();
			if (d == 0) error("divide by zero");
			left /= d;
            t = ts.get();
			break;
		}
        case '%':
        {   double d = primary();
            int i1 = int(left);
            if (i1 != left) error("left-hand operand of % not int");
            int i2 = int(d);
            if (i2 != d) error("right-hand operand of % not int");
            if (i2 == 0) error("%: divide by zero");
            left = i1%i2;
            t = ts.get();
            break;
        }
		default:
			ts.putback(t);  
			return left;
		}
	}
}

double expression()
{
	double left = term();   
    Token t = ts.get();     

	while (true) {
		switch(t.kind) {
		case '+':
			left += term(); 
            t = ts.get();
			break;
		case '-':
			left -= term(); 
            t = ts.get();
			break;
        case '=':
            error("use of '=' outside of a declaration");
		default:
			ts.putback(t);  
			return left;    
		}
	}
}

double declaration(bool b)
{
	Token t = ts.get();
	if (t.kind != name) error ("name expected in declaration");
	string var_name = t.name;

	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ",var_name);

	double d = expression();
    st.declare(var_name,d,b);
	return d;
}

double statement()
{
	Token t = ts.get();
	switch(t.kind) {
	case let:
		return declaration(false);
    case con:
        return declaration(true);
	default:
		ts.putback(t);
		return expression();
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

void print_help()
{
    cout << "Instructions: enter expressions, there are\n";
    cout << "a few functions, you can declare variables using\n";
    cout << "the 'let' keyword and constants with the 'const'\n";
    cout << "keyword.\n";
}

const string prompt = "> ";
const string result = "= "; 

void calculate()
{
	while (cin)
    try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t=ts.get(); 
        if (t.kind == help) print_help();   
        else {
            if (t.kind == quit) return;
            ts.putback(t);
            cout << result << statement() << endl;
        }
	}
	catch(exception& e) {
		cerr << e.what() << endl;   
		clean_up_mess();
	}
}
int main()
try
{
    
    st.declare("pi",3.1415926535,true);
    st.declare("e",2.7182818284,true);
    st.declare("k",1000,true);

	calculate();

    keep_window_open(); 
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin>>c && c!=';') ;
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin>>c && c!=';');
	return 2;
}
