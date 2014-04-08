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

/*
 * Mapping between a hash (represented as size_t) and a list of indexes to the
 * values field of the database.
 */
typedef std::map<size_t, std::list<int>> bucket;

/*
 * Random complex data structure to provide an example how the hashing should
 * be implemented. No actual fields are used.
 */
struct birth_date
{
	int year;
	int month;
	int day;
	bool afternoon;
};

/* 
 * DISCLAIMER: I am not the author of this function, somebody in the Boost
 * library is. I just found it in the source it, since there is no std:: method
 * to provide this functionality.
 */
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

/*
 * Extract the idx-th value of the tuple and save it to the corresponding
 * bucket. In case that there is no such bucket, it is created.
 */
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
		l.push_back(db::values.size() - 1);
		db::index_bucket[idx][h] = l;
	}
	
	if (count == 1)
	{
		db::index_bucket[idx][h].push_back(db::values.size() - 1);
	}
};

namespace std 
{
	/*
	 * As encouraged by the standard library, specialized the std::hash struct to
	 * provide the hashing functionality for our complex structure. This approach
	 * might not be the ideal one, but it provides a nice generic example.
	 */
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

	/*
	 * Debugging output helper.
	 */
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

	/* Number of the tuple elements. */
	static size_t tuple_size;

	/* Bucket for each index. Indexed with the array index. */
	static std::vector<bucket> index_bucket;

	/* 
	 * Place where the whole tuples are saved. The buckets save indices to this
	 * vector for further access (and also to prevent data duplication) 
	 */
	static std::vector<value_type> values;

	static void insert(value_type value)
	{
		values.push_back(value);

		save<people_db, value_type, 0>(value);
		save<people_db, value_type, 1>(value);
		save<people_db, value_type, 2>(value);
	}
};

/* Initialization of static variables. */
size_t people_db::tuple_size = std::tuple_size<people_db::value_type>::value;
std::vector<bucket> people_db::index_bucket(people_db::tuple_size);
std::vector<people_db::value_type> people_db::values;

/* 
 * The find function.
 * 
 * Creates a hash of the key and looks for the hashed key in the appropriate
 * index bucket. Now the function takes the located list of indices to the
 * values array and needs to check not only the hash, but also the actual
 * values stored (since the hash function contains collisions). A nice
 * optimization takes place, when we found already two (simply more than just
 * one) same data pieces - we stop the search, since there is no need to search
 * the list till the end, we already know that the data is duplicated and we
 * can throw an exception.
 */
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

