#pragma once

#ifdef DEBUG
#define choiceassert(msg)\
if(!msg){__debugbreak();}
#else
#define choiceassert(msg)
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <utility>
#include <map>
#include <stdint.h>
#include <cstdlib>
#include <optional>
#include <vector>