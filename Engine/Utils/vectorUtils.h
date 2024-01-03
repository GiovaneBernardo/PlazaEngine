#pragma once
#include <iostream>
#include <random>
#include <unordered_map>
#include <fileSystem>
#include "ThirdParty/filesystem/filesys.h"
#include <algorithm>

namespace Utils {
	namespace Vector {
		template<typename T>
        inline int indexOf(std::vector<T> vector, T item) {
            auto it = std::find(vector.begin(), vector.end(), item);

            if (it != vector.end()) {
                // Item found
                return std::distance(vector.begin(), it);
            }
            else {
                std::cout << "Item doesn't belong to this vector" << std::endl;
                return -1;
            }
        }
	}
}