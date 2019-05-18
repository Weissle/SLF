#include<iostream>
using namespace std;
class A {
	int a;
public:
	virtual bool operator==(const A &a) {
		cout << "==" << endl;
		return true;
	}
};
class AA :public A {
public:
	bool operator>(const A &a) {
		cout << ">" << endl;
		return false;
	}
};

int main() {

	A *p = new A();
	A *q = new A();
	p->operator==(*q);
	AA *pp = new AA();
//	pp->


	return 0;
}