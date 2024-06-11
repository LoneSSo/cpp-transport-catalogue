#include "domain.h"

using namespace std;

size_t PairHash::operator()(const pair<const Stop*, const Stop*>& pair) const {
    return sv_hasher_(pair.first -> name) + sv_hasher_(pair.second -> name) * 37;
}