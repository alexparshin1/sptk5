/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       thread_rwlock.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/cutils>
#include <sptk5/cthreads>

using namespace std;
using namespace sptk;

class T1: public Thread
{
    RWLock *lock;
    const char* name;
    int a;
public:
    T1(const char* name, RWLock *lock) :
        Thread(name), lock(lock), name(name), a(1)
    {
    }
    void act(int _a)
    {
        cout << name << " Setting act to " << _a << endl;
        this->a = _a;
    }
    void actionL()
    {
        cout << name << " is trying to lock" << endl;
        int rc = lock->lockR(chrono::milliseconds(10));
        cout << name << " got it(" << rc << ")" << endl;
        if (a == 1)
            a = 0;
    }
    void actionU()
    {
        cout << name << " is trying to ulock" << endl;
        lock->unlock();
        cout << name << " got it" << endl;
        if (a == 2)
            a = 0;
    }
    void threadFunction() override
    {
        cout << "Thread " << name << " starting" << endl;
        while (true) {
            //cout<<name<<": act="<<a<<'\n';
            switch (a)
            {
            case 1:
                actionL();
                break;
            case 2:
                actionU();
                break;
            case 3:
                return;
            default:
                break;
            }
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
};
class T2: public Thread
{
    RWLock *lock;
    const char* name;
    int a;
public:
    T2(const char* name, RWLock *lock) :
        Thread(name), lock(lock), name(name), a(1)
    {
    }
    void act(int _a)
    {
        cout << name << " Setting act to " << _a << endl;
        this->a = _a;
    }
    void actionL()
    {
        cout << name << " is trying to lock" << endl;
        int rc = lock->lockRW(chrono::milliseconds(10));
        cout << name << " got it(" << rc << ")" << endl;
        if (a == 1)
            a = 0;
    }
    void actionU()
    {
        cout << name << " is trying to ulock" << endl;
        lock->unlock();
        cout << name << " got it" << endl;
        if (a == 2)
            a = 0;
    }
    void threadFunction() override
    {
        cout << "Thread " << name << " starting" << endl;
        while (true) {
            //cout<<name<<": act="<<a<<'\n';
            switch (a)
            {
            case 1:
                actionL();
                break;
            case 2:
                actionU();
                break;
            case 3:
                return;
            default:
                break;
            }
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
};

int main()
{
    RWLock lock;
    T1 t11("t11", &lock), t12("t12", &lock);
    T2 t21("t21", &lock), t22("t22", &lock);
    t11.run();
    this_thread::sleep_for(chrono::seconds(1));
    t12.run();
    this_thread::sleep_for(chrono::seconds(1));
    t21.run();
    this_thread::sleep_for(chrono::seconds(1));
    t22.run();
    this_thread::sleep_for(chrono::seconds(1));
    t12.act(2);
    this_thread::sleep_for(chrono::seconds(1));
    t21.act(2);
    this_thread::sleep_for(chrono::seconds(1));
    t22.act(2);
    this_thread::sleep_for(chrono::seconds(1));
    t11.act(2);
    this_thread::sleep_for(chrono::seconds(1));
    t11.act(3);
    t12.act(3);
    t21.act(3);
    t22.act(3);
//    t11.terminate();
//    t12.terminate();
//    t21.terminate();
//    t22.terminate();
    this_thread::sleep_for(chrono::seconds(1));
}
