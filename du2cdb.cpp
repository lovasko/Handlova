#include "du2cdb.hpp"
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

const int people_count = 100000;

/* Very-random name generator. */
char*
generate_name (size_t given_length, size_t family_length)
{
	char alphabet[] = "abcdefghijklmnopqrstuvwxy";
	size_t alphabet_size = strlen(alphabet);
	char *name = (char*)malloc(given_length + 1 + family_length + 1);

	for (int i = 0; i < given_length; i++)
	{
		name[i] = alphabet[rand() % alphabet_size];
	}
	name[given_length] = ' ';

	for (int i = 0; i < family_length; i++)
	{
		name[i + given_length + 1] = alphabet[rand() % alphabet_size];
	}
	name[given_length + 1 + family_length] = '\0';

	name[0] = toupper(name[0]);
	name[given_length + 1] = toupper(name[given_length + 1]);

	return name;
}

/* Very-random money generator. */
int
generate_money ()
{
	static const int max_money = 100000;
	return (rand() % max_money);
}

/* 
 * Debug printing of the whole database. Please note, that this function can
 * slow down the program significantly. 
 */
void
debug_print_database ()
{
	printf("Database: (%lu)\n", people_db::values.size());
	for (std::vector<people_db::value_type>::iterator it = people_db::values.begin(); 
	     it != people_db::values.end(); it++)
	{
		people_db::value_type_print(*it);
		printf("\n");
	}
	printf("\n");
}

/*
 * Fill the database with random data and then perform few queries.
 */
int 
main (void)
{
	srand(time(NULL));
	for (int i = 0; i < people_count; i++)
	{
		struct birth_date bd {(rand() % 35) + 1990, rand() % 12, rand() % 31, 
		    rand() % 2 == 0 ? true : false};

		people_db::insert(std::make_tuple(
			std::string(generate_name(6, 9)), 
			bd, 
			generate_money()
			));
	}

	debug_print_database();
	
	people_db::value_type query;
	for (int i = 0; i < 10; i++)
	{
		int money = generate_money();
		printf("Query in column 2, key %d\n", money);

		try
		{
			query = find<people_db, 2>(money);
			people_db::value_type_print(query);
			printf("\n");
		}
		catch (std::exception& re)
		{
			printf("ERROR: money %d: %s\n", money, re.what());
		}

		printf("\n");
	}

	struct birth_date my_bd {1992, 8, 5, true};
	people_db::value_type me = std::make_tuple(std::string("Daniel Lovasko"),
	my_bd, 0);

	printf("Searching for my birthday buddy:\nMe: ");
	people_db::value_type_print(me);
	printf("\n");
	try
	{
		query = find<people_db, 1>(my_bd);
		printf("Yay! Exactly one person has the same birth day as me!\n");
		people_db::value_type_print(query);
		printf("\n");
	}
	catch (std::exception& re)
	{
		printf("ERROR: %s\n", re.what());
	}
}

