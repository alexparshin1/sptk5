/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         thread_rwlock.cpp  -  description
                             -------------------
    begin                : Tue Dec 14 1999
    copyright            : (C) 2005-2012 by Ilya A. Volynets-Evenbakh
    email                : ilya@total-knowledge.com
 ***************************************************************************/

/***************************************************************************
 This library is free software; you can redistribute it and/or modify it
 under the terms of the GNU Library General Public License as published by
 the Free Software Foundation; either version 2 of the License, or (at
 your option) any later version.

 This library is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
 General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; if not, write to the Free Software Foundation,
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

 Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/cutils>
#include <iostream>

using namespace sptk;

class T1: public CThread
{
    CRWLock *lock;
    const char* name;
    int a;
public:
    T1(const char* name, CRWLock *lock) :
            CThread(name, 1), lock(lock), name(name), a(1)
    {
    }
    void act(int _a)
    {
        std::cout << name << " Setting act to " << _a << '\n';
        this->a = _a;
    }
    void actionL()
    {
        std::cout << name << " is trying to lock\n";
        int rc = lock->lockR(10);
        std::cout << name << " got it(" << rc << ")\n";
        if (a == 1)
            a = 0;
    }
    void actionU()
    {
        std::cout << name << " is trying to ulock\n";
        lock->unlock();
        std::cout << name << " got it\n";
        if (a == 2)
            a = 0;
    }
    virtual void threadFunction()
    {
        std::cout << "Thread " << name << " starting\n";
        while (1) {
            //std::cout<<name<<": act="<<a<<'\n';
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
            msleep(1000);
        }
    }
};
class T2: public CThread
{
    CRWLock *lock;
    const char* name;
    int a;
public:
    T2(const char* name, CRWLock *lock) :
            CThread(name, 1), lock(lock), name(name), a(1)
    {
    }
    void act(int _a)
    {
        std::cout << name << " Setting act to " << _a << '\n';
        this->a = _a;
    }
    void actionL()
    {
        std::cout << name << " is trying to lock\n";
        int rc = lock->lockRW(10);
        std::cout << name << " got it(" << rc << ")\n";
        if (a == 1)
            a = 0;
    }
    void actionU()
    {
        std::cout << name << " is trying to ulock\n";
        lock->unlock();
        std::cout << name << " got it\n";
        if (a == 2)
            a = 0;
    }
    virtual void threadFunction()
    {
        std::cout << "Thread " << name << " starting\n";
        while (1) {
            //std::cout<<name<<": act="<<a<<'\n';
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
            msleep(1000);
        }
    }
};

int main()
{
    CRWLock lock;
    T1 t11("t11", &lock), t12("t12", &lock);
    T2 t21("t21", &lock), t22("t22", &lock);
    t11.run();
    CThread::msleep(1000);
    t12.run();
    CThread::msleep(1000);
    t21.run();
    CThread::msleep(1000);
    t22.run();
    CThread::msleep(1000);
    t12.act(2);
    CThread::msleep(1000);
    t21.act(2);
    CThread::msleep(1000);
    t22.act(2);
    CThread::msleep(1000);
    t11.act(2);
    CThread::msleep(5000);
    t11.act(3);
    t12.act(3);
    t21.act(3);
    t22.act(3);
//    t11.terminate();
//    t12.terminate();
//    t21.terminate();
//    t22.terminate();
    CThread::msleep(1000);
}
