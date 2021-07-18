#include <iostream>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <list>
using namespace std;
//#define _DEV
int getwidth()
{
    int width=1;
    cout<<"\033[?25l ";
    string str;
    termios tm, tm_old;
    tcgetattr(0, &tm_old);
    tm=tm_old;
    tm.c_cc[VTIME] = 0;//立即输入
    tm.c_lflag &= ~(ECHO | ICANON );
    tm.c_cc[VMIN] = 0;//立即输入，getchar()没输入时返回-1
    tcsetattr(0, TCSANOW, &tm);
    for(;str!="2";++width)
    {
        cout<<" \033[6n";
        str="";
        getchar();
        getchar();
        for(char ch; (ch = getchar()) != ';';);
        for(char ch; (ch = getchar()) != 'R'; str += ch);//读取整个缓存
    }
    tcsetattr(0, TCSANOW, &tm_old);
    cout<<"\b\033[1A\033[?25h";
    return width-1;
}

inline void replace_all(string &str, string sub, string replace_to)//把所有子串替换
{
    for(int i; (i = str.find(sub)) != string::npos; str.replace(i, i + sub.size(), replace_to));
}

inline bool isLetterOrNum(char ch)
{
    return (ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9');
}

string _prs(const string&str,int startpoint,int length)
{
    if (str.size() < startpoint)//溢出
        return "";
    if (str.size() < startpoint + length)
        length = str.size() - startpoint;
    int realstart = startpoint;
    for (; realstart > 0 && isLetterOrNum(str[realstart]); --realstart);
    int realend = startpoint + length;
    for (auto const &sz = str.size(); realend < sz && isLetterOrNum(str[realstart]); ++realend);
    string ret = "", color = "";
    int pos, posend;
    pos = posend = realstart;
    bool flag = false;
    int offset = 0;
    while (true)
    {
        for (pos = posend; !isLetterOrNum(str[pos]); ++pos)
            if (pos == realend)
            {
                flag = true;
                break;
            }
        if (flag)
        {
            ret.append(str.substr(posend, realend - posend));
            color.append(realend - posend, '-');
            break;
        }
        ret.append(str.substr(posend, pos - posend));
        color.append(pos - posend, '-');
        for (posend = pos; isLetterOrNum(str[posend]); ++posend);
        string &&tmp = str.substr(pos, posend - pos);
        ret.append(tmp);
        if (tmp == "int")
            color.append(3,'r');
        else
            color.append(posend - pos, '-');
    }
    ret = ret.substr(startpoint - realstart, length);
    color = color.substr(startpoint - realstart, length);
    char currcolor = '-';
    string tmp;
    for (int i = 0; i < length; ++i)
        if (color[i] != currcolor)
        {
            if (color[i] == 'r')
                tmp = "\033[31m";
            else if (color[i] == '-')
                tmp = "\033[0m";
            currcolor = color[i];
            ret.insert(i + offset, tmp);
            offset += tmp.size();
        }
    ret.append("\033[0m");
    return ret;
}

class parse//like ostream
{
public:
    parse &operator<<(const char*);
    parse &operator<<(int);
    parse &operator<<(const list<string>::iterator&);
private:
    string tkin="";
    int startpoint=0;
    bool doparse=false;
    bool available=false;
}pout;
parse&parse::operator<<(const char*str)
{
    cout<<str;
    return *this;
}
parse&parse::operator<<(int i)
{
    if(this->doparse)
    {
        if(this->available)
        {
            cout<<_prs(this->tkin,this->startpoint,i);
            this->available=false;
            this->doparse=false;
        }
        else
        {
            this->available=true;
            this->startpoint=i;
        }
    }
    else
        cout<<i;
    return *this;
}
parse&parse::operator<<(const list<string>::iterator&iter)
{
    this->tkin=*iter;
    this->doparse=true;
    this->available=false;
    return *this;
}

string getch()//返回整个缓存区。（无阻塞）字符会转义替换！
{
    termios tm, tm_old;
    tcgetattr(0, &tm);
    tm_old = tm;
    tm.c_cc[VMIN] = 0;//立即输入，getchar()没输入时返回-1
    tm.c_cc[VTIME] = 0;//立即输入
    tm.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tcsetattr(0, TCSANOW, &tm);

    string str="";

    for(char ch; (ch = getchar()) != -1; str += ch);//读取整个缓存

    tcsetattr(0, TCSANOW, &tm_old);

    //转义替换
    replace_all(str, "\x7F"    , "\x08");//backspace
    replace_all(str, "\033[3~" , "\x7F");//delete
    //replace_all(str, "\033[5~" , "\xF0");//PgUp
    //replace_all(str, "\033[6~" , "\xF1");//PgDn
    replace_all(str, "\033,"   , "\xFE");//Alt+<
    replace_all(str, "\033."   , "\xFF");//Alt+>
    replace_all(str, "\0336"   , "\xFC");//Alt+^
    replace_all(str, "\033v"   , "\xFD");//Alt+v
    return str;
}
struct coord{int x,y;};

#define END 15
#ifdef _DEV
int main()
{
    list<string> lststr(1,"const int intconst;");
    pout<<lststr.begin()<<1<<12;
}
#else
int main()
{
    const int scrwidth=getwidth();
    system("clear");
    cout<<"    nt\n    nt\n    nt\n\n\n\n\n\n\n\n\n\n\n\n\n---------------\033[2;6H";
    string str;
    list<string> lststr(3,"int");
    list<string>::iterator currstr=lststr.begin();
    ++currstr;
    coord scr={1,0},cur={1,1};
    while(true)
    {
        str=getch();
        if(!str.empty())
            for(char ch:str)
            {
                switch(ch)
                {
                case 8://Backspace
                    if(cur.x!=0)
                    {
                        currstr->erase(scr.x+cur.x-1,1);
                        pout<<"\033["<<cur.y+1<<";5H\033[2K"<<currstr<<scr.x<<scrwidth-5;
                        cout<<"\033["<<cur.y+1<<";"<<4+cur.x--<<"H";
                    }
                    else if(scr.x!=0)
                    {
                        currstr->erase(scr.x+cur.x-1,1);
                        --scr.x;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";5H";
                    }
                    else if(cur.y!=0)
                    {
                        list<string>::iterator deaditer=currstr--;
                        scr.x=0;
                        cur.x=currstr->size();
                        currstr->append(*deaditer);
                        lststr.erase(deaditer);
                        --cur.y;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\n\033[2K";
                        cout<<"\033["<<cur.y+1<<";"<<cur.x+5<<"H";
                    }
                    else if(scr.y!=0)
                    {
                        list<string>::iterator deaditer=currstr--;
                        scr.x=currstr->size();
                        *currstr+=*deaditer;
                        lststr.erase(deaditer);
                        --scr.y;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";5H";
                    }
                    break;
                case 127://Delete
                    if(cur.x+scr.x<currstr->size())
                    {
                        currstr->erase(scr.x+cur.x,1);
                        pout<<"\033["<<cur.y+1<<";5H\033[2K"<<currstr<<scr.x<<scrwidth-5;
                        cout<<"\033["<<cur.y+1<<";"<<5+cur.x<<"H";
                    }
                    else if(cur.y+scr.y+1!=lststr.size())
                    {
                        list<string>::iterator deaditer=currstr;
                        currstr->append(*(++deaditer));
                        lststr.erase(deaditer);
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                            pout<<"\033["<<i+1<<";5H\033[2K"<<(headiter++)<<0<<scrwidth-5;
                        cout<<"\n\033[2K";
                        cout<<"\033["<<cur.y+1<<";"<<cur.x+5<<"H";
                    }
                    break;
                case -2://Alt+<
                    if(cur.x!=0)
                        cout<<"\033["<<cur.y+1<<";"<<4+cur.x--<<"H";
                    else if(scr.x!=0)
                    {
                        --scr.x;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";5H";
                    }
                    else if(cur.y!=0)
                        cout<<"\033["<<cur.y--<<";"<<5+(cur.x=(--currstr)->size())<<"H";
                    else if(scr.y!=0)
                    {
                        --scr.y;
                        --currstr;
                        cur.x=currstr->size();
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";5H";
                    }
                    break;
                case -1://Alt+>
                    if(cur.x+scr.x<currstr->size())
                        if(cur.x<scrwidth-5)
                            cout<<"\033["<<cur.y+1<<";"<<++cur.x+5<<"H";
                        else
                        {
                            ++scr.x;
                            list<string>::iterator headiter=currstr;
                            for(int i=cur.y;i>0;--i)
                                --headiter;
                            for(int i=0;i<END&&i<lststr.size();++i)
                            {
                                pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                                ++headiter;
                            }
                            cout<<"\033["<<cur.y+1<<";"<<cur.x+5<<"H";
                        }
                    else if(cur.y+scr.y+1!=lststr.size())
                    {
                        scr.x=0;
                        cur.x=0;
                        if(cur.y==END-1)
                            ++scr.y;
                        else
                            ++cur.y;
                        ++currstr;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<0<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";5H";
                    }
                    break;
                case -4://Alt+^
                    cout<<"F**K";
                    break;
                case -3://Alt+v
                    //cout<<"\033[1B";
                    break;
                case 9://TAB
                    //cout<<"\033[4@\033[4C";
                    break;
                case 10:
                case 13:
                {
                    list<string>::iterator olditer=currstr;
                    ++currstr;
                    lststr.insert(currstr,olditer->substr(scr.x+cur.x));
                    olditer->erase(scr.x+cur.x);
                    cout<<*currstr;
                    scr.x=0;
                    cur.x=0;
                    if(cur.y==END-1)
                        ++scr.y;
                    else
                        ++cur.y;
                    list<string>::iterator headiter=--currstr;
                    for(int i=cur.y;i>0;--i)
                        --headiter;
                    for(int i=0;i<END&&i<lststr.size();++i)
                    {
                        pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                        ++headiter;
                    }
                    cout<<"\033["<<cur.y+1<<";"<<cur.x+5<<"H";
                    break;
                }
                case -16://PgUp, still with bugs
                    if(scr.y!=0)
                    {
                        --scr.y;
                        cur.x=0;
                        --currstr;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";5H";
                    }
                    else if(cur.y!=0)
                    {
                        --cur.y;
                        cur.x=0;
                        --currstr;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";5H";
                    }
                case -15://PgDn, still with bugs
                    if(scr.y+cur.y+1<lststr.size())
                    {
                        ++scr.y;
                        ++currstr;
                        cur.x=0;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H\033[2K"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";5H";
                    }
                default:
                    if(cur.x<scrwidth-5)
                    {
                        currstr->insert(scr.x+cur.x,1,ch);
                        pout<<"\033["<<cur.y+1<<";5H"<<currstr<<scr.x<<scrwidth-5;
                        cout<<"\033["<<cur.y+1<<";"<<++cur.x+5<<"H";
                    }
                    else
                    {
                        currstr->insert(scr.x+cur.x,1,ch);
                        ++scr.x;
                        list<string>::iterator headiter=currstr;
                        for(int i=cur.y;i>0;--i)
                            --headiter;
                        for(int i=0;i<END&&i<lststr.size();++i)
                        {
                            pout<<"\033["<<i+1<<";5H"<<headiter<<scr.x<<scrwidth-5;
                            ++headiter;
                        }
                        cout<<"\033["<<cur.y+1<<";"<<cur.x+5<<"H";
                    }
                }
                cout<<"\033[s";
                for(int i=0;i<END;++i)
                    cout<<"\033["<<i+1<<";1H\033[30;1m"<<i+scr.y+1<<"\033[0m";
                cout<<"\033[16;1H---------------\033[u";
            }
    }
}
#endif
