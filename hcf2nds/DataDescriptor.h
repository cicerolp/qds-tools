#pragma once
#include "stdafx.h"

class DataDescriptor {
public:
	DataDescriptor(ulong size) : _size(size) { }
	DataDescriptor(const Schema& schema) : _schema(schema) {
		std::ifstream is(schema.path + "hcf/descriptor.hcf");

std::cout << schema.path + "hcf/descriptor.hcf" << std::endl;
		
		boost::archive::text_iarchive archive(is);

		archive & (*this);
	}

	virtual ~DataDescriptor() = default;

	inline const Schema& schema() const;

	inline token_t get(const std::string& key) const;
	inline void set(const tuple_t& tuple);

	void save(const std::string& fileName) const {
		std::ofstream os(fileName);
		boost::archive::text_oarchive archive(os);

		archive & (*this);
	}

	inline unsigned int size() const;

	template <class Archive>
	void serialize(Archive & ar, const unsigned int version);


protected:
	unsigned int _size{ 0 };
	unsigned int _bitsPerHash{ 0 }; // legacy
	std::unordered_map<std::string, token_t> _tokens;

	Schema _schema;

private:
	friend class boost::serialization::access;
};

template <class Archive>
void DataDescriptor::serialize(Archive & ar, const unsigned int version) {
	ar & _size;
	ar & _bitsPerHash;
	ar & _tokens;
}

const Schema& DataDescriptor::schema() const {
	return _schema;
}

token_t DataDescriptor::get(const std::string& key) const {
	return _tokens.at(key);
}

void DataDescriptor::set(const tuple_t& tuple) {
	_tokens.emplace(std::get<0>(tuple), token_t(std::get<1>(tuple), std::get<2>(tuple)));
}

unsigned int DataDescriptor::size() const {
	return _size * _schema.fraction;
}