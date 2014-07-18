////////////////////////////////////////////////////////////////////////////////
#include "appearance.hpp"
////////////////////////////////////////////////////////////////////////////////
namespace citygml
{
////////////////////////////////////////////////////////////////////////////////
Appearance::Appearance( const std::string& id, const std::string& typeString )
    : Object( id ), _typeString( typeString ), _isFront(true)
{}
////////////////////////////////////////////////////////////////////////////////
Appearance::~Appearance( void )
{}
////////////////////////////////////////////////////////////////////////////////
std::string Appearance::getType( void ) const
{
    return _typeString;
}
////////////////////////////////////////////////////////////////////////////////
bool Appearance::getIsFront( void ) const
{
    return _isFront;
}
////////////////////////////////////////////////////////////////////////////////
std::string Appearance::toString( void ) const
{
    return _typeString + " " + _id;
}
////////////////////////////////////////////////////////////////////////////////
} // namespace citygml
////////////////////////////////////////////////////////////////////////////////
