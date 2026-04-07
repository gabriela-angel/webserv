#pragma once

#include <string>
#include <map>
#include <vector>

#define HOST "HOST"
#define CONTENT_LENGTH "CONTENT-LENGTH"
#define TRANSFER_ENCODING "TRANSFER-ENCODING"
#define CONNECTION "CONNECTION"
#define EXPECT "EXPECT"
#define CONTENT_TYPE "CONTENT-TYPE"
#define COOKIE "COOKIE"

class   Headers {

	/* Ortodoxal-Canonical-Form */
	public:
		Headers() {}
		Headers(const Headers& copy);
		Headers& operator=(const Headers& other);
		~Headers() {}
	/* ------------------------ */


    public:
		typedef std::string							Key;
		typedef std::vector<std::string>			Values;
		typedef std::map<Key, Values>		Map;
		typedef Map::const_iterator					ConstIterator;
		typedef Map::iterator						Iterator;
		typedef Values::const_iterator				ConstValueIterator;
		typedef Values::iterator					ValueIterator;
	
	private:
		Map											_headers;
	
	public:
		static std::string							toUpper(const std::string& str);
		static std::string							TrainCase(const std::string& str);
		static bool									compare(const std::string& str1, const std::string& str2);

		void										add(const std::string& key, const std::string& value);
		void										remove(const std::string& key);
		bool										hasKey(const std::string& key) const;
		std::string									toString() const;

		Iterator									find(const std::string& key);
		ConstIterator								find(const std::string& key) const;

		Iterator									begin();
		ConstIterator								begin() const;

		Iterator									end();
		ConstIterator								end() const;

		Values &									at(const std::string& key);
		const Values &								at(const std::string& key) const;

		Values &									operator[](const std::string& key);
};