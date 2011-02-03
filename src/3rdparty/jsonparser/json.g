-- ***************************************************************************
-- 
--  Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
-- 
--  Use, modification and distribution is allowed without limitation,
--  warranty, liability or support of any kind.
-- 
-- **************************************************************************

%parser    JsonGrammar
%merged_output jsonparser.cpp

%token T_STRING "string"
%token T_NUMBER "number"
%token T_LCURLYBRACKET "{"
%token T_RCURLYBRACKET "}"
%token T_LSQUAREBRACKET "["
%token T_RSQUAREBRACKET "]"
%token T_COLON ":"
%token T_COMMA ","
%token T_FALSE "false" 
%token T_TRUE "true"
%token T_NULL "null"
%token ERROR "error"

%start Root

/:
/****************************************************************************
**
** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef JSONPARSER_P_H
#define JSONPARSER_P_H

#include <QString>
#include <QVariant>
#include <QVector>

class JsonLexer
{
public:
    JsonLexer(const QByteArray &data);
    ~JsonLexer();

    int lex();
    QVariant symbol() const { return m_symbol; }
    int lineNumber() const { return m_lineNumber; }
    int pos() const { return m_pos; }

private:
    int parseNumber();
    int parseString();
    int parseKeyword();

    QByteArray m_data;
    int m_lineNumber;
    int m_pos;
    QVariant m_symbol;
    QHash<QByteArray, int> m_keywords;
};

class JsonParser : protected $table
{
public:
    JsonParser();
    ~JsonParser();

    bool parse(JsonLexer *lex);
    QVariant result() const { return m_result; }
    QString errorMessage() const { return QString("%1 at line %2 pos %3").arg(m_errorMessage).arg(m_errorLineNumber).arg(m_errorPos); }

private:
    void reallocateStack();

    inline QVariant &sym(int index)
    { return m_symStack[m_tos + index - 1]; }

    int m_tos;
    QVector<int> m_stateStack;
    QVector<QVariant> m_symStack;
    QString m_errorMessage;
    int m_errorLineNumber;
    int m_errorPos;
    QVariant m_result;
};

#endif // JSONPARSER_P_H
:/

/.
/****************************************************************************
**
** Copyright (c) 2010 Girish Ramakrishnan <girish@forwardbias.in>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include <QtDebug>

JsonLexer::JsonLexer(const QByteArray &ba)
    : m_data(ba), m_lineNumber(1), m_pos(0)
{
    m_keywords.insert("true", JsonGrammar::T_TRUE);
    m_keywords.insert("false", JsonGrammar::T_FALSE);
    m_keywords.insert("null", JsonGrammar::T_NULL);
}

JsonLexer::~JsonLexer()
{
}

int JsonLexer::parseString()
{
    QString str;
    bool esc = false;
    ++m_pos; // skip initial "
    for (; m_pos < m_data.length(); ++m_pos) {
        const char c = m_data[m_pos];
        if (esc) {
            if (c == 'b') str += '\b';
            else if (c == 'f') str += '\f';
            else if (c == 'n') str += '\n';
            else if (c == 'r') str += '\r';
            else if (c == 't') str += '\t';
            else if (c == '\\') str += '\\';
            else if (c == '\"') str += '\"';
            else if (c == 'u' && m_pos+4<m_data.length()-1) {
                QByteArray u1 = m_data.mid(m_pos+1, 2);
                QByteArray u2 = m_data.mid(m_pos+3, 2);
                bool ok;
                str += QChar(u2.toInt(&ok, 16), u1.toInt(&ok, 16));
                m_pos += 4;
            } else {
                str += c;
            }
            esc = false;
        } else if (c == '\\') {
            esc = true;
        } else if (c == '\"') {
            m_symbol = str;
            ++m_pos;
            return JsonGrammar::T_STRING;
        } else {
            str += c;
        }
    }
    return JsonGrammar::ERROR;
}

int JsonLexer::parseNumber()
{
    int start = m_pos;
    bool isDouble = false;
    for (; m_pos < m_data.length(); ++m_pos) {
        const char c = m_data[m_pos];
        if (c == '+' || c == '-' || (c >= '0' && c <= '9'))
            continue;
        if (c == '.' || c == 'e' || c == 'E') {
            isDouble = true;
            continue;
        }
        break;
    }
    QByteArray number = QByteArray::fromRawData(m_data.constData()+start, m_pos-start);
    bool ok;
    if (!isDouble) {
        m_symbol = number.toInt(&ok);
        if (!ok)
            m_symbol = number.toLongLong(&ok);
    }
    if (isDouble || !ok)
        m_symbol = number.toDouble();
    return JsonGrammar::T_NUMBER;
}

int JsonLexer::parseKeyword()
{
    int start = m_pos;
    for (; m_pos < m_data.length(); ++m_pos) {
        const char c = m_data[m_pos];
        if (c >= 'a' && c <= 'z')
            continue;
        break;
    }
    QByteArray keyword = QByteArray::fromRawData(m_data.constData()+start, m_pos-start);
    if (m_keywords.contains(keyword))
        return m_keywords.value(keyword);
    return JsonGrammar::ERROR;
}

int JsonLexer::lex()
{
    m_symbol.clear();
    while (m_pos < m_data.length()) {
        const char c = m_data[m_pos];
        switch (c) {
        case '[': ++m_pos; return JsonGrammar::T_LSQUAREBRACKET;
        case ']': ++m_pos; return JsonGrammar::T_RSQUAREBRACKET;
        case '{': ++m_pos; return JsonGrammar::T_LCURLYBRACKET;
        case '}': ++m_pos; return JsonGrammar::T_RCURLYBRACKET;
        case ':': ++m_pos; return JsonGrammar::T_COLON;
        case ',': ++m_pos; return JsonGrammar::T_COMMA;
        case ' ': case '\r': case '\t': case 'b': ++m_pos; break;
        case '\n': ++m_pos; ++m_lineNumber; break;
        case '"': return parseString();
        default: 
            if (c == '+' || c == '-' || (c >= '0' && c <= '9')) {
                return parseNumber(); 
            }
            if (c >= 'a' && c <= 'z') {
                return parseKeyword();
            }
            return JsonGrammar::ERROR;
        }
    }
    return JsonGrammar::EOF_SYMBOL;
}

JsonParser::JsonParser()
{
}

JsonParser::~JsonParser()
{
}

void JsonParser::reallocateStack()
{
    int size = m_stateStack.size();
    if (size == 0)
        size = 128;
    else
        size <<= 1;
    m_symStack.resize(size);
    m_stateStack.resize(size);
}

bool JsonParser::parse(JsonLexer *lexer)
{
    const int INITIAL_STATE = 0;
    int yytoken = -1;
    reallocateStack();
    m_tos = 0;
    m_stateStack[++m_tos] = INITIAL_STATE;

    while (true) {
        const int state = m_stateStack[m_tos];
        if (yytoken == -1 && -TERMINAL_COUNT != action_index[state]) {
            yytoken = lexer->lex();
        }
        int act = t_action(state, yytoken);
        if (act == ACCEPT_STATE)
            return true;
        else if (act > 0) {
            if (++m_tos == m_stateStack.size())
                reallocateStack();
            m_stateStack[m_tos] = act;
            m_symStack[m_tos] = lexer->symbol();
            yytoken = -1;
        } else if (act < 0) {
            int r = -act-1;
            m_tos -= rhs[r];
            act = m_stateStack.at(m_tos++);
            switch (r) {
./

Root ::= Value;
/.          case $rule_number: { m_result = sym(1); break; } ./

Object ::= T_LCURLYBRACKET Members T_RCURLYBRACKET;
/.          case $rule_number: { sym(1) = sym(2); break; } ./

Members ::= Member; 

Members ::= Members T_COMMA Member; 
/.          case $rule_number: { sym(1) = sym(1).toMap().unite(sym(3).toMap()); break; } ./

Members ::= ; 
/.          case $rule_number: { sym(1) = QVariantMap(); break; } ./

Member ::= T_STRING T_COLON Value; 
/.          case $rule_number: { QVariantMap map; map.insert(sym(1).toString(), sym(3)); sym(1) = map; break; } ./

Value ::= T_FALSE;    
/.          case $rule_number: { sym(1) = QVariant(false); break; } ./

Value ::= T_TRUE;
/.          case $rule_number: { sym(1) = QVariant(true); break; } ./

Value ::= T_NULL;
Value ::= Object;
Value ::= Array;
Value ::= T_NUMBER;
Value ::= T_STRING;

Array ::= T_LSQUAREBRACKET Values T_RSQUAREBRACKET;
/.          case $rule_number: { sym(1) = sym(2); break; } ./

Values ::= Value;
/.          case $rule_number: { QVariantList list; list.append(sym(1)); sym(1) = list; break; } ./

Values ::= Values T_COMMA Value;
/.          case $rule_number: { QVariantList list = sym(1).toList(); list.append(sym(3)); sym(1) = list; break; } ./

Values ::= ;
/.          case $rule_number: { sym(1) = QVariantList(); break; } ./

/.
            } // switch
            m_stateStack[m_tos] = nt_action(act, lhs[r] - TERMINAL_COUNT);
        } else {
            int ers = state;
            int shifts = 0;
            int reduces = 0;
            int expected_tokens[3];
            for (int tk = 0; tk < TERMINAL_COUNT; ++tk) {
                int k = t_action(ers, tk);

                if (! k)
                  continue;
                else if (k < 0)
                  ++reduces;
                else if (spell[tk]) {
                    if (shifts < 3)
                      expected_tokens[shifts] = tk;
                    ++shifts;
                }
            }

            m_errorLineNumber = lexer->lineNumber();
            m_errorPos = lexer->pos();
            m_errorMessage.clear();
            if (shifts && shifts < 3) {
                bool first = true;

                for (int s = 0; s < shifts; ++s) {
                    if (first)
                      m_errorMessage += QLatin1String("Expected ");
                    else
                      m_errorMessage += QLatin1String(", ");

                    first = false;
                    m_errorMessage += QLatin1String("'");
                    m_errorMessage += QLatin1String(spell[expected_tokens[s]]);
                    m_errorMessage += QLatin1String("'");
                }
            }
            return false;
        }
    }

    return false;
}

./

