#include <stdio.h>
#include <tuple>
#include <utility>
#include <vector>
#include <list>
#include <map>
#include <stdexcept>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <functional>
#include <string>

typedef std::map<size_t, std::list<int>> bucket;

struct birth_date
{
	int year;
	int month;
	int day;
	bool afternoon;
};

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template<class db, class T, size_t idx>
void save (T tuple)
{
	typename std::tuple_element<idx, typename db::value_type>::type element = std::get<idx>(tuple);
	std::hash<typename std::tuple_element<idx, typename db::value_type>::type> hash_functor;
	size_t h = hash_functor(element);

	int count = db::index_bucket[idx].count(h);
	if (count == 0)
	{
		std::list<int> l;
		l.push_back(db::current());
		db::index_bucket[idx][h] = l;
	}
	
	if (count == 1)
	{
		db::index_bucket[idx][h].push_back(db::current());
	}
};

namespace std 
{
	template <> struct hash<struct birth_date>
	{
		size_t operator()(const struct birth_date& x) const
		{
			size_t h = std::hash<int>()(x.year);
			hash_combine(h, x.month);
			hash_combine(h, x.day);
			hash_combine(h, x.afternoon);
			
			return h;
		}
	};
}

struct people_db
{
	typedef std::tuple<std::string, struct birth_date, int> value_type;

	static void value_type_print (value_type& vt)
	{
		std::cout << "(" 
			<< std::get<0>(vt) 
			<< ", {" 
			<< std::get<1>(vt).day 
			<< ", "
			<< std::get<1>(vt).month
			<< ", " 
			<< std::get<1>(vt).year 
			<< ", " <<
			std::get<1>(vt).afternoon 
			<< "}, " 
			<< std::get<2>(vt) 
			<< ")";
	}

	static size_t tuple_size;
	static std::vector<bucket> index_bucket;
	static std::vector<value_type> values;
	static int n; 

	static int current() 
	{ 
		return n;
	}

	static void advance()
	{
		n++;
	}

	static void insert(value_type value)
	{
		values.push_back(value);

		save<people_db, value_type, 0>(value);
		save<people_db, value_type, 1>(value);
		save<people_db, value_type, 2>(value);

		advance();
	}
};

size_t people_db::tuple_size = std::tuple_size<people_db::value_type>::value;
std::vector<bucket> people_db::index_bucket(people_db::tuple_size);
std::vector<people_db::value_type> people_db::values;
int people_db::n = 0;


template <class db, int idx>
const typename db::value_type &find(
const typename std::tuple_element<idx, typename db::value_type>::type &key)
{
	bucket b = db::index_bucket[idx];

	std::hash<typename std::tuple_element<idx, typename db::value_type>::type> hash_functor;
	size_t h = hash_functor(key);

	bucket::iterator it = b.find(h);
	if (it != b.end())
	{
	  std::list<int> v = std::get<1>(*it);
		int match_count = 0;
		int match;

		for (std::list<int>::iterator iti = v.begin(); iti != v.end(); iti++)
		{
			if (memcmp(&std::get<idx>(db::values[*iti]), &key, sizeof(key)) == 0)
			{
				match = *iti;
				match_count += 1;
				if (match_count > 1)
				{
					throw std::runtime_error("The object is stored many times.");
				}
			}
		}

		if (match_count == 1)
		{
			return db::values[match];
		}
		else if (match_count == 0) 
		{
			throw std::runtime_error("The object was not found.");
		}
		else
		{
			throw std::runtime_error("Unexpected match count");
		}
	}
	else
	{
		throw std::runtime_error("The object was not found.");
	}
};

