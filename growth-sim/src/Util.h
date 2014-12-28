#pragma once

#include "cinder/PolyLine.h"
#include "cinder/Json.h"
#include <vector>
#include <string>

ci::JsonTree toJson(const std::string &key, ci::Vec2f &vec);
ci::JsonTree toJson(const std::string &key, ci::PolyLine2f &poly);
