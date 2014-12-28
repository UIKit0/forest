#include "Util.h"

using namespace ci;
using namespace std;


JsonTree toJson(const string &key, Vec2f &vec)
{
    JsonTree obj = JsonTree::makeObject(key);
    
    obj.addChild(JsonTree("x", vec.x));
    obj.addChild(JsonTree("y", vec.y));

    return obj;
}


JsonTree toJson(const string &key, PolyLine2f &poly)
{
    JsonTree array = JsonTree::makeArray(key);
    PolyLine2f::iterator i;
    
    for (i = poly.begin(); i != poly.end(); i++) {
        array.addChild(toJson("", *i));
    }
    
    return array;
}
