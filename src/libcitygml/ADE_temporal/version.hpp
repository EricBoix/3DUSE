#ifndef _VERSION_HPP_
#define _VERSION_HPP_

#include "citygml.hpp"

namespace temporal
{

class Version : public citygml::Object
{
	friend class citygml::CityGMLHandler;

public :

	Version(const std::string& id);

	const std::vector<std::string>& getTags();
	void addTag( std::string );

	const std::vector<citygml::CityObject*>& getVersionMembers();

private :
	std::vector<std::string> _tags;
	std::vector<citygml::CityObject*> _versionMembers;

};

} //namespace temporal


#endif //_VERSION_HPP_