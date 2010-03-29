#ifndef __STREAMOPS_H__
#define __STREAMOPS_H__

#include <ostream>
#include <vector>
#include <map>
#include <list>
#include <utility>

template<typename T, typename char_T, typename traits>
std::basic_ostream<char_T, traits>& operator<<(
    std::basic_ostream<char_T, traits>& os, std::vector<T> const& a){
    os << "vec[" << a.size() << "] {";
    for(typename std::vector<T>::const_iterator i = a.begin(); i != a.end();){
        os << *i;
        if(++i != a.end())
            os << ", ";
    }
    os << "}";
    return os;
}

template<typename T, typename char_T, typename traits>
std::basic_ostream<char_T, traits>& operator<<(
    std::basic_ostream<char_T, traits>& os, std::list<T> const& a){
    os << "list[" << a.size() << "] {";
    for(typename std::list<T>::const_iterator i = a.begin(); i != a.end();){
        os << *i;
        if(++i != a.end())
            os << ", ";
    }
    os << "}";
    return os;
}

template<typename key_T, typename val_T, typename char_T, typename traits>
std::basic_ostream<char_T, traits>& operator<<(
    std::basic_ostream<char_T, traits>& os, std::map<key_T, val_T> const& m){
    os << "map[" << m.size() << "] {";
    for(typename std::map<key_T, val_T>::const_iterator i = m.begin(); i != m.end();){
        os << i->first << " : " << i->second;
        if(++i != m.end())
            os << ", ";
    }
    os << "}";
    return os;
}

template<typename T1, typename T2, typename char_T, typename traits>
std::basic_ostream<char_T, traits>& operator<<(
    std::basic_ostream<char_T, traits>& os, std::pair<T1, T2> const& a){
    os << "{" << a.first << "," << a.second << "}";
    return os;
}

#endif // __STREAMOPS_H__

