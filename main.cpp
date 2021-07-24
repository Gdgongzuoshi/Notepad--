#include <iostream>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>  
#include <list>
using namespace std;
//#define _DEV
int getwidth()
{
    termios tm, tm_old;
    tcgetattr(0, &tm_old);
    tm=tm_old;
    tm.c_cc[VTIME] = 0;//立即输入
    tm.c_lflag &= ~(ECHO | ICANON );
    tm.c_cc[VMIN] = 0;//立即输入，getchar()没输入时返回-1
    tcsetattr(0, TCSANOW, &tm);
    cout<<"\033[1;1H\033[1000C\033[6n";
    string str="";
    getchar();
    getchar();
    while(getchar() != ';');
    for(char ch; (ch = getchar()) != 'R'; str += ch);//读取整个缓存
    tcsetattr(0, TCSANOW, &tm_old);
    cout<<"\033[1000D";
    return stoi(str);
}

inline void replace_all(string &str, string sub, string replace_to)//把所有子串替换
{
    for(int i; (i = str.find(sub)) != string::npos; str.replace(i, i + sub.size(), replace_to));
}

inline bool isLetterOrNum(char ch)
{
    return (ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')||(ch=='_');
}
inline bool isLetter(char ch)
{
    return (ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch=='_');
}
inline bool isNum(char ch)
{
    return (ch>='0'&&ch<='9');
}


string getch();//返回整个缓存区。（无阻塞）字符会转义替换！

struct coord{int x,y;};

const int scrwidth=getwidth();
string matchcolor(char);
#define END 15


void highlight(list<string>&,list<string>&,list<string>::iterator,list<string>::iterator,int);


void print(int,list<string>::iterator,list<string>::iterator,coord,coord);



#ifdef _DEV
int main()
{
    list<string> lststr(3,"-int s");
    list<string> lstcolor(3,"######");
    list<string>::iterator currstr=lststr.begin();
    list<string>::iterator currcolor=lstcolor.begin();
    highlight(lststr,lstcolor,currstr,currcolor,0);
    print(3,currstr,currcolor,{0,0},{0,0});
    cout<<"\n\n\n\n\n\n\n\n";
}
#else
int main()
{
    system("clear");
    cout<<"\033[31m    int\n    int\n    int\n\n\n\n\n\n\n\n\n\n\n\n\n\033[0m---------------\033[2;6H";
    string str;
    list<string> lststr(3,"int");
    list<string> lstcolor(3,"ttt");
    list<string>::iterator currstr=lststr.begin();
    list<string>::iterator currcolor=lstcolor.begin();
    ++currcolor;
    ++currstr;
    coord scr={0,0},cur={1,1};
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
                        currcolor->erase(scr.x+cur.x-1,1);
                        --cur.x;
                        highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    else if(scr.x!=0)
                    {
                        currstr->erase(scr.x+cur.x-1,1);
                        currcolor->erase(scr.x+cur.x-1,1);
                        --scr.x;
                        highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    else if(cur.y!=0)
                    {
                        list<string>::iterator deaditer=currstr--,deadcor=currcolor--;
                        scr.x=0;
                        cur.x=currstr->size();
                        currstr->append(*deaditer);
                        currcolor->append(*deadcor);
                        lststr.erase(deaditer);
                        lstcolor.erase(deadcor);
                        --cur.y;
                        highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    else if(scr.y!=0)
                    {
                        list<string>::iterator deaditer=currstr--;
                        list<string>::iterator deadcolor=currcolor--;
                        scr.x=currstr->size();
                        *currstr+=*deaditer;
                        *currcolor+=*deadcolor;
                        lststr.erase(deaditer);
                        lstcolor.erase(deadcolor);
                        --scr.y;
                        highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    break;
                case 127://Delete
                    if(cur.x+scr.x<currstr->size())
                    {
                        currstr->erase(scr.x+cur.x,1);
                        currcolor->erase(scr.x+cur.x,1);
                        highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    else if(cur.y+scr.y+1!=lststr.size())
                    {
                        list<string>::iterator deaditer=currstr;
                        list<string>::iterator deadcolor=currcolor;
                        currstr->append(*(++deaditer));
                        currcolor->append(*(++deadcolor));
                        lststr.erase(deaditer);
                        lstcolor.erase(deadcolor);
                        highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    break;
                case -2://Alt+<
                    if(cur.x!=0)
                    {
                        --cur.x;
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    else if(scr.x!=0)
                    {
                        --scr.x;
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    else if(cur.y!=0)
                    {
                        cout<<"\033["<<cur.y--<<";"<<5+(cur.x=(--currstr)->size())<<"H";
                        --currcolor;
                    }
                    else if(scr.y!=0)
                    {
                        --scr.y;
                        --currstr;
                        --currcolor;
                        cur.x=currstr->size();
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    break;
                case -1://Alt+>
                    if(cur.x+scr.x<currstr->size())
                        if(cur.x<scrwidth-5)
                        {
                            ++cur.x;
                            cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                            print(lststr.size(),currstr,currcolor,scr,cur);
                        }
                        else
                        {
                            ++scr.x;
                            cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                            print(lststr.size(),currstr,currcolor,scr,cur);
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
                        ++currcolor;
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    break;
                case -4://Alt+^
                    cout<<"F**K";
                    break;
                case -3://Alt+v
                case 9://TAB
                    break;
                case 10:
                case 13:
                {
                    list<string>::iterator olditer=currstr;
                    list<string>::iterator oldcolor=currcolor;
                    ++currstr;
                    ++currcolor;
                    lststr.insert(currstr,olditer->substr(scr.x+cur.x));
                    lstcolor.insert(currcolor,oldcolor->substr(scr.x+cur.x));
                    olditer->erase(scr.x+cur.x);
                    oldcolor->erase(scr.x+cur.x);
                    --currstr;
                    --currcolor;
                    scr.x=0;
                    cur.x=0;
                    if(cur.y==END-1)
                        ++scr.y;
                    else
                        ++cur.y;
                    highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                    cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                    print(lststr.size(),currstr,currcolor,scr,cur);
                    break;
                }
                case -16://PgUp
                case -15://PgDn
                    break;
                default:
                    if(cur.x<scrwidth-5)
                    {
                        currstr->insert(scr.x+cur.x,1,ch);
                        currcolor->insert(scr.x+cur.x,1,'-');
                        ++cur.x;
                        highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
                    }
                    else
                    {
                        currstr->insert(scr.x+cur.x,1,ch);
                        currcolor->insert(scr.x+cur.x,1,'-');
                        ++scr.x;
                        highlight(lststr,lstcolor,currstr,currcolor,cur.x+scr.x);
                        cout<<"\033[20;1H\033[2K"<<*currstr<<"\n\033[2K"<<*currcolor;
                        print(lststr.size(),currstr,currcolor,scr,cur);
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






















string matchcolor(char ch)
{
    switch(ch)
    {
    case '-':
        return "\033[0m";
    case 't':
        return "\033[31m";
    case 'i':
        return "\033[0m";
    case 'N':
        return "\033[38;5;21m";
    default:
        return "\033[36m";
    }
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

void print(int sz,list<string>::iterator headstr,list<string>::iterator headcolor,coord scr,coord cur)
{
    for(int i=cur.y;i>0;--i)
    {
        --headstr;
        --headcolor;
    }
    string tmp,color;
    for(int i=0;i<END&&i<sz;++i)
    {
        cout<<"\033["<<i+1<<";5H\033[2K";
        if(scr.x<headstr->size())
        {
            tmp=headstr->substr(scr.x,scrwidth-5);
            color=headcolor->substr(scr.x,scrwidth-5);
        }
        else
        {
            tmp="";
            color="";
        }
        for(int i=0;i<tmp.size();++i)
            cout<<matchcolor(color[i])<<tmp[i];
        ++headstr;
        ++headcolor;
    }
    cout<<"\n\033[2K"<<"\033["<<cur.y+1<<";"<<cur.x+5<<"H";
}

void highlight(list<string>&lststr,list<string>&lstcolor,list<string>::iterator changed,list<string>::iterator clr,int cval)
{
    enum{NUMBER,IDENTIFIER,LINE_COMMENT,BLOCK_COMMENT,PRE_PROCESS,STRING,LINE_START,WAITING}status;
    string currstr=*changed;
    string currcolor=*clr;
    int startpoint=-1;
    for(;isLetterOrNum(currstr[startpoint]);--startpoint);
    int currpoint=0;
    while(1)
    {
        if(isNum(currstr[currpoint]))
            status=NUMBER;
        else if(isLetter(currstr[currpoint]))
            status=IDENTIFIER;
        else
            status=WAITING;
        if(status==NUMBER)
            for(;isLetterOrNum(currstr[currpoint]);++currpoint)
                currcolor[currpoint]='N';
        if(status==IDENTIFIER)
        {
            string id="";
            const int startid=currpoint;
            int idlength=0;
            while(isLetterOrNum(currstr[currpoint]))
            {
                id+=currstr[currpoint];
                ++currpoint;
                ++idlength;
            }
            cout<<startid;
            if(id=="int")
                currcolor[startid]=currcolor[startid+1]=currcolor[startid+2]='t';
            else
                for(int i=0;i<idlength;++i)
                    currcolor[startid+i]='i';
        }
        if(status==WAITING)
        {
            currcolor[currpoint]='-';
            ++currpoint;
        }
        if(currpoint>=currstr.size())
        {
            *clr=currcolor;
            if(changed==(--lststr.end()))
                break;
            currstr=*(++changed);
            currcolor=*(++clr);
            currpoint=0;
        }
    }
}