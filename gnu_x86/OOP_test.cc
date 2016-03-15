/*
 * =====================================================================================
 *
 *       Filename:  OOP_test.cc
 *
 *    Description:  C++类的继承，重载和多态
 *                  1. 继承体现在子类对于父类的成员变量和成员函数的继承
 *                  2. 重载体现在关于子类赋值给父类的时候，=其实已经是类做过重载operator=
 *                  3. 多态体现在对象使用new分配内存的时候,带有virtual关键字的成员函数调用
 *
         Version:  1.0
 *        Created:  2015年09月22日 17时41分28秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Morning
 *   Organization:  
 *
 * =====================================================================================
 */
#include <iostream>
#include <cstring>
using namespace std;


class test_a
{
    public: 
        int         a;
        int         b;
        string      c;
    public:
        //test_a() {
        //    cout<<"construct class test_a"<<endl;
        //    //c = "hello test_a";
        //}

        void display() {
            c = "hello test_a";
            cout<<"string:"<<c<<endl;
        }

        virtual void disp() {
            display();
            cout<<"parent: "<<"a="<<a<<" b="<<b<<endl;
        }
        
        void sum() {
            cout<<"parent: "<<"a+b="<<a+b<<endl;
        }
};

class test_son_of_a : public test_a
{
    public:
        //test_son_of_a() {
        //    cout<<"construct class test_son_of_a"<<endl;
        //}

        void display() {
            c = "hello test_son_of_a";
            cout<<"string:"<<c<<endl;
        }

        virtual void disp() {
            display();
            cout<<"son: "<<"a="<<a<<" b="<<b<<endl;
        }           

        void sum() {
            cout<<"son: "<<"a+b="<<a+b<<endl;
        }
};


int main(int argc, char *argv[])
{
    test_a          parent;
    test_son_of_a   son;
    test_a          son_of_parent;

    test_a          *p_parent;
    test_son_of_a   *p_son;
    test_a          *p_son_of_parent;
    test_a          *p_badson_of_parent;
    test_a          *p_st;

    cout<<"object info"<<endl;
    parent.a = 3;
    parent.b = 4;
    son.a    = 2;
    son.b    = 3;
    //parent = son;
    son_of_parent = son;

    parent.disp();            //call disp of test_a
    son.disp();               //call disp of test_son_of a
    son_of_parent.disp();     //call disp of test_a

    parent.sum();             //call sum of test_a
    son.sum();                //call sum of test_son_of_a
    son_of_parent.sum();      //call sum of test_a

    /////////////////////////////////////////////////////////
    cout<<endl<<"object point info"<<endl;
    p_son = new test_son_of_a;
    p_parent = new test_a;
    p_son_of_parent = new test_son_of_a;
    p_st = p_badson_of_parent = new test_son_of_a;
    p_badson_of_parent = p_parent;

    p_son->a = 3;
    p_son->b = 4;
    p_parent->a = 2;
    p_parent->b = 3;
    p_son_of_parent->a = 8;
    p_son_of_parent->b = 0;
    p_badson_of_parent->a = 11;
    p_badson_of_parent->b = 22;

    p_parent->disp();               //call disp of test_a
    p_son->disp();                  //call disp of test_son_of_a
    p_son_of_parent->disp();        //call disp of test_son_of_a
    p_badson_of_parent->disp();     //call disp of test_a;
    
    p_parent->sum();                //call sum of test_a
    p_son->sum();                   //call sum of test_son_of_a
    p_son_of_parent->sum();         //call sum of test_a
    p_badson_of_parent->sum();      //call sum of test_a

    delete p_son;
    delete p_parent;
    delete p_son_of_parent;
    delete p_st;

    return 0;
}
