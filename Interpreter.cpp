#include "Interpreter.h"
#include "header/typedef.h"
#include <algorithm>
#include <iostream>
#include <utility>
#include <c++/fstream>

using namespace std;

void Interpreter::setQuery(string q)
{
    query = std::move(q);
    Pretreatment();
    setWords();
}

void Interpreter::Pretreatment()
{
    //replace all tab with space
    replace(query.begin(), query.end(), '\t', ' ');

    //delete spaces in the beginning or end
    query.erase(0, query.find_first_not_of(' '));
    query.erase(1 + query.find_last_not_of(' '));

    if (query.substr(0, 8) == "execfile")
    {
        runExecFile();
        return;
    }

    /*
     * add a space before and after every sign
     * notice that we donot add a space around quotation mark
     * it saves a lot of work
     */
    for (int pos = 0; pos < query.length(); ++pos)
    {
        if (query[pos] == '<' || query[pos] == '>' || query[pos] == '=' || query[pos] == '(' || query[pos] == ')' ||
            query[pos] == ';' || query[pos] == ',' || query[pos] == '*')
        {
            if (query[pos - 1] != ' ')
            {
                query.insert(pos, " ");
                pos++;
            }

            if (query[pos + 1] != ' ')
            {
                query.insert(pos + 1, " ");
                pos++;
            }
        }
    }

    //delete consecutive spaces, which avoids methods like "find_first_not_of"
    for (int pos = 0; pos < query.size(); ++pos)
        if (query[pos] == ' ' && query[pos + 1] == ' ')
        {
            query.erase(pos + 1, 1);
            pos--;//if we just continue next loop, we only check this position once
        }

}

void Interpreter::setWords()
{
    /*
     * this function split the query string into words
     * we do not view punctuation marks as a word
     */
    words.emplace_back("");
    string s = query;
    while (!s.empty())
    {
        /*
         * handle strings
         * note that we only use single quotation mark to express a string
         */
        if (s[0] == '\'')
        {
            s.erase(0, 1);
            words.push_back(s.substr(0, s.find_first_of('\'')));
            isString.push_back(true);
            s = s.substr(1 + s.find_first_of('\''));
            continue;
        }
        //a number is also a word
        if ((s[0] >= 'A' && s[0] <= 'Z') || (s[0] >= 'a' && s[0] <= 'z')
            || (s[0] >= '0' && s[0] <= '9'))
        {
            words.push_back(s.substr(0, s.find_first_of(' ')));
            isString.push_back(false);
        }
        s = s.substr(1 + s.find_first_of(' '));
    }
}


int Interpreter::runQuery()
{
    //op can be select, create, drop, insert, delete, execfile, quit
    //otherwise throw an invalid query exception
    string op = words[1];

    if (op == "quit")
    { return 0; }

    if (op == "create")
    {
        string obj = words[2];
        if (obj == "table")
        {
            string tableName = words[3];
            int cnt = 4;
            vector<Attribute> attr;
            string primaryKey;
            bool primaryKeyDefined = false;
            while (cnt < words.size())
            {
                if (words[cnt] == "primary" && words[cnt + 1] == "key")
                {
                    if (primaryKeyDefined)//TODO: throw exception
                        ;
                    primaryKeyDefined = true;
                    cnt += 2;
                    primaryKey = words[cnt];
                    cnt++;
                    continue;
                }
                Attribute nextAttr;
                nextAttr.name = words[cnt];
                cnt++;//type
                if (words[cnt] == "int")
                {
                    nextAttr.type = 0;
                } else if (words[cnt] == "float")
                {
                    nextAttr.type = 1;
                } else if (words[cnt] == "char")
                {
                    nextAttr.type = 2;
                    cnt++;
                    if (atoi(words[cnt].c_str()) < 1 || atoi(words[cnt].c_str()) > 255);//TODO: throw exception
                    nextAttr.length = atoi(words[cnt].c_str());
                } else
                {
                    //TODO: throw exception
                }
                cnt++;
                if (words[cnt] == "unique")
                    nextAttr.isUnique = true;
                attr.push_back(nextAttr);
            }
            //now we have a vector of attributes, a table name and a string of priamry key name
            //TODO: invoke create_table from api
        }
        if (obj == "index")
        {
            if (words[4] != "on")//TODO: throw exception
                ;
            string indexName = words[3];
            string tableName = words[5];
            string colName = words[6];
            //TODO: invoke create_index from api
        } else
        {
            //TODO: throw exception
        }
    } else if (op == "select")
    {
        if (words[2] != "from")
        {
            //TODO:throw exception
        }
        //in MiniSQL, we only need to implement "select *"
        string tableName = words[3];

        if (query.find("where") != string::npos)
        {
            vector<WhereQuery> wq = runWhere(5);
            //TODO: invoke select
        }

    } else if (op == "drop")
    {
        string obj = words[2];
        if (obj == "table")
        {
            string tableName = words[3];
            //TODO: invoke drop_table from api
        } else if (obj == "index")
        {
            string indexName = words[3];
            //TODO: invoke drop_index from api
        } else
        {
            //TODO: throw exception
        }
    } else if (op == "insert")
    {
        if (words[2] != "into" || words[4] != "values")//TODO: throw exception
            ;
        string tableName = words[3];
        vector<Data *> data;
        for (int cnt = 5; cnt < words.size(); ++cnt)
        {
            Data *a;
            if (isString[cnt])
                a = new sData(words[cnt]);
            else
            {
                if (words[cnt].find('.') != string::npos)
                    a = new fData(atof(words[cnt].c_str()));
                else
                    a = new iData(atoi(words[cnt].c_str()));
            }
            data.push_back(a);
        }
        //TODO: invoke insert
    } else if (op == "delete")
    {
        string tableName = words[3];
        vector<WhereQuery> wq = runWhere(5);
    } else
    {
        //TODO: throw exception
    }
    return 1;
}

vector<WhereQuery> Interpreter::runWhere(int k)
{
    vector<WhereQuery> wq;
    string col;
    COMPARE op;
    while (k < words.size())
    {
        col = words[k];
        k++;
        if (words[k] == "=")
            op = e;
        else if (words[k] == "!=")
            op = ne;
        else if (words[k] == "<")
            op = lt;
        else if (words[k] == ">")
            op = gt;
        else if (words[k] == "<=")
            op = le;
        else if (words[k] == ">=")
            op = ge;
        else
        {
            //TODO: throw exception
        }
        k++;
        if (isString[k])
        {
            sData s(words[k]);
            WhereQuery w(s);
            w.op = op;
            w.col = col;
            wq.push_back(w);
        } else if (words[k].find('.') != string::npos)
        {
            fData f(atof(words[k].c_str()));
            WhereQuery w(f);
            w.op = op;
            w.col = col;
            wq.push_back(w);
        } else
        {
            iData i(atoi(words[k].c_str()));
            WhereQuery w(i);
            w.op = op;
            w.col = col;
            wq.push_back(w);
        }
    }
    return wq;
}

void Interpreter::runExecFile()
{
    string fileName = query.substr(9);
    fileName.erase(fileName.size() - 1);
    ifstream in(fileName);
    if (!in);//TODO: throw exception
    query = "";
    string saved;
    while (in.peek() != EOF)
    {
        try
        {
            in >> saved;
            query = query + saved + " ";
            if (saved.back() == ';')
            {
                Pretreatment();
                setWords();
                runQuery();
                query = "";
            }
        }
        catch (exception e){}
    }
}