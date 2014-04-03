#include "du2cdb.hpp"
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

const int people_count = 100000;

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
			(rand() % 50) + 50 
			));
	}

	debug_print_database();
	
	people_db::value_type query;
	for (int i = 0; i < 10; i++)
	{
		int weight = (rand() % 50) + 50;
		printf("Query in column 2, key %d\n", weight);

		try
		{
			query = find<people_db, 2>(weight);
			people_db::value_type_print(query);
			printf("\n");
		}
		catch (std::exception& re)
		{
			printf("ERROR: weight %d: %s\n",weight, re.what());
		}

		printf("\n");
	}

	struct birth_date my_bd {1992, 8, 5, true};
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

