#ifndef __CITYGML_APPEARANCE_HPP__
#define __CITYGML_APPEARANCE_HPP__
////////////////////////////////////////////////////////////////////////////////
#include "object.hpp"
////////////////////////////////////////////////////////////////////////////////
namespace citygml
{
////////////////////////////////////////////////////////////////////////////////
class Appearance : public Object
{
    friend class CityGMLHandler;
public:
    Appearance( const std::string& id, const std::string& typeString );

    virtual ~Appearance( void );

    std::string getType( void ) const;
    bool getIsFront( void ) const;

    virtual std::string toString( void ) const;

protected:
    std::string _typeString;
    bool _isFront;
};
////////////////////////////////////////////////////////////////////////////////
} // namespace citygml
////////////////////////////////////////////////////////////////////////////////
#endif // __CITYGML_APPEARANCE_HPP__
