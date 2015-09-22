#include "ADE.hpp"

//instanciate the ADE Handler constructors map
ADEHandlerFactory::mapType * ADEHandlerFactory::ADEmap = new mapType();

//some gmlHandler methods
std::string ADEHandler::getGmlIdAttribute( void* attributes ) { return getAttribute( attributes, "gml:id", "" ); }
unsigned int ADEHandler::getPathDepth( void ) { return (getNodePath())->size(); }
void ADEHandler::pushCityObject( citygml::CityObject* object )
{
	citygml::CityObject** currentCityObject = getCurrentCityObject();
    // add parent relation
    if(*currentCityObject)
    {
        object->_parent = *currentCityObject;
    }

	if ( *currentCityObject && object ) (*currentCityObject)->getChildren().push_back( object );
	std::stack<citygml::CityObject*>* cityObjectStack = getCityObjectStack();
	cityObjectStack->push( *currentCityObject );
	*currentCityObject = object;
}
void ADEHandler::pushObject( citygml::Object* object )
{
	std::stack<citygml::Object*>* objectStack = getObjectStack();
	objectStack->push( object );
	citygml::Object** currentObject = getCurrentObject();
	*currentObject = object;
}
void ADEHandler::popCityObject( void )
{
	citygml::CityObject** currentCityObject = getCurrentCityObject();
	std::stack<citygml::CityObject*>* cityObjectStack = getCityObjectStack();
	*currentCityObject = 0; 
	if ( cityObjectStack->empty() ) return; 
	*currentCityObject = cityObjectStack->top(); 
	cityObjectStack->pop();
}
void ADEHandler::popObject( void )
{
	citygml::Object** currentObject = getCurrentObject();
	std::stack<citygml::Object*>* objectStack = getObjectStack();
	*currentObject = 0; 
	if ( objectStack->empty() ) return; 
	objectStack->pop();
	*currentObject = objectStack->empty() ? 0 : objectStack->top();			
}