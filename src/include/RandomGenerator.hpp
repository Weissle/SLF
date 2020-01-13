#pragma once
#include<time.h>
#include<random>
#include<math.h>
#include<assert.h>
namespace rg {

	//PROBILITY is form 0 to 1 ,meanly
	enum RANDOMMODEL { NORMAL = 1, MEANLY = 2, PROBILITY = 3, NOREPEATINT = 4 };
	class RandomGenerator
	{
		RANDOMMODEL RM;
		std::normal_distribution<double> normalDistribution;
	public:
		RANDOMMODEL getRandomModelType();
		virtual double getOne();
		virtual RandomGenerator& operator >> (double &num);
		virtual RandomGenerator& operator >> (int &num);
		RandomGenerator(RANDOMMODEL _RM);
		RandomGenerator() {};
		~RandomGenerator() {};
	};
	class NormalRandomGenerator :public RandomGenerator {

		double mean, variance;
		std::normal_distribution<double> normalDistribution;
		std::default_random_engine generator;
	public:
		NormalRandomGenerator(double _mean, double _variance, bool seed = true);
		NormalRandomGenerator() :RandomGenerator(RANDOMMODEL::NORMAL) {};
		~NormalRandomGenerator() {};
		double getOne();
	};
	class MeanlyRandomGenerator :public RandomGenerator {
		double min, max, dis;
	public:
		double getOne();
		MeanlyRandomGenerator(double _min, double _max);
		MeanlyRandomGenerator() :RandomGenerator(RANDOMMODEL::MEANLY) {};
		~MeanlyRandomGenerator() {};
	};
	class ProbilityRandomGenerator :public RandomGenerator {
	public:
		double getOne();
		ProbilityRandomGenerator() :RandomGenerator(RANDOMMODEL::PROBILITY) {};
		~ProbilityRandomGenerator() {};
	};
	class NoRepeatIntRandomGenerator : public RandomGenerator {
		int top = 0;
		int *p;
	public:
		NoRepeatIntRandomGenerator() :RandomGenerator(RANDOMMODEL::PROBILITY) {};
		~NoRepeatIntRandomGenerator() {
			delete []p;
		};
		NoRepeatIntRandomGenerator(int max);
		double getOne();
	};
	
		double RandomGenerator::getOne()
	{
		return 0.0;

	}
	RandomGenerator & RandomGenerator::operator >> (double &num)
	{
		num = this->getOne();
		return *this;
	}
	
	RANDOMMODEL RandomGenerator::getRandomModelType()
	{
		return RM;
	}
	RandomGenerator::RandomGenerator(RANDOMMODEL _RM) :RM(_RM)
	{
	}
	NormalRandomGenerator::NormalRandomGenerator(double _mean, double _variance, bool seed) :RandomGenerator(RANDOMMODEL::NORMAL), mean(_mean), variance(_variance)
	{
		if (seed)	this->generator.seed((unsigned int)time(NULL));
		normalDistribution = std::normal_distribution<double>(_mean, _variance);
	}
	double NormalRandomGenerator::getOne()
	{
		return normalDistribution(generator);
	}

	MeanlyRandomGenerator::MeanlyRandomGenerator(double _min, double _max) :RandomGenerator(RANDOMMODEL::MEANLY), min(_min), max(_max)
	{
		dis = _max - _min;
		assert(dis >= 0 && "max should larger than min");
	}
	double MeanlyRandomGenerator::getOne()
	{
		double one = (double)((rand() * 100 + rand()) % 10000) / 10000.0;
		double two = rand() % (int)dis + min;
		return one + two;
	}

	double ProbilityRandomGenerator::getOne()
	{
		int randomNum = rand() * 100 + rand();
		return (double)(randomNum % 10000) / 10000.0;
	}

	RandomGenerator & RandomGenerator::operator >> (int & num)
	{
		num = getOne();
		return *this;
	}

	NoRepeatIntRandomGenerator::NoRepeatIntRandomGenerator(int max) :RandomGenerator(RANDOMMODEL::NOREPEATINT), top(max)
	{
		p = new int[max]();

	}

	double NoRepeatIntRandomGenerator::getOne()
	{
		if (top == 0)return -1;
		int randomNum = (unsigned int)(rand() * 100 + rand()) % top;
		int answer;
		if (p[randomNum] == 0) answer = randomNum;
		else answer = p[randomNum];
		top--;
		if (p[top] == 0) p[randomNum] = top;
		else p[randomNum] = p[top];
		return answer;
	}


}
